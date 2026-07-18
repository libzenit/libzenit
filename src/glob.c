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

#include <libzenit/glob.h>
#include <stddef.h>

/*
 * Match a single character against a character class pattern [abc] or [a-z].
 * p points past the opening '['.  Returns 0 on mismatch, 1 on match,
 * and advances *p past the closing ']'.
 */
static int match_char_class(const char **p, char c) {
    int negated = 0;
    if (**p == '!') {
        negated = 1;
        (*p)++;
    }

    int matched = 0;
    while (**p != '\0' && **p != ']') {
        if (*(*p + 1) == '-' && *(*p + 2) != '\0' && *(*p + 2) != ']') {
            if (c >= **p && c <= *(*p + 2)) {
                matched = 1;
            }
            *p += 3;
        } else {
            if (c == **p) {
                matched = 1;
            }
            (*p)++;
        }
    }

    if (**p == ']') {
        (*p)++;
    }

    return (negated && !matched) || (!negated && matched) ? 1 : 0;
}

/*
 * Recursive glob matching engine with backtracking for '*'.
 */
static int match(const char *p, const char *s) {
    while (*p != '\0') {
        if (*p == '*') {
            p++;
            if (*p == '\0') {
                return 1;
            }
            while (*s != '\0') {
                if (match(p, s)) {
                    return 1;
                }
                s++;
            }
            return match(p, s);
        }

        if (*s == '\0') {
            return 0;
        }

        if (*p == '?') {
            p++;
            s++;
            continue;
        }

        if (*p == '[') {
            p++;
            if (!match_char_class(&p, *s)) {
                return 0;
            }
            s++;
            continue;
        }

        if (*p != *s) {
            return 0;
        }
        p++;
        s++;
    }

    return *s == '\0';
}

int zenit_glob_match(const char *pattern, const char *str) {
    if (pattern == NULL || str == NULL) {
        return 0;
    }
    return match(pattern, str) ? 1 : 0;
}
