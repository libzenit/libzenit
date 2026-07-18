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

struct zenit_json_value_t {
    zenit_json_type_t type;
    zenit_allocator_t allocator;
    union {
        int bool_val;
        double num_val;
        char *str;
        struct {
            zenit_json_value_t **items;
            size_t count;
            size_t capacity;
        } arr;
        struct {
            char **keys;
            zenit_json_value_t **values;
            size_t count;
            size_t capacity;
        } obj;
    };
    struct zenit_json_value_t *next;
};

struct zenit_json_t {
    zenit_allocator_t allocator;
    zenit_json_value_t *root;
    zenit_json_value_t *head;
    zenit_json_value_t *tail;
};

/* ──────────────────────────────────────────────────────────────────────────
 * Parser state
 * ────────────────────────────────────────────────────────────────────────── */

typedef struct {
    const char *pos;
    const char *end;
    zenit_json_t *doc;
    int error;
} parser_t;

/* ──────────────────────────────────────────────────────────────────────────
 * Allocation helpers
 * ────────────────────────────────────────────────────────────────────────── */

static void value_free_string(zenit_json_value_t *val, zenit_allocator_t a) {
    if (val->str != NULL) {
        a.free_fn(val->str, a.ctx);
        val->str = NULL;
    }
}

static void value_free_array(zenit_json_value_t *val, zenit_allocator_t a) {
    if (val->arr.items != NULL) {
        a.free_fn(val->arr.items, a.ctx);
        val->arr.items = NULL;
    }
    val->arr.count = 0;
    val->arr.capacity = 0;
}

static void value_free_object(zenit_json_value_t *val, zenit_allocator_t a) {
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

static void value_free_payload(zenit_json_value_t *val, zenit_allocator_t a) {
    if (val->type == ZENIT_JSON_STRING) {
        value_free_string(val, a);
    } else if (val->type == ZENIT_JSON_ARRAY) {
        value_free_array(val, a);
    } else if (val->type == ZENIT_JSON_OBJECT) {
        value_free_object(val, a);
    }
    val->type = ZENIT_JSON_NULL;
}

static zenit_json_value_t *value_alloc(zenit_json_t *doc) {
    zenit_allocator_t a = doc->allocator;
    zenit_json_value_t *val = a.alloc_fn(sizeof(zenit_json_value_t), a.ctx);
    if (val == NULL) {
        return NULL;
    }
    memset(val, 0, sizeof(*val));
    val->allocator = a;
    val->next = NULL;
    if (doc->tail != NULL) {
        doc->tail->next = val;
    } else {
        doc->head = val;
    }
    doc->tail = val;
    return val;
}

static char *str_dup_n(zenit_allocator_t a, const char *s, size_t n) {
    char *copy = a.alloc_fn(n + 1, a.ctx);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}

static char *str_dup(zenit_allocator_t a, const char *s) {
    return str_dup_n(a, s, strlen(s));
}

/* ──────────────────────────────────────────────────────────────────────────
 * Grow helpers for array and object backing stores
 * ────────────────────────────────────────────────────────────────────────── */

static int array_grow(zenit_json_value_t *arr, size_t min_capacity, zenit_allocator_t a) {
    (void)min_capacity;
    size_t old_cap = arr->arr.capacity;
    size_t new_cap = old_cap == 0 ? 8 : old_cap + (old_cap / 2);
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

static int object_grow(zenit_json_value_t *obj, size_t min_capacity, zenit_allocator_t a) {
    (void)min_capacity;
    size_t old_cap = obj->obj.capacity;
    size_t new_cap = old_cap == 0 ? 8 : old_cap + (old_cap / 2);
    size_t old_size = old_cap * sizeof(char *);

    /* Allocate zero-initialised memory so unused slots between old_size and
       new_size are NULL rather than garbage (defensive — obj->count is the
       authoritative tracker, but a read beyond count would otherwise consume
       uninitialised pointers). */
    void *new_keys = zenit_allocator_alloc_zero(a, new_cap, sizeof(char *));
    if (new_keys == NULL) {
        return 0;
    }

    void *new_values = zenit_allocator_alloc_zero(a, new_cap, sizeof(char *));
    if (new_values == NULL) {
        a.free_fn(new_keys, a.ctx);
        return 0;
    }

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
 * ────────────────────────────────────────────────────────────────────────── */

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

static zenit_json_value_t *parse_value(parser_t *p);
static zenit_json_value_t *parse_array(parser_t *p);
static zenit_json_value_t *parse_object(parser_t *p);

/* Grow the buffer only when len + needed >= cap.  On realloc failure clean
 * up and set error.  Returns 0 on failure, 1 on success. */
static int buf_grow(char **buf, size_t *cap, size_t len, size_t needed, zenit_allocator_t a, parser_t *p) {
    if (len + needed <= *cap) {
        return 1;
    }
    size_t new_cap = *cap * 2;
    char *nb = zenit_allocator_realloc(a, *buf, len, new_cap);
    if (nb == NULL) {
        a.free_fn(*buf, a.ctx);
        p->error = 1;
        return 0;
    }
    *buf = nb;
    *cap = new_cap;
    return 1;
}

/* Decode a \uXXXX escape sequence at p->pos into a codepoint.
 * On success, advances p->pos past the 4 hex digits and returns 1.
 * On failure, sets p->error and returns 0. */
static int parse_unicode_cp(parser_t *p, unsigned int *cp) {
    if (p->pos + 4 > p->end) {
        p->error = 1;
        return 0;
    }
    *cp = 0;
    for (int i = 0; i < 4; i++) {
        char h = p->pos[i];
        *cp <<= 4;
        if      (h >= '0' && h <= '9') *cp |= (unsigned)(h - '0');
        else if (h >= 'a' && h <= 'f') *cp |= (unsigned)(h - 'a' + 10);
        else if (h >= 'A' && h <= 'F') *cp |= (unsigned)(h - 'A' + 10);
        else {
            p->error = 1;
            return 0;
        }
    }
    p->pos += 4;
    return 1;
}

/* Encode a Unicode codepoint as UTF-8 into buf at len.  Grows buf if needed.
 * Returns 0 on realloc failure (error already set), 1 on success. */
static int encode_utf8(char **buf, size_t *cap, size_t *len, unsigned int cp, zenit_allocator_t a, parser_t *p) {
    if (cp <= 0x7F) {
        if (!buf_grow(buf, cap, *len, 1, a, p)) { return 0; }
        (*buf)[(*len)++] = (char)cp;
    } else if (cp <= 0x7FF) {
        if (!buf_grow(buf, cap, *len, 2, a, p)) { return 0; }
        (*buf)[(*len)++] = (char)(0xC0 | (cp >> 6));
        (*buf)[(*len)++] = (char)(0x80 | (cp & 0x3F));
    } else {
        if (!buf_grow(buf, cap, *len, 3, a, p)) { return 0; }
        (*buf)[(*len)++] = (char)(0xE0 | (cp >> 12));
        (*buf)[(*len)++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        (*buf)[(*len)++] = (char)(0x80 | (cp & 0x3F));
    }
    return 1;
}

/* Decode a single-character escape (e.g. 'n' -> '\n').
 * Returns the decoded character, or -1 if not a simple escape. */
static int parse_simple_escape(char esc) {
    switch (esc) {
        case '"': return '"';
        case '\\': return '\\';
        case '/': return '/';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        default: return -1;
    }
}

/* Parse an escape sequence at p->pos (which must be at the character
 * AFTER the backslash).  Returns the decoded codepoint on success,
 * or -1 on error (p->error set).  Handles both simple escapes and \uXXXX. */
static int parse_escape_seq(parser_t *p, unsigned int *cp) {
    if (p->pos >= p->end) { p->error = 1; return -1; }
    int simple = parse_simple_escape(*p->pos);
    if (simple >= 0) {
        p->pos++;
        *cp = (unsigned int)simple;
        return 1;
    }
    if (*p->pos == 'u') {
        p->pos++;
        return parse_unicode_cp(p, cp) ? 1 : -1;
    }
    p->error = 1;
    return -1;
}

static char *parse_string_raw(parser_t *p) {
    p->pos++;

    zenit_allocator_t a = p->doc->allocator;
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
            p->pos++;
            buf[len] = '\0';
            return buf;
        }
        if (c != '\\') {
            if (!buf_grow(&buf, &cap, len, 1, a, p)) { return NULL; }
            buf[len++] = c;
            p->pos++;
            continue;
        }
        p->pos++;
        unsigned int cp;
        int ret = parse_escape_seq(p, &cp);
        if (ret < 0) {
            a.free_fn(buf, a.ctx);
            return NULL;
        }
        if (!encode_utf8(&buf, &cap, &len, cp, a, p)) {
            return NULL;
        }
    }

    a.free_fn(buf, a.ctx);
    p->error = 1;
    return NULL;
}

static int parse_number_digits(const char **s, const char *end) {
    if (*s >= end || **s < '0' || **s > '9') { return 0; }
    if (**s == '0') {
        (*s)++;
    } else {
        while (*s < end && **s >= '0' && **s <= '9') { (*s)++; }
    }
    return 1;
}

static double parse_number(parser_t *p) {
    const char *start = p->pos;
    char *end = NULL;
    double result = strtod(start, &end);

    if (end == start) {
        p->error = 1;
        return 0.0;
    }

    const char *s = start;
    if (*s == '-') s++;
    if (!parse_number_digits(&s, end)) { p->error = 1; return 0.0; }

    if (s < end && *s == '.') {
        s++;
        if (!parse_number_digits(&s, end)) { p->error = 1; return 0.0; }
    }

    if (s < end && (*s == 'e' || *s == 'E')) {
        s++;
        if (s < end && (*s == '+' || *s == '-')) s++;
        if (!parse_number_digits(&s, end)) { p->error = 1; return 0.0; }
    }

    if (s != end) {
        p->error = 1;
        return 0.0;
    }

    p->pos = end;
    return result;
}

static zenit_json_value_t *parse_literal_null(parser_t *p) {
    if (p->end - p->pos >= 4 && memcmp(p->pos, "null", 4) == 0) {
        zenit_json_value_t *val = value_alloc(p->doc);
        if (val == NULL) { p->error = 1; return NULL; }
        val->type = ZENIT_JSON_NULL;
        p->pos += 4;
        return val;
    }
    p->error = 1;
    return NULL;
}

static zenit_json_value_t *parse_literal_true(parser_t *p) {
    if (p->end - p->pos >= 4 && memcmp(p->pos, "true", 4) == 0) {
        zenit_json_value_t *val = value_alloc(p->doc);
        if (val == NULL) { p->error = 1; return NULL; }
        val->type = ZENIT_JSON_BOOL;
        val->bool_val = 1;
        p->pos += 4;
        return val;
    }
    p->error = 1;
    return NULL;
}

static zenit_json_value_t *parse_literal_false(parser_t *p) {
    if (p->end - p->pos >= 5 && memcmp(p->pos, "false", 5) == 0) {
        zenit_json_value_t *val = value_alloc(p->doc);
        if (val == NULL) { p->error = 1; return NULL; }
        val->type = ZENIT_JSON_BOOL;
        val->bool_val = 0;
        p->pos += 5;
        return val;
    }
    p->error = 1;
    return NULL;
}

static zenit_json_value_t *parse_value(parser_t *p) {
    skip_ws(p);
    if (p->pos >= p->end) {
        p->error = 1;
        return NULL;
    }

    char c = *p->pos;

    if (c == 'n') { return parse_literal_null(p); }
    if (c == 't') { return parse_literal_true(p); }
    if (c == 'f') { return parse_literal_false(p); }
    if (c == '[') { return parse_array(p); }
    if (c == '{') { return parse_object(p); }
    if (c == '"') {
        char *str = parse_string_raw(p);
        if (str == NULL) { return NULL; }
        zenit_json_value_t *val = value_alloc(p->doc);
        if (val == NULL) {
            p->doc->allocator.free_fn(str, p->doc->allocator.ctx);
            p->error = 1;
            return NULL;
        }
        val->type = ZENIT_JSON_STRING;
        val->str = str;
        return val;
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        zenit_json_value_t *val = value_alloc(p->doc);
        if (val == NULL) { p->error = 1; return NULL; }
        val->type = ZENIT_JSON_NUMBER;
        val->num_val = parse_number(p);
        if (p->error) { return NULL; }
        return val;
    }

    p->error = 1;
    return NULL;
}

static int parse_array_separator(parser_t *p) {
    skip_ws(p);
    if (p->pos >= p->end) { p->error = 1; return 0; }
    if (*p->pos == ',') {
        p->pos++;
        skip_ws(p);
        if (p->pos >= p->end || *p->pos == ']') { p->error = 1; return 0; }
        return 1;
    }
    if (*p->pos == ']') {
        p->pos++;
        return 2;
    }
    p->error = 1;
    return 0;
}

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

        if (arr->arr.count >= arr->arr.capacity && !array_grow(arr, arr->arr.count + 1, a)) {
            p->error = 1;
            return NULL;
        }
        arr->arr.items[arr->arr.count++] = elem;

        int sep = parse_array_separator(p);
        if (sep == 0) { return NULL; }
        if (sep == 2) { return arr; }
    }
}

static int parse_object_separator(parser_t *p) {
    skip_ws(p);
    if (p->pos >= p->end) { p->error = 1; return 0; }
    if (*p->pos == ',') {
        p->pos++;
        skip_ws(p);
        if (p->pos >= p->end || *p->pos == '}') { p->error = 1; return 0; }
        return 1;
    }
    if (*p->pos == '}') {
        p->pos++;
        return 2;
    }
    p->error = 1;
    return 0;
}

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

        if (obj->obj.count >= obj->obj.capacity && !object_grow(obj, obj->obj.count + 1, a)) {
            a.free_fn(key, a.ctx);
            p->error = 1;
            return NULL;
        }
        obj->obj.keys[obj->obj.count] = key;
        obj->obj.values[obj->obj.count] = val;
        obj->obj.count++;

        int sep = parse_object_separator(p);
        if (sep == 0) { return NULL; }
        if (sep == 2) { return obj; }
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

static zenit_result_t array_grow_or_error(zenit_json_value_t *arr, zenit_allocator_t a) {
    if (arr->arr.count >= arr->arr.capacity && !array_grow(arr, arr->arr.count + 1, a)) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_json_array_append(zenit_json_value_t *arr, zenit_json_value_t *item) {
    if (arr == NULL || item == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (arr->type != ZENIT_JSON_ARRAY) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    zenit_result_t r = array_grow_or_error(arr, arr->allocator);
    if (r.error != ZENIT_OK) { return r; }
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
    zenit_result_t r = array_grow_or_error(arr, arr->allocator);
    if (r.error != ZENIT_OK) { return r; }
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

    for (size_t i = 0; i < obj->obj.count; i++) {
        if (strcmp(obj->obj.keys[i], key) == 0) {
            obj->obj.values[i] = val;
            return ZENIT_RESULT_OK;
        }
    }

    if (obj->obj.count >= obj->obj.capacity && !object_grow(obj, obj->obj.count + 1, a)) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
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

static int value_serialize_into(const zenit_json_value_t *val, zenit_string_t *out, zenit_allocator_t a);

static int append_cstr(zenit_string_t *out, const char *s) {
    return zenit_string_append_cstr(out, s).error == ZENIT_OK ? 1 : 0;
}

static int append_char(zenit_string_t *out, char c) {
    return zenit_string_append(out, &c, 1).error == ZENIT_OK ? 1 : 0;
}

static int serialize_escape_char(unsigned char c, zenit_string_t *out) {
    switch (c) {
        case '"':  return append_cstr(out, "\\\"");
        case '\\': return append_cstr(out, "\\\\");
        case '\b': return append_cstr(out, "\\b");
        case '\f': return append_cstr(out, "\\f");
        case '\n': return append_cstr(out, "\\n");
        case '\r': return append_cstr(out, "\\r");
        case '\t': return append_cstr(out, "\\t");
        default:
            if (c < 0x20) {
                char hex[7];
                snprintf(hex, sizeof(hex), "\\u%04x", c);
                return append_cstr(out, hex);
            }
            return append_char(out, (char)c);
    }
}

static int string_escape_into(const char *s, zenit_string_t *out, zenit_allocator_t a) {
    (void)a;
    if (!append_cstr(out, "\"")) { return 0; }
    while (*s) {
        if (!serialize_escape_char((unsigned char)*s, out)) { return 0; }
        s++;
    }
    return append_cstr(out, "\"");
}

static void format_number(double num, char *buf, size_t buf_size) {
    double int_part;
    if (modf(num, &int_part) == 0.0 && num >= -9007199254740992.0 && num <= 9007199254740992.0) {
        snprintf(buf, buf_size, "%.0f", num);
        return;
    }
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
    snprintf(buf, buf_size, "%s", best);
}

static int serialize_array(const zenit_json_value_t *val, zenit_string_t *out, zenit_allocator_t a) {
    if (!append_cstr(out, "[")) { return 0; }
    for (size_t i = 0; i < val->arr.count; i++) {
        if (i > 0 && !append_cstr(out, ",")) { return 0; }
        if (!value_serialize_into(val->arr.items[i], out, a)) { return 0; }
    }
    return append_cstr(out, "]");
}

static int serialize_object(const zenit_json_value_t *val, zenit_string_t *out, zenit_allocator_t a) {
    if (!append_cstr(out, "{")) { return 0; }
    for (size_t i = 0; i < val->obj.count; i++) {
        if (i > 0 && !append_cstr(out, ",")) { return 0; }
        if (!string_escape_into(val->obj.keys[i], out, a)) { return 0; }
        if (!append_cstr(out, ":")) { return 0; }
        if (!value_serialize_into(val->obj.values[i], out, a)) { return 0; }
    }
    return append_cstr(out, "}");
}

static int value_serialize_into(const zenit_json_value_t *val, zenit_string_t *out, zenit_allocator_t a) {
    if (val == NULL) {
        return append_cstr(out, "null");
    }
    if (val->type == ZENIT_JSON_NULL) {
        return append_cstr(out, "null");
    }
    if (val->type == ZENIT_JSON_BOOL) {
        return val->bool_val ? append_cstr(out, "true") : append_cstr(out, "false");
    }
    if (val->type == ZENIT_JSON_NUMBER) {
        char buf[64];
        format_number(val->num_val, buf, sizeof(buf));
        return append_cstr(out, buf);
    }
    if (val->type == ZENIT_JSON_STRING) {
        return string_escape_into(val->str, out, a);
    }
    if (val->type == ZENIT_JSON_ARRAY) {
        return serialize_array(val, out, a);
    }
    return serialize_object(val, out, a);
}

char *zenit_json_value_serialize_with_allocator(const zenit_json_value_t *val, zenit_allocator_t allocator) {
    zenit_string_t *s = zenit_string_create_with_allocator(allocator);
    if (s == NULL) {
        return NULL;
    }

    if (!value_serialize_into(val, s, allocator)) {
        zenit_string_destroy(s);
        return NULL;
    }

    const char *cstr = zenit_string_cstr(s);
    char *result = str_dup(allocator, cstr);
    zenit_string_destroy(s);
    return result;
}

char *zenit_json_value_serialize(const zenit_json_value_t *val) {
    return zenit_json_value_serialize_with_allocator(val, ZENIT_ALLOCATOR_DEFAULT);
}

char *zenit_json_serialize_with_allocator(const zenit_json_t *json, zenit_allocator_t allocator) {
    if (json == NULL || json->root == NULL) {
        return NULL;
    }
    return zenit_json_value_serialize_with_allocator(json->root, allocator);
}

char *zenit_json_serialize(const zenit_json_t *json) {
    if (json == NULL || json->root == NULL) {
        return NULL;
    }
    return zenit_json_value_serialize(json->root);
}
