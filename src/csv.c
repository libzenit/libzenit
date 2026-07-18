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

#include <libzenit/csv.h>
#include <stdlib.h>
#include <string.h>

/* ─── Internal helpers ─── */

/* Grow buf to new_cap using the record's allocator. On failure, cleans up the record. */
static int grow_buf(char **buf, size_t new_cap, zenit_csv_record_t *out, zenit_allocator_t a) {
    char *tmp = zenit_allocator_realloc(a, *buf, new_cap / 2, new_cap);
    if (tmp == NULL) {
        a.free_fn(*buf, a.ctx);
        zenit_csv_record_destroy(out);
        return 0;
    }
    *buf = tmp;
    return 1;
}

/* Append a char to buf, growing if needed. Returns 0 on alloc failure. */
static int buf_append(char **buf, size_t *cap, size_t *len, char c, zenit_csv_record_t *out, zenit_allocator_t a) {
    if (*len + 1 >= *cap) {
        if (!grow_buf(buf, *cap * 2, out, a)) return 0;
        *cap *= 2;
    }
    (*buf)[(*len)++] = c;
    return 1;
}

/* Read a quoted field from p into buf. Advances p past the closing quote. */
static int read_quoted(const char **p, char **buf, size_t *cap, size_t *len, zenit_csv_record_t *out, zenit_allocator_t a) {
    (*p)++;
    while (**p != '\0') {
        if (**p != '"') {
            if (!buf_append(buf, cap, len, **p, out, a)) return 0;
            (*p)++;
            continue;
        }
        if (*(*p + 1) == '"') {
            (*p)++;
            (*p)++;
            if (!buf_append(buf, cap, len, '"', out, a)) return 0;
            continue;
        }
        (*p)++;
        return 1;
    }
    return 1;
}

/* Read an unquoted field from p into buf. Stops at delimiter, \n, \r, or \0.
   Returns 0 on alloc failure. */
static int read_unquoted(const char **p, char **buf, size_t *cap, size_t *len, char delim, zenit_csv_record_t *out, zenit_allocator_t a) {
    while (**p != '\0' && **p != delim && **p != '\n' && **p != '\r') {
        if (!buf_append(buf, cap, len, **p, out, a)) return 0;
        (*p)++;
    }
    return 1;
}

/* Grow the fields array in out. Returns 0 on alloc failure. */
static int grow_fields(zenit_csv_record_t *out, size_t *cap, zenit_allocator_t a) {
    *cap *= 2;
    char **tmp = zenit_allocator_realloc(a, out->fields, (*cap / 2) * sizeof(char *), *cap * sizeof(char *));
    if (tmp == NULL) {
        zenit_csv_record_destroy(out);
        return 0;
    }
    out->fields = tmp;
    return 1;
}

/* ─── Public API ─── */

zenit_result_t zenit_csv_parse_record_with_allocator(const char *line, char delimiter, zenit_csv_record_t *out, zenit_allocator_t allocator) {
    if (line == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    out->fields = NULL;
    out->count = 0;

    size_t field_cap = 8;
    out->fields = allocator.alloc_fn(field_cap * sizeof(char *), allocator.ctx);
    if (out->fields == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    const char *p = line;
    size_t fi = 0;

    while (1) {
        if (fi >= field_cap && !grow_fields(out, &field_cap, allocator)) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }

        size_t buf_cap = 64;
        size_t buf_len = 0;
        char *buf = allocator.alloc_fn(buf_cap, allocator.ctx);
        if (buf == NULL) {
            zenit_csv_record_destroy(out);
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }

        if (*p == '"') {
            if (!read_quoted(&p, &buf, &buf_cap, &buf_len, out, allocator)) {
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            }
        } else {
            if (!read_unquoted(&p, &buf, &buf_cap, &buf_len, delimiter, out, allocator)) {
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            }
        }

        if (buf_len >= buf_cap) {
            if (!grow_buf(&buf, buf_cap + 1, out, allocator)) {
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            }
            buf_cap++;
        }
        buf[buf_len] = '\0';
        out->fields[fi] = buf;
        out->count = ++fi;

        if (*p == delimiter) {
            p++;
            continue;
        }
        break;
    }

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_csv_parse_record(const char *line, char delimiter, zenit_csv_record_t *out) {
    return zenit_csv_parse_record_with_allocator(line, delimiter, out, ZENIT_ALLOCATOR_DEFAULT);
}

void zenit_csv_record_destroy(zenit_csv_record_t *record) {
    if (record == NULL) {
        return;
    }
    if (record->fields != NULL) {
        for (size_t i = 0; i < record->count; i++) {
            free(record->fields[i]);
        }
        free(record->fields);
    }
    record->fields = NULL;
    record->count = 0;
}

static int needs_quoting(const char *s, char delimiter) {
    if (s == NULL || *s == '\0') return 1;
    if (*s == ' ' || *s == '\t') return 1;
    size_t len = strlen(s);
    if (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) return 1;

    for (; *s; s++) {
        if (*s == delimiter || *s == '"' || *s == '\n' || *s == '\r') {
            return 1;
        }
    }
    return 0;
}

/* Compute the serialised length of a single field (quoted or raw). */
static size_t serialised_field_len(const char *s, char delimiter) {
    if (needs_quoting(s, delimiter)) {
        size_t n = 2;
        for (const char *p = s; *p; p++) {
            if (*p == '"') n++;
            n++;
        }
        return n;
    }
    return strlen(s);
}

/* Write a single field into buf at pos. Returns the new position. */
static size_t serialised_field_write(const char *s, char delimiter, char *buf, size_t pos) {
    if (needs_quoting(s, delimiter)) {
        buf[pos++] = '"';
        for (const char *p = s; *p; p++) {
            if (*p == '"') {
                buf[pos++] = '"';
            }
            buf[pos++] = *p;
        }
        buf[pos++] = '"';
    } else {
        size_t flen = strlen(s);
        memcpy(buf + pos, s, flen);
        pos += flen;
    }
    return pos;
}

zenit_result_t zenit_csv_serialise_record_with_allocator(const zenit_csv_record_t *record, char delimiter, char **out, zenit_allocator_t allocator) {
    if (record == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    size_t total = 0;
    for (size_t i = 0; i < record->count; i++) {
        if (i > 0) total++;
        total += serialised_field_len(record->fields[i], delimiter);
    }
    total++;

    char *buf = allocator.alloc_fn(total, allocator.ctx);
    if (buf == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    size_t pos = 0;
    for (size_t i = 0; i < record->count; i++) {
        if (i > 0) buf[pos++] = delimiter;
        pos = serialised_field_write(record->fields[i], delimiter, buf, pos);
    }
    buf[pos] = '\0';

    *out = buf;
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_csv_serialise_record(const zenit_csv_record_t *record, char delimiter, char **out) {
    return zenit_csv_serialise_record_with_allocator(record, delimiter, out, ZENIT_ALLOCATOR_DEFAULT);
}
