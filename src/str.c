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

#include <libzenit/str.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

static int is_whitespace(char c) {
    return c == ' '  || c == '\t' || c == '\n' ||
           c == '\r' || c == '\f' || c == '\v';
}

static void free_split_result_with_allocator(char **result, size_t count, zenit_allocator_t a) {
    for (size_t j = 0; j < count; j++) {
        a.free_fn(result[j], a.ctx);
    }
    a.free_fn(result, a.ctx);
}

static char *trim_impl(const char *s, zenit_allocator_t a) {
    if (s == NULL) return NULL;

    const char *start = s;
    while (*start && is_whitespace(*start)) {
        start++;
    }

    if (*start == '\0') {
        char *out = a.alloc_fn(1, a.ctx);
        if (out == NULL) return NULL;
        out[0] = '\0';
        return out;
    }

    const char *end = s + strlen(s) - 1;
    while (end > start && is_whitespace(*end)) {
        end--;
    }

    size_t trimmed_len = (size_t)(end - start + 1);
    char *out = a.alloc_fn(trimmed_len + 1, a.ctx);
    if (out == NULL) return NULL;

    memcpy(out, start, trimmed_len);
    out[trimmed_len] = '\0';

    return out;
}

static char **split_impl(const char *s, const char *delim, size_t *out_count, zenit_allocator_t a) {
    if (s == NULL || delim == NULL || out_count == NULL) return NULL;

    size_t len = strlen(s);
    size_t cap = 4;
    size_t idx = 0;
    size_t token_start = 0;

    char **result = a.alloc_fn(cap * sizeof(char *), a.ctx);
    if (result == NULL) return NULL;

    for (size_t i = 0; i <= len; i++) {
        int is_delim = (i < len) && (strchr(delim, s[i]) != NULL);
        if (!is_delim && i != len) continue;

        if (idx + 2 > cap) {
            cap *= 2;
            size_t old_size = (cap / 2) * sizeof(char *);
            size_t new_size = cap * sizeof(char *);
            char **tmp = zenit_allocator_realloc(a, result, old_size, new_size);
            if (tmp == NULL) {
                free_split_result_with_allocator(result, idx, a);
                return NULL;
            }
            result = tmp;
        }

        size_t tlen = i - token_start;
        char *tok = a.alloc_fn(tlen + 1, a.ctx);
        if (tok == NULL) {
            free_split_result_with_allocator(result, idx, a);
            return NULL;
        }
        if (tlen > 0) {
            memcpy(tok, s + token_start, tlen);
        }
        tok[tlen] = '\0';
        result[idx++] = tok;
        token_start = i + 1;
    }

    result[idx] = NULL;
    *out_count = idx;
    return result;
}

static char *join_impl(const char **parts, size_t count, const char *delim, zenit_allocator_t a) {
    if (parts == NULL || delim == NULL) return NULL;
    if (count == 0) {
        char *out = a.alloc_fn(1, a.ctx);
        if (out == NULL) return NULL;
        out[0] = '\0';
        return out;
    }

    size_t delim_len = strlen(delim);

    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        if (parts[i] != NULL) {
            total += strlen(parts[i]);
        }
    }
    total += delim_len * (count - 1);
    total += 1;

    char *out = a.alloc_fn(total, a.ctx);
    if (out == NULL) return NULL;

    size_t pos = 0;
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            memcpy(out + pos, delim, delim_len);
            pos += delim_len;
        }
        if (parts[i] != NULL) {
            size_t plen = strlen(parts[i]);
            memcpy(out + pos, parts[i], plen);
            pos += plen;
        }
    }
    out[pos] = '\0';

    return out;
}

char *zenit_str_trim(const char *s) {
    return zenit_str_trim_with_allocator(s, ZENIT_ALLOCATOR_DEFAULT);
}

char *zenit_str_trim_with_allocator(const char *s, zenit_allocator_t allocator) {
    return trim_impl(s, allocator);
}

char **zenit_str_split(const char *s, const char *delim, size_t *out_count) {
    return zenit_str_split_with_allocator(s, delim, out_count, ZENIT_ALLOCATOR_DEFAULT);
}

char **zenit_str_split_with_allocator(const char *s, const char *delim, size_t *out_count, zenit_allocator_t allocator) {
    return split_impl(s, delim, out_count, allocator);
}

char *zenit_str_join(const char **parts, size_t count, const char *delim) {
    return zenit_str_join_with_allocator(parts, count, delim, ZENIT_ALLOCATOR_DEFAULT);
}

char *zenit_str_join_with_allocator(const char **parts, size_t count, const char *delim, zenit_allocator_t allocator) {
    return join_impl(parts, count, delim, allocator);
}
