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

zenit_result_t zenit_csv_parse_record_with_allocator(const char *line, char delimiter, zenit_csv_record_t *out, zenit_allocator_t allocator) {
    if (line == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Initialise output */
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
        /* Grow fields array if needed */
        if (fi >= field_cap) {
            field_cap *= 2;
            char **tmp = zenit_allocator_realloc(allocator, out->fields, (field_cap / 2) * sizeof(char *), field_cap * sizeof(char *));
            if (tmp == NULL) {
                zenit_csv_record_destroy(out);
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            }
            out->fields = tmp;
        }

        /* Parse one field */
        size_t buf_cap = 64;
        size_t buf_len = 0;
        char *buf = allocator.alloc_fn(buf_cap, allocator.ctx);
        if (buf == NULL) {
            zenit_csv_record_destroy(out);
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }

        if (*p == '"') {
            /* Quoted field */
            p++;
            while (1) {
                if (*p == '\0') {
                    /* End of input while in quoted field — accept it */
                    break;
                }
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Escaped quote */
                        p++;
                        /* Append one quote */
                        if (buf_len + 1 >= buf_cap) {
                            buf_cap *= 2;
                            char *tmp = zenit_allocator_realloc(allocator, buf, buf_cap / 2, buf_cap);
                            if (tmp == NULL) {
                                allocator.free_fn(buf, allocator.ctx);
                                zenit_csv_record_destroy(out);
                                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
                            }
                            buf = tmp;
                        }
                        buf[buf_len++] = '"';
                        p++;
                    } else {
                        /* End of quoted field */
                        p++;
                        break;
                    }
                } else {
                    if (buf_len + 1 >= buf_cap) {
                        buf_cap *= 2;
                        char *tmp = zenit_allocator_realloc(allocator, buf, buf_cap / 2, buf_cap);
                        if (tmp == NULL) {
                            allocator.free_fn(buf, allocator.ctx);
                            zenit_csv_record_destroy(out);
                            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
                        }
                        buf = tmp;
                    }
                    buf[buf_len++] = *p;
                    p++;
                }
            }
        } else {
            /* Unquoted field */
            while (*p != '\0' && *p != delimiter && *p != '\n' && *p != '\r') {
                if (buf_len + 1 >= buf_cap) {
                    buf_cap *= 2;
                    char *tmp = zenit_allocator_realloc(allocator, buf, buf_cap / 2, buf_cap);
                    if (tmp == NULL) {
                        allocator.free_fn(buf, allocator.ctx);
                        zenit_csv_record_destroy(out);
                        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
                    }
                    buf = tmp;
                }
                buf[buf_len++] = *p;
                p++;
            }
        }

        /* Null-terminate the field */
        if (buf_len >= buf_cap) {
            buf_cap++;
            char *tmp = zenit_allocator_realloc(allocator, buf, buf_cap - 1, buf_cap);
            if (tmp == NULL) {
                allocator.free_fn(buf, allocator.ctx);
                zenit_csv_record_destroy(out);
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            }
            buf = tmp;
        }
        buf[buf_len] = '\0';
        out->fields[fi++] = buf;

        /* Check for delimiter or end of record */
        if (*p == delimiter) {
            p++;
            continue;
        }
        break;
    }

    out->count = fi;
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

/* Check if a field needs quoting (contains delimiter, quote, newline, or leading/trailing whitespace) */
static int needs_quoting(const char *s, char delimiter) {
    if (s == NULL || *s == '\0') return 1;
    /* Leading or trailing whitespace */
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

zenit_result_t zenit_csv_serialise_record_with_allocator(const zenit_csv_record_t *record, char delimiter, char **out, zenit_allocator_t allocator) {
    if (record == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Calculate total output length */
    size_t total = 0;
    for (size_t i = 0; i < record->count; i++) {
        const char *field = record->fields[i];
        if (i > 0) total++; /* delimiter */

        if (needs_quoting(field, delimiter)) {
            total += 2; /* opening and closing quotes */
            for (const char *p = field; *p; p++) {
                if (*p == '"') total++; /* doubled quote */
                total++;
            }
        } else {
            total += strlen(field);
        }
    }
    total++; /* NUL */

    char *buf = allocator.alloc_fn(total, allocator.ctx);
    if (buf == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    size_t pos = 0;
    for (size_t i = 0; i < record->count; i++) {
        const char *field = record->fields[i];
        if (i > 0) buf[pos++] = delimiter;

        if (needs_quoting(field, delimiter)) {
            buf[pos++] = '"';
            for (const char *p = field; *p; p++) {
                if (*p == '"') {
                    buf[pos++] = '"';
                }
                buf[pos++] = *p;
            }
            buf[pos++] = '"';
        } else {
            size_t flen = strlen(field);
            memcpy(buf + pos, field, flen);
            pos += flen;
        }
    }
    buf[pos] = '\0';

    *out = buf;
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_csv_serialise_record(const zenit_csv_record_t *record, char delimiter, char **out) {
    return zenit_csv_serialise_record_with_allocator(record, delimiter, out, ZENIT_ALLOCATOR_DEFAULT);
}
