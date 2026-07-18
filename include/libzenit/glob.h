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

#ifndef LIBZENIT_GLOB_H
#define LIBZENIT_GLOB_H

#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Match a string against a glob pattern.
 *
 * Supports the following wildcards:
 *   - `*`  Matches any sequence of characters (including empty).
 *   - `?`  Matches any single character.
 *   - `[abc]` Character class (matches any of 'a', 'b', 'c').
 *   - `[a-z]` Character range.
 *   - `[!abc]` Negated character class.
 *
 * Matching is case-sensitive.  Backslash is treated as a literal character.
 *
 * @param pattern  Glob pattern.  NULL matches nothing.
 * @param str      String to test.  NULL matches nothing.
 * @return 1 if the string matches the pattern, 0 otherwise.
 */
int zenit_glob_match(const char *pattern, const char *str);

#endif
