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
 * Recursive glob matching engine.  This implementation uses a
 * recursive-descent parser for character classes and a backtracking
 * approach for '*' (tries to match the rest of the pattern against
 * each suffix of the remaining string).
 */
static int match(const char *p, const char *s) {
    /* When the pattern is exhausted, the string must also be exhausted */
    while (*p != '\0') {
        if (*p == '*') {
            /* '*' matches any sequence — try matching the remaining pattern
             * against each suffix of the input string */
            p++;
            if (*p == '\0') {
                /* Trailing '*' matches everything */
                return 1;
            }
            /* Try every suffix */
            while (*s != '\0') {
                if (match(p, s)) {
                    return 1;
                }
                s++;
            }
            /* Couldn't match the rest of the pattern — try empty suffix */
            return match(p, s);
        }

        if (*s == '\0') {
            /* String exhausted but pattern has more non-'*' characters */
            return 0;
        }

        if (*p == '?') {
            /* '?' matches any single character */
            p++;
            s++;
            continue;
        }

        if (*p == '[') {
            /* Character class / range */
            p++;
            int negated = 0;
            if (*p == '!') {
                negated = 1;
                p++;
            }

            int matched = 0;
            while (*p != '\0' && *p != ']') {
                if (*(p + 1) == '-' && *(p + 2) != '\0' && *(p + 2) != ']') {
                    /* Range like a-z */
                    if (*s >= *p && *s <= *(p + 2)) {
                        matched = 1;
                    }
                    p += 3;
                } else {
                    /* Single character */
                    if (*s == *p) {
                        matched = 1;
                    }
                    p++;
                }
            }

            if (*p == ']') {
                p++;
            }

            if ((negated && matched) || (!negated && !matched)) {
                return 0;
            }
            s++;
            continue;
        }

        /* Literal character */
        if (*p != *s) {
            return 0;
        }
        p++;
        s++;
    }

    /* Pattern exhausted — string must also be exhausted */
    return *s == '\0';
}

int zenit_glob_match(const char *pattern, const char *str) {
    if (pattern == NULL || str == NULL) {
        return 0;
    }
    return match(pattern, str) ? 1 : 0;
}
