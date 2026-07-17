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
#include <stdlib.h>
#include <string.h>

static int is_whitespace(char c) {
    return c == ' '  || c == '\t' || c == '\n' ||
           c == '\r' || c == '\f' || c == '\v';
}

char *zenit_str_trim(const char *s) {
    if (s == NULL) return NULL;

    /* Find first non-whitespace character */
    const char *start = s;
    while (*start && is_whitespace(*start)) {
        start++;
    }

    /* If the string is all whitespace, return an empty string */
    if (*start == '\0') {
        char *out = malloc(1);
        if (out == NULL) return NULL;
        out[0] = '\0';
        return out;
    }

    /* Find last non-whitespace character */
    const char *end = s + strlen(s) - 1;
    while (end > start && is_whitespace(*end)) {
        end--;
    }

    size_t trimmed_len = (size_t)(end - start + 1);
    char *out = malloc(trimmed_len + 1);
    if (out == NULL) return NULL;

    memcpy(out, start, trimmed_len);
    out[trimmed_len] = '\0';

    return out;
}

char **zenit_str_split(const char *s, const char *delim, size_t *out_count) {
    if (s == NULL || delim == NULL || out_count == NULL) return NULL;

    size_t len = strlen(s);

    /* Count tokens: each delimiter (or end-of-string) terminates the
     * current token and starts a new one, so token count =
     * number of delimiter characters + 1 (even if the string is empty). */
    size_t tokens = 1;
    for (size_t i = 0; i < len; i++) {
        if (strchr(delim, s[i]) != NULL) {
            tokens++;
        }
    }

    /* Allocate the result array (NULL-terminated) */
    char **result = malloc((tokens + 1) * sizeof(char *));
    if (result == NULL) return NULL;

    size_t idx = 0;
    size_t token_start = 0;

    for (size_t i = 0; i <= len; i++) {
        int is_delim = (i < len) && (strchr(delim, s[i]) != NULL);
        if (is_delim || i == len) {
            /* Extract the token from token_start to i (exclusive) */
            size_t tlen = i - token_start;
            char *tok = malloc(tlen + 1);
            if (tok == NULL) {
                for (size_t j = 0; j < idx; j++) free(result[j]);
                free(result);
                return NULL;
            }
            if (tlen > 0) {
                memcpy(tok, s + token_start, tlen);
            }
            tok[tlen] = '\0';
            result[idx++] = tok;
            /* Next token starts after the delimiter */
            token_start = i + 1;
        }
    }

    result[idx] = NULL;
    *out_count = idx;
    return result;
}

char *zenit_str_join(const char **parts, size_t count, const char *delim) {
    if (parts == NULL || delim == NULL) return NULL;
    if (count == 0) {
        char *out = malloc(1);
        if (out == NULL) return NULL;
        out[0] = '\0';
        return out;
    }

    size_t delim_len = strlen(delim);

    /* Compute total length needed */
    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        if (parts[i] != NULL) {
            total += strlen(parts[i]);
        }
    }
    total += delim_len * (count - 1);
    total += 1; /* null terminator */

    char *out = malloc(total);
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
