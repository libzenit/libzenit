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

#ifndef LIBZENIT_STR_H
#define LIBZENIT_STR_H

#include <stddef.h>

/**
 * @brief Remove leading and trailing whitespace from a string.
 *
 * Whitespace characters are: space (' '), tab ('\\t'), newline ('\\n'),
 * carriage return ('\\r'), form-feed ('\\f'), and vertical tab ('\\v').
 *
 * Allocates a new null-terminated string via malloc().  The caller owns
 * the returned pointer and must free() it.  Returns NULL on allocation
 * failure.
 *
 * @param s Null-terminated string to trim.
 * @return A newly allocated trimmed copy, or NULL on allocation failure.
 */
char *zenit_str_trim(const char *s);

/**
 * @brief Split a string into substrings at each delimiter occurrence.
 *
 * Splits @p s at every occurrence of @p delim, returning an array of
 * newly allocated (null-terminated) substrings.  Consecutive delimiters
 * produce empty-string entries.  The array itself is malloc'd and the
 * caller must free each element, then free the array.
 *
 * @param s       Null-terminated string to split.
 * @param delim   Null-terminated delimiter string (each character is a
 *                separator, i.e. strtok-style multi-character delimiter set).
 * @param out_count On success, receives the number of substrings.
 * @return Allocated NULL-terminated array of allocated strings, or NULL
 *         on allocation failure.
 */
char **zenit_str_split(const char *s, const char *delim, size_t *out_count);

/**
 * @brief Join an array of strings with a delimiter.
 *
 * Concatenates all strings in @p parts separated by @p delim.
 *
 * Allocates a new null-terminated string via malloc().  The caller owns
 * the returned pointer and must free() it.
 *
 * @param parts Array of null-terminated strings.
 * @param count Number of elements in @p parts.
 * @param delim Null-terminated delimiter string.
 * @return Joined string, or NULL on allocation failure.
 */
char *zenit_str_join(const char **parts, size_t count, const char *delim);

#endif
