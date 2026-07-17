//
//    LibZenit
//    Copyright (C) 2026  Ian Torres
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <libzenit/json.h>
#include <libzenit/allocator.h>
#include <libzenit/string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ──────────────────────────────────────────────────────────────────────────
 * Internal type definitions
 * ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Internal representation of a single JSON value.
 *
 * Each value is individually allocated via the document's custom allocator.
 * The @p allocator field is a COPY of the document's allocator, stored so
 * that array/object mutation functions can grow backing stores without
 * needing a back-pointer to the document.
 *
 * Non-trivial payloads (string copies, array/item buffers) are allocated
 * through the same allocator.  The @p next field links every value in a
 * singly-linked list owned by the document, enabling O(#values) cleanup in
 * zenit_json_destroy().
 */
struct zenit_json_value_t {
    zenit_json_type_t type;           /**< Discriminant — selects the active union member */
    zenit_allocator_t allocator;      /**< Allocator copy — needed for array/object growth */
    union {
        int bool_val;                        /**< ZENIT_JSON_BOOL payload */
        double num_val;                      /**< ZENIT_JSON_NUMBER payload */
        char *str;                           /**< ZENIT_JSON_STRING — owned, null-terminated */
        struct {
            zenit_json_value_t **items;      /**< ZENIT_JSON_ARRAY — owned array of value ptrs */
            size_t count;                    /**< Number of elements currently stored */
            size_t capacity;                 /**< Allocated slot count */
        } arr;
        struct {
            char **keys;                     /**< ZENIT_JSON_OBJECT — owned array of key strings */
            zenit_json_value_t **values;     /**< Parallel array of value pointers */
            size_t count;                    /**< Number of key/value pairs */
            size_t capacity;                 /**< Allocated slot count */
        } obj;
    };
    struct zenit_json_value_t *next;   /**< Linked-list — owned by the document for cleanup */
};

/**
 * @brief Internal document representation.
 *
 * Holds the custom allocator, the root value (may be NULL for empty
 * documents), and a singly-linked list of every value allocated through
 * the document.  The linked list is used by zenit_json_destroy() to free
 * every value without needing to traverse the tree recursively.
 */
struct zenit_json_t {
    zenit_allocator_t allocator; /**< Custom allocator used for all memory */
    zenit_json_value_t *root;    /**< Root value, or NULL if the document is empty */
    zenit_json_value_t *head;    /**< Head of the value linked list (for cleanup) */
    zenit_json_value_t *tail;    /**< Tail of the value linked list (for O(1) append) */
};

/* ──────────────────────────────────────────────────────────────────────────
 * Parser state
 * ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Transient state threaded through every recursive descent parse
 *        function.
 *
 * Wraps the input range, a back-pointer to the document being populated,
 * and an error flag that short-circuits further parsing on failure.
 */
typedef struct {
    const char *pos;       /**< Current parse position */
    const char *end;       /**< One past the last valid byte */
    zenit_json_t *doc;     /**< Document being populated */
    int error;             /**< Set to 1 when a syntax or allocation error occurs */
} parser_t;

/* ──────────────────────────────────────────────────────────────────────────
 * Allocation helpers
 * ────────────────────────────────────────────────────────────────────────── */

/* Dispose of a value's dynamically-allocated internals WITHOUT freeing the
 * value struct itself and WITHOUT unlinking it from the document list.
 * Called during partial cleanup or when replacing a value's payload. */
static void value_free_payload(zenit_json_value_t *val, zenit_allocator_t a) {
    if (val->type == ZENIT_JSON_STRING) {
        if (val->str != NULL) {
            a.free_fn(val->str, a.ctx);
            val->str = NULL;
        }
    } else if (val->type == ZENIT_JSON_ARRAY) {
        if (val->arr.items != NULL) {
            a.free_fn(val->arr.items, a.ctx);
            val->arr.items = NULL;
        }
        val->arr.count = 0;
        val->arr.capacity = 0;
    } else if (val->type == ZENIT_JSON_OBJECT) {
        if (val->obj.keys != NULL) {
            for (size_t i = 0; i < val->obj.count; i++) {
                if (val->obj.keys[i] != NULL) {
                    a.free_fn(val->obj.keys[i], a.ctx);
                }
            }
            a.free_fn(val->obj.keys, a.ctx);
            val->obj.keys = NULL;
        }
        if (val->obj.values != NULL) {
            a.free_fn(val->obj.values, a.ctx);
            val->obj.values = NULL;
        }
        val->obj.count = 0;
        val->obj.capacity = 0;
    }
    val->type = ZENIT_JSON_NULL;
}

/* Allocate a new value, zero its fields, store a copy of the document's
 * allocator, link it into the document's linked list, and return a stable
 * pointer.  Returns NULL on OOM. */
static zenit_json_value_t *value_alloc(zenit_json_t *doc) {
    zenit_allocator_t a = doc->allocator;
    zenit_json_value_t *val = a.alloc_fn(sizeof(zenit_json_value_t), a.ctx);
    if (val == NULL) {
        return NULL;
    }
    memset(val, 0, sizeof(*val));
    val->allocator = a;
    /* Append to the document's value linked list */
    val->next = NULL;
    if (doc->tail != NULL) {
        doc->tail->next = val;
    } else {
        doc->head = val;
    }
    doc->tail = val;
    return val;
}

/* Duplicate a counted byte sequence into an allocator-owned, null-terminated
 * copy.  Returns NULL on OOM. */
static char *str_dup_n(zenit_allocator_t a, const char *s, size_t n) {
    char *copy = a.alloc_fn(n + 1, a.ctx);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}

/* Convenience wrapper — duplicate a null-terminated string. */
static char *str_dup(zenit_allocator_t a, const char *s) {
    return str_dup_n(a, s, strlen(s));
}

/* ──────────────────────────────────────────────────────────────────────────
 * Grow helpers for array and object backing stores
 * ────────────────────────────────────────────────────────────────────────── */

/* Grow an array's item buffer to fit at least @p min_capacity elements.
 * Uses 1.5x growth factor, matching the rest of the library. */
static int array_grow(zenit_json_value_t *arr, size_t min_capacity, zenit_allocator_t a) {
    size_t old_cap = arr->arr.capacity;
    size_t new_cap = old_cap == 0 ? 8 : old_cap + (old_cap / 2);
    if (new_cap < min_capacity) {
        new_cap = min_capacity;
    }
    size_t new_size = new_cap * sizeof(zenit_json_value_t *);
    zenit_json_value_t **new_items;
    if (arr->arr.items == NULL) {
        new_items = a.alloc_fn(new_size, a.ctx);
    } else {
        size_t old_size = old_cap * sizeof(zenit_json_value_t *);
        new_items = zenit_allocator_realloc(a, arr->arr.items, old_size, new_size);
    }
    if (new_items == NULL) {
        return 0;
    }
    arr->arr.items = new_items;
    arr->arr.capacity = new_cap;
    return 1;
}

/* Grow an object's key and value buffers in lockstep.
 *
 * Allocates both new buffers first, copies old content, then frees old
 * buffers.  This avoids the rollback problem: if one allocation succeeds
 * and the other fails, the old (still valid) buffers are not touched. */
static int object_grow(zenit_json_value_t *obj, size_t min_capacity, zenit_allocator_t a) {
    size_t old_cap = obj->obj.capacity;
    size_t new_cap = old_cap == 0 ? 8 : old_cap + (old_cap / 2);
    if (new_cap < min_capacity) {
        new_cap = min_capacity;
    }
    size_t new_size = new_cap * sizeof(char *);
    size_t old_size = old_cap * sizeof(char *);

    /* Allocate key buffer */
    void *new_keys = a.alloc_fn(new_size, a.ctx);
    if (new_keys == NULL) {
        return 0;
    }

    /* Allocate value buffer */
    void *new_values = a.alloc_fn(new_size, a.ctx);
    if (new_values == NULL) {
        a.free_fn(new_keys, a.ctx);
        return 0;
    }

    /* Copy old data (if any) and free old buffers */
    if (obj->obj.keys != NULL) {
        memcpy(new_keys, obj->obj.keys, old_size);
        a.free_fn(obj->obj.keys, a.ctx);
    }
    if (obj->obj.values != NULL) {
        memcpy(new_values, obj->obj.values, old_size);
        a.free_fn(obj->obj.values, a.ctx);
    }

    obj->obj.keys = new_keys;
    obj->obj.values = new_values;
    obj->obj.capacity = new_cap;
    return 1;
}

/* ──────────────────────────────────────────────────────────────────────────
 * Recursive-descent JSON parser
 *
 * Every parse_* function takes a parser_t, advances @p pos, and returns
 * a freshly allocated value (or NULL on error).  On failure the parser's
 * error flag is set and partial values are left attached to the document
 * — zenit_json_destroy() will clean them up.
 * ────────────────────────────────────────────────────────────────────────── */

/* Advance past any whitespace characters. */
static void skip_ws(parser_t *p) {
    while (p->pos < p->end) {
        unsigned char c = (unsigned char)*p->pos;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            p->pos++;
        } else {
            break;
        }
    }
}

/* Forward declarations for recursive value parsing. */
static zenit_json_value_t *parse_value(parser_t *p);
static zenit_json_value_t *parse_array(parser_t *p);
static zenit_json_value_t *parse_object(parser_t *p);

/* Parse a JSON string literal (including the surrounding double quotes).
 * Returns an owned char* (the decoded content) or NULL on error. */
static char *parse_string_raw(parser_t *p) {
    /* Expect and skip the opening double-quote */
    if (p->pos >= p->end || *p->pos != '"') {
        p->error = 1;
        return NULL;
    }
    p->pos++;

    zenit_allocator_t a = p->doc->allocator;

    /* Start with a modest buffer and grow on demand */
    size_t cap = 64;
    size_t len = 0;
    char *buf = a.alloc_fn(cap, a.ctx);
    if (buf == NULL) {
        p->error = 1;
        return NULL;
    }

    while (p->pos < p->end) {
        char c = *p->pos;
        if (c == '"') {
            /* Closing quote found — null-terminate and return */
            p->pos++;
            buf[len] = '\0';
            return buf;
        }
        if (c == '\\') {
            /* Escape sequence — consume the backslash and decode the next char */
            p->pos++;
            if (p->pos >= p->end) {
                a.free_fn(buf, a.ctx);
                p->error = 1;
                return NULL;
            }
            char esc = *p->pos;
            switch (esc) {
                case '"':  buf[len++] = '"';  break;
                case '\\': buf[len++] = '\\'; break;
                case '/':  buf[len++] = '/';  break;
                case 'b':  buf[len++] = '\b'; break;
                case 'f':  buf[len++] = '\f'; break;
                case 'n':  buf[len++] = '\n'; break;
                case 'r':  buf[len++] = '\r'; break;
                case 't':  buf[len++] = '\t'; break;
                case 'u': {
                    /* \uXXXX — parse 4 hex digits, encode as UTF-8 */
                    p->pos++;
                    if (p->pos + 4 > p->end) {
                        a.free_fn(buf, a.ctx);
                        p->error = 1;
                        return NULL;
                    }
                    unsigned int cp = 0;
                    for (int i = 0; i < 4; i++) {
                        char h = p->pos[i];
                        cp <<= 4;
                        if      (h >= '0' && h <= '9') cp |= (unsigned)(h - '0');
                        else if (h >= 'a' && h <= 'f') cp |= (unsigned)(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') cp |= (unsigned)(h - 'A' + 10);
                        else {
                            a.free_fn(buf, a.ctx);
                            p->error = 1;
                            return NULL;
                        }
                    }
                    p->pos += 3; /* outer p->pos++ accounts for the 4th char */

                    /* Encode codepoint as UTF-8 */
                    if (cp <= 0x7F) {
                        if (len + 1 > cap) { cap *= 2; char *nb = zenit_allocator_realloc(a, buf, len, cap); if (!nb) { a.free_fn(buf, a.ctx); p->error = 1; return NULL; } buf = nb; }
                        buf[len++] = (char)cp;
                    } else if (cp <= 0x7FF) {
                        if (len + 2 > cap) { cap *= 2; char *nb = zenit_allocator_realloc(a, buf, len, cap); if (!nb) { a.free_fn(buf, a.ctx); p->error = 1; return NULL; } buf = nb; }
                        buf[len++] = (char)(0xC0 | (cp >> 6));
                        buf[len++] = (char)(0x80 | (cp & 0x3F));
                    } else {
                        if (len + 3 > cap) { cap *= 2; char *nb = zenit_allocator_realloc(a, buf, len, cap); if (!nb) { a.free_fn(buf, a.ctx); p->error = 1; return NULL; } buf = nb; }
                        buf[len++] = (char)(0xE0 | (cp >> 12));
                        buf[len++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                        buf[len++] = (char)(0x80 | (cp & 0x3F));
                    }
                    break;
                }
                default:
                    /* Unknown escape sequence */
                    a.free_fn(buf, a.ctx);
                    p->error = 1;
                    return NULL;
            }
            p->pos++;
        } else {
            /* Regular character — append as-is */
            if (len + 1 > cap) {
                cap *= 2;
                char *nb = zenit_allocator_realloc(a, buf, len, cap);
                if (nb == NULL) {
                    a.free_fn(buf, a.ctx);
                    p->error = 1;
                    return NULL;
                }
                buf = nb;
            }
            buf[len++] = c;
            p->pos++;
        }
    }

    /* Unterminated string */
    a.free_fn(buf, a.ctx);
    p->error = 1;
    return NULL;
}

/* Parse a JSON number.  Validates strict JSON number grammar manually, then
 * uses strtod for the final floating-point conversion.  Advances p->pos
 * past the number.  Returns the double value. */
static double parse_number(parser_t *p) {
    const char *start = p->pos;

    /* Use strtod to get the double and the number of bytes consumed */
    char *end = NULL;
    double result = strtod(start, &end);

    /* strtod must consume at least one character */
    if (end == start) {
        p->error = 1;
        return 0.0;
    }

    /* Validate the consumed range against strict JSON number grammar */
    const char *s = start;

    if (*s == '-') s++;
    if (s >= end || *s < '0' || *s > '9') { p->error = 1; return 0.0; }
    if (*s == '0') {
        s++;
    } else {
        while (s < end && *s >= '0' && *s <= '9') s++;
    }

    if (s < end && *s == '.') {
        s++;
        if (s >= end || *s < '0' || *s > '9') { p->error = 1; return 0.0; }
        while (s < end && *s >= '0' && *s <= '9') s++;
    }

    if (s < end && (*s == 'e' || *s == 'E')) {
        s++;
        if (s < end && (*s == '+' || *s == '-')) s++;
        if (s >= end || *s < '0' || *s > '9') { p->error = 1; return 0.0; }
        while (s < end && *s >= '0' && *s <= '9') s++;
    }

    /* If s != end, strtod consumed characters we didn't validate */
    if (s != end) {
        p->error = 1;
        return 0.0;
    }

    p->pos = end;
    return result;
}

/* Dispatch to the appropriate parser based on the current character. */
static zenit_json_value_t *parse_value(parser_t *p) {
    skip_ws(p);
    if (p->pos >= p->end) {
        p->error = 1;
        return NULL;
    }

    char c = *p->pos;
    zenit_json_value_t *val = NULL;

    if (c == 'n') {
        if (p->end - p->pos >= 4 && memcmp(p->pos, "null", 4) == 0) {
            val = value_alloc(p->doc);
            if (val == NULL) { p->error = 1; return NULL; }
            val->type = ZENIT_JSON_NULL;
            p->pos += 4;
        } else {
            p->error = 1;
            return NULL;
        }
    } else if (c == 't') {
        if (p->end - p->pos >= 4 && memcmp(p->pos, "true", 4) == 0) {
            val = value_alloc(p->doc);
            if (val == NULL) { p->error = 1; return NULL; }
            val->type = ZENIT_JSON_BOOL;
            val->bool_val = 1;
            p->pos += 4;
        } else {
            p->error = 1;
            return NULL;
        }
    } else if (c == 'f') {
        if (p->end - p->pos >= 5 && memcmp(p->pos, "false", 5) == 0) {
            val = value_alloc(p->doc);
            if (val == NULL) { p->error = 1; return NULL; }
            val->type = ZENIT_JSON_BOOL;
            val->bool_val = 0;
            p->pos += 5;
        } else {
            p->error = 1;
            return NULL;
        }
    } else if (c == '"') {
        char *str = parse_string_raw(p);
        if (str == NULL) { return NULL; }
        val = value_alloc(p->doc);
        if (val == NULL) {
            p->doc->allocator.free_fn(str, p->doc->allocator.ctx);
            p->error = 1;
            return NULL;
        }
        val->type = ZENIT_JSON_STRING;
        val->str = str;
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        val = value_alloc(p->doc);
        if (val == NULL) { p->error = 1; return NULL; }
        val->type = ZENIT_JSON_NUMBER;
        val->num_val = parse_number(p);
        if (p->error) { return NULL; }
    } else if (c == '[') {
        return parse_array(p);
    } else if (c == '{') {
        return parse_object(p);
    } else {
        p->error = 1;
        return NULL;
    }

    return val;
}

/* Parse a JSON array: '[' value (',' value)* ']' */
static zenit_json_value_t *parse_array(parser_t *p) {
    if (p->pos >= p->end || *p->pos != '[') { p->error = 1; return NULL; }
    p->pos++;

    zenit_json_value_t *arr = value_alloc(p->doc);
    if (arr == NULL) { p->error = 1; return NULL; }
    arr->type = ZENIT_JSON_ARRAY;

    skip_ws(p);

    if (p->pos < p->end && *p->pos == ']') {
        p->pos++;
        return arr;
    }

    zenit_allocator_t a = arr->allocator;

    for (;;) {
        zenit_json_value_t *elem = parse_value(p);
        if (elem == NULL || p->error) { return NULL; }

        if (arr->arr.count >= arr->arr.capacity) {
            if (!array_grow(arr, arr->arr.count + 1, a)) {
                p->error = 1;
                return NULL;
            }
        }
        arr->arr.items[arr->arr.count++] = elem;

        skip_ws(p);
        if (p->pos >= p->end) { p->error = 1; return NULL; }

        if (*p->pos == ',') {
            p->pos++;
            skip_ws(p);
            if (p->pos >= p->end || *p->pos == ']') { p->error = 1; return NULL; }
        } else if (*p->pos == ']') {
            p->pos++;
            return arr;
        } else {
            p->error = 1;
            return NULL;
        }
    }
}

/* Parse a JSON object: '{' string ':' value (',' string ':' value)* '}' */
static zenit_json_value_t *parse_object(parser_t *p) {
    if (p->pos >= p->end || *p->pos != '{') { p->error = 1; return NULL; }
    p->pos++;

    zenit_json_value_t *obj = value_alloc(p->doc);
    if (obj == NULL) { p->error = 1; return NULL; }
    obj->type = ZENIT_JSON_OBJECT;

    skip_ws(p);

    if (p->pos < p->end && *p->pos == '}') {
        p->pos++;
        return obj;
    }

    zenit_allocator_t a = obj->allocator;

    for (;;) {
        skip_ws(p);
        if (p->pos >= p->end || *p->pos != '"') { p->error = 1; return NULL; }
        char *key = parse_string_raw(p);
        if (key == NULL) { return NULL; }

        skip_ws(p);
        if (p->pos >= p->end || *p->pos != ':') {
            a.free_fn(key, a.ctx);
            p->error = 1;
            return NULL;
        }
        p->pos++;

        zenit_json_value_t *val = parse_value(p);
        if (val == NULL || p->error) {
            a.free_fn(key, a.ctx);
            return NULL;
        }

        if (obj->obj.count >= obj->obj.capacity) {
            if (!object_grow(obj, obj->obj.count + 1, a)) {
                a.free_fn(key, a.ctx);
                p->error = 1;
                return NULL;
            }
        }
        obj->obj.keys[obj->obj.count] = key;
        obj->obj.values[obj->obj.count] = val;
        obj->obj.count++;

        skip_ws(p);
        if (p->pos >= p->end) { p->error = 1; return NULL; }

        if (*p->pos == ',') {
            p->pos++;
            skip_ws(p);
            if (p->pos >= p->end || *p->pos == '}') { p->error = 1; return NULL; }
        } else if (*p->pos == '}') {
            p->pos++;
            return obj;
        } else {
            p->error = 1;
            return NULL;
        }
    }
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: document creation / destruction
 * ────────────────────────────────────────────────────────────────────────── */

zenit_json_t *zenit_json_create_with_allocator(zenit_allocator_t allocator) {
    zenit_json_t *doc = allocator.alloc_fn(sizeof(zenit_json_t), allocator.ctx);
    if (doc == NULL) {
        return NULL;
    }
    memset(doc, 0, sizeof(*doc));
    doc->allocator = allocator;
    return doc;
}

zenit_json_t *zenit_json_create(void) {
    return zenit_json_create_with_allocator(ZENIT_ALLOCATOR_DEFAULT);
}

/* Walk the linked list and free every value and its payload. */
static void value_destroy_chain(zenit_json_value_t *head, zenit_allocator_t a) {
    while (head != NULL) {
        zenit_json_value_t *next = head->next;
        value_free_payload(head, a);
        a.free_fn(head, a.ctx);
        head = next;
    }
}

void zenit_json_destroy(zenit_json_t *json) {
    if (json == NULL) {
        return;
    }
    value_destroy_chain(json->head, json->allocator);
    json->allocator.free_fn(json, json->allocator.ctx);
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: parsing
 * ────────────────────────────────────────────────────────────────────────── */

/* Internal entry point.  Expects an already-allocated document. */
static int json_parse_into(zenit_json_t *doc, const char *data, size_t length) {
    parser_t p;
    p.pos = data;
    p.end = data + length;
    p.doc = doc;
    p.error = 0;

    zenit_json_value_t *val = parse_value(&p);
    if (val == NULL || p.error) {
        return 0;
    }

    skip_ws(&p);
    if (p.pos != p.end) {
        return 0;
    }

    doc->root = val;
    return 1;
}

zenit_json_t *zenit_json_parse_with_length_and_allocator(const char *input, size_t length, zenit_allocator_t allocator) {
    if (input == NULL) {
        return NULL;
    }

    zenit_json_t *doc = zenit_json_create_with_allocator(allocator);
    if (doc == NULL) {
        return NULL;
    }

    if (!json_parse_into(doc, input, length)) {
        zenit_json_destroy(doc);
        return NULL;
    }

    return doc;
}

zenit_json_t *zenit_json_parse_with_allocator(const char *input, zenit_allocator_t allocator) {
    return zenit_json_parse_with_length_and_allocator(input, strlen(input), allocator);
}

zenit_json_t *zenit_json_parse_with_length(const char *input, size_t length) {
    return zenit_json_parse_with_length_and_allocator(input, length, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_json_t *zenit_json_parse(const char *input) {
    return zenit_json_parse_with_length_and_allocator(input, strlen(input), ZENIT_ALLOCATOR_DEFAULT);
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: root value access
 * ────────────────────────────────────────────────────────────────────────── */

zenit_json_value_t *zenit_json_root(const zenit_json_t *json) {
    if (json == NULL) {
        return NULL;
    }
    return json->root;
}

zenit_result_t zenit_json_set_root(zenit_json_t *json, zenit_json_value_t *val) {
    if (json == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    json->root = val;
    return ZENIT_RESULT_OK;
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: value type introspection
 * ────────────────────────────────────────────────────────────────────────── */

zenit_json_type_t zenit_json_value_type(const zenit_json_value_t *val) {
    if (val == NULL) {
        return ZENIT_JSON_NULL;
    }
    return val->type;
}

int zenit_json_value_is_null(const zenit_json_value_t *val) {
    return (val == NULL || val->type == ZENIT_JSON_NULL);
}

int zenit_json_value_get_bool(const zenit_json_value_t *val) {
    if (val == NULL || val->type != ZENIT_JSON_BOOL) {
        return 0;
    }
    return val->bool_val;
}

double zenit_json_value_get_number(const zenit_json_value_t *val) {
    if (val == NULL || val->type != ZENIT_JSON_NUMBER) {
        return 0.0;
    }
    return val->num_val;
}

const char *zenit_json_value_get_string(const zenit_json_value_t *val) {
    if (val == NULL || val->type != ZENIT_JSON_STRING) {
        return NULL;
    }
    return val->str;
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: value constructors
 * ────────────────────────────────────────────────────────────────────────── */

zenit_json_value_t *zenit_json_value_null(zenit_json_t *json) {
    if (json == NULL) {
        return NULL;
    }
    zenit_json_value_t *val = value_alloc(json);
    if (val == NULL) { return NULL; }
    val->type = ZENIT_JSON_NULL;
    return val;
}

zenit_json_value_t *zenit_json_value_bool(zenit_json_t *json, int bool_val) {
    if (json == NULL) {
        return NULL;
    }
    zenit_json_value_t *val = value_alloc(json);
    if (val == NULL) { return NULL; }
    val->type = ZENIT_JSON_BOOL;
    val->bool_val = (bool_val != 0);
    return val;
}

zenit_json_value_t *zenit_json_value_number(zenit_json_t *json, double num_val) {
    if (json == NULL) {
        return NULL;
    }
    zenit_json_value_t *val = value_alloc(json);
    if (val == NULL) { return NULL; }
    val->type = ZENIT_JSON_NUMBER;
    val->num_val = num_val;
    return val;
}

zenit_json_value_t *zenit_json_value_string(zenit_json_t *json, const char *str_val) {
    if (json == NULL) {
        return NULL;
    }
    zenit_json_value_t *val = value_alloc(json);
    if (val == NULL) { return NULL; }
    val->type = ZENIT_JSON_STRING;
    if (str_val != NULL) {
        val->str = str_dup(json->allocator, str_val);
    } else {
        val->str = str_dup(json->allocator, "");
    }
    if (val->str == NULL) { return NULL; }
    return val;
}

zenit_json_value_t *zenit_json_value_array(zenit_json_t *json) {
    if (json == NULL) {
        return NULL;
    }
    zenit_json_value_t *val = value_alloc(json);
    if (val == NULL) { return NULL; }
    val->type = ZENIT_JSON_ARRAY;
    return val;
}

zenit_json_value_t *zenit_json_value_object(zenit_json_t *json) {
    if (json == NULL) {
        return NULL;
    }
    zenit_json_value_t *val = value_alloc(json);
    if (val == NULL) { return NULL; }
    val->type = ZENIT_JSON_OBJECT;
    return val;
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: array operations
 * ────────────────────────────────────────────────────────────────────────── */

size_t zenit_json_array_count(const zenit_json_value_t *val) {
    if (val == NULL || val->type != ZENIT_JSON_ARRAY) {
        return 0;
    }
    return val->arr.count;
}

zenit_json_value_t *zenit_json_array_get(const zenit_json_value_t *val, size_t index) {
    if (val == NULL || val->type != ZENIT_JSON_ARRAY || index >= val->arr.count) {
        return NULL;
    }
    return val->arr.items[index];
}

zenit_result_t zenit_json_array_append(zenit_json_value_t *arr, zenit_json_value_t *item) {
    if (arr == NULL || item == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (arr->type != ZENIT_JSON_ARRAY) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    zenit_allocator_t a = arr->allocator;
    if (arr->arr.count >= arr->arr.capacity) {
        if (!array_grow(arr, arr->arr.count + 1, a)) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }
    }
    arr->arr.items[arr->arr.count++] = item;
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_json_array_remove(zenit_json_value_t *arr, size_t index) {
    if (arr == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (arr->type != ZENIT_JSON_ARRAY) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    if (index >= arr->arr.count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    for (size_t i = index + 1; i < arr->arr.count; i++) {
        arr->arr.items[i - 1] = arr->arr.items[i];
    }
    arr->arr.count--;
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_json_array_insert(zenit_json_value_t *arr, size_t index, zenit_json_value_t *item) {
    if (arr == NULL || item == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (arr->type != ZENIT_JSON_ARRAY) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    if (index > arr->arr.count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    zenit_allocator_t a = arr->allocator;
    if (arr->arr.count >= arr->arr.capacity) {
        if (!array_grow(arr, arr->arr.count + 1, a)) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }
    }
    for (size_t i = arr->arr.count; i > index; i--) {
        arr->arr.items[i] = arr->arr.items[i - 1];
    }
    arr->arr.items[index] = item;
    arr->arr.count++;
    return ZENIT_RESULT_OK;
}

/* ──────────────────────────────────────────────────────────────────────────
 * Public API: object operations
 * ────────────────────────────────────────────────────────────────────────── */

size_t zenit_json_object_count(const zenit_json_value_t *val) {
    if (val == NULL || val->type != ZENIT_JSON_OBJECT) {
        return 0;
    }
    return val->obj.count;
}

const char *zenit_json_object_key(const zenit_json_value_t *val, size_t index) {
    if (val == NULL || val->type != ZENIT_JSON_OBJECT || index >= val->obj.count) {
        return NULL;
    }
    return val->obj.keys[index];
}

zenit_json_value_t *zenit_json_object_value_at(const zenit_json_value_t *val, size_t index) {
    if (val == NULL || val->type != ZENIT_JSON_OBJECT || index >= val->obj.count) {
        return NULL;
    }
    return val->obj.values[index];
}

zenit_json_value_t *zenit_json_object_get(const zenit_json_value_t *val, const char *key) {
    if (val == NULL || val->type != ZENIT_JSON_OBJECT || key == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < val->obj.count; i++) {
        if (strcmp(val->obj.keys[i], key) == 0) {
            return val->obj.values[i];
        }
    }
    return NULL;
}

zenit_result_t zenit_json_object_set(zenit_json_value_t *obj, const char *key, zenit_json_value_t *val) {
    if (obj == NULL || key == NULL || val == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (obj->type != ZENIT_JSON_OBJECT) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    zenit_allocator_t a = obj->allocator;

    /* Overwrite if key already exists */
    for (size_t i = 0; i < obj->obj.count; i++) {
        if (strcmp(obj->obj.keys[i], key) == 0) {
            obj->obj.values[i] = val;
            return ZENIT_RESULT_OK;
        }
    }

    /* New key — append */
    if (obj->obj.count >= obj->obj.capacity) {
        if (!object_grow(obj, obj->obj.count + 1, a)) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }
    }

    char *key_copy = str_dup(a, key);
    if (key_copy == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    obj->obj.keys[obj->obj.count] = key_copy;
    obj->obj.values[obj->obj.count] = val;
    obj->obj.count++;
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_json_object_remove(zenit_json_value_t *obj, const char *key) {
    if (obj == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (obj->type != ZENIT_JSON_OBJECT) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    for (size_t i = 0; i < obj->obj.count; i++) {
        if (strcmp(obj->obj.keys[i], key) == 0) {
            obj->allocator.free_fn(obj->obj.keys[i], obj->allocator.ctx);
            for (size_t j = i + 1; j < obj->obj.count; j++) {
                obj->obj.keys[j - 1] = obj->obj.keys[j];
                obj->obj.values[j - 1] = obj->obj.values[j];
            }
            obj->obj.count--;
            return ZENIT_RESULT_OK;
        }
    }

    return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
}

/* ──────────────────────────────────────────────────────────────────────────
 * Serializer
 * ────────────────────────────────────────────────────────────────────────── */

/* Forward declaration. */
static int value_serialize_into(const zenit_json_value_t *val, zenit_string_t *out, zenit_allocator_t a);

/* Append a JSON-escaped string (with surrounding double quotes) to @p out. */
static int string_escape_into(const char *s, zenit_string_t *out, zenit_allocator_t a) {
    (void)a;
    if (s == NULL) {
        return zenit_string_append_cstr(out, "null").error == ZENIT_OK ? 1 : 0;
    }

    if (zenit_string_append_cstr(out, "\"").error != ZENIT_OK) { return 0; }

    while (*s) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
            case '"':  if (zenit_string_append_cstr(out, "\\\"").error != ZENIT_OK) return 0; break;
            case '\\': if (zenit_string_append_cstr(out, "\\\\").error != ZENIT_OK) return 0; break;
            case '\b': if (zenit_string_append_cstr(out, "\\b").error != ZENIT_OK) return 0; break;
            case '\f': if (zenit_string_append_cstr(out, "\\f").error != ZENIT_OK) return 0; break;
            case '\n': if (zenit_string_append_cstr(out, "\\n").error != ZENIT_OK) return 0; break;
            case '\r': if (zenit_string_append_cstr(out, "\\r").error != ZENIT_OK) return 0; break;
            case '\t': if (zenit_string_append_cstr(out, "\\t").error != ZENIT_OK) return 0; break;
            default: {
                if (c < 0x20) {
                    char hex[7];
                    snprintf(hex, sizeof(hex), "\\u%04x", c);
                    if (zenit_string_append_cstr(out, hex).error != ZENIT_OK) return 0;
                } else {
                    if (zenit_string_append(out, (const char *)&c, 1).error != ZENIT_OK) return 0;
                }
                break;
            }
        }
        s++;
    }

    if (zenit_string_append_cstr(out, "\"").error != ZENIT_OK) { return 0; }
    return 1;
}

/* Recursive value serialiser.  Returns 0 on allocation failure, 1 on success. */
static int value_serialize_into(const zenit_json_value_t *val, zenit_string_t *out, zenit_allocator_t a) {
    if (val == NULL) {
        return zenit_string_append_cstr(out, "null").error == ZENIT_OK ? 1 : 0;
    }

    switch (val->type) {
        case ZENIT_JSON_NULL:
            return zenit_string_append_cstr(out, "null").error == ZENIT_OK ? 1 : 0;

        case ZENIT_JSON_BOOL: {
            int ok;
            if (val->bool_val) {
                ok = (zenit_string_append_cstr(out, "true").error == ZENIT_OK);
            } else {
                ok = (zenit_string_append_cstr(out, "false").error == ZENIT_OK);
            }
            return ok;
        }

        case ZENIT_JSON_NUMBER: {
            char buf[64];
            double num = val->num_val;
            double int_part;
            if (modf(num, &int_part) == 0.0 && num >= -9007199254740992.0 && num <= 9007199254740992.0) {
                snprintf(buf, sizeof(buf), "%.0f", num);
            } else {
                /* Format with full precision, then try progressively shorter
                 * representations until we find the shortest one that round-trips
                 * to the same double when parsed by strtod. */
                char best[64];
                snprintf(best, sizeof(best), "%.17g", num);
                for (int prec = 16; prec >= 1; prec--) {
                    char test[64];
                    snprintf(test, sizeof(test), "%.*g", prec, num);
                    char *end;
                    double roundtrip = strtod(test, &end);
                    if (roundtrip == num && *end == '\0') {
                        snprintf(best, sizeof(best), "%.*g", prec, num);
                        break;
                    }
                }
                snprintf(buf, sizeof(buf), "%s", best);
            }
            return zenit_string_append_cstr(out, buf).error == ZENIT_OK ? 1 : 0;
        }

        case ZENIT_JSON_STRING:
            return string_escape_into(val->str, out, a);

        case ZENIT_JSON_ARRAY: {
            if (zenit_string_append_cstr(out, "[").error != ZENIT_OK) { return 0; }
            for (size_t i = 0; i < val->arr.count; i++) {
                if (i > 0) {
                    if (zenit_string_append_cstr(out, ",").error != ZENIT_OK) { return 0; }
                }
                if (!value_serialize_into(val->arr.items[i], out, a)) { return 0; }
            }
            return zenit_string_append_cstr(out, "]").error == ZENIT_OK ? 1 : 0;
        }

        case ZENIT_JSON_OBJECT: {
            if (zenit_string_append_cstr(out, "{").error != ZENIT_OK) { return 0; }
            for (size_t i = 0; i < val->obj.count; i++) {
                if (i > 0) {
                    if (zenit_string_append_cstr(out, ",").error != ZENIT_OK) { return 0; }
                }
                if (!string_escape_into(val->obj.keys[i], out, a)) { return 0; }
                if (zenit_string_append_cstr(out, ":").error != ZENIT_OK) { return 0; }
                if (!value_serialize_into(val->obj.values[i], out, a)) { return 0; }
            }
            return zenit_string_append_cstr(out, "}").error == ZENIT_OK ? 1 : 0;
        }
    }

    return 0;
}

char *zenit_json_value_serialize(const zenit_json_value_t *val) {
    zenit_string_t *s = zenit_string_create();
    if (s == NULL) {
        return NULL;
    }

    if (!value_serialize_into(val, s, ZENIT_ALLOCATOR_DEFAULT)) {
        zenit_string_destroy(s);
        return NULL;
    }

    const char *cstr = zenit_string_cstr(s);
    if (cstr == NULL) {
        zenit_string_destroy(s);
        return NULL;
    }
    char *result = str_dup(ZENIT_ALLOCATOR_DEFAULT, cstr);
    zenit_string_destroy(s);
    return result;
}

char *zenit_json_serialize(const zenit_json_t *json) {
    if (json == NULL || json->root == NULL) {
        return NULL;
    }
    return zenit_json_value_serialize(json->root);
}
