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

#include <libzenit/allocator.h>
#include <stddef.h>

/**
 * @brief Remove leading and trailing whitespace from a string (default allocator).
 *
 * Whitespace characters are: space (' '), tab ('\\t'), newline ('\\n'),
 * carriage return ('\\r'), form-feed ('\\f'), and vertical tab ('\\v').
 *
 * @param s Null-terminated string to trim.
 * @return A newly allocated trimmed copy, or NULL on allocation failure.
 */
char *zenit_str_trim(const char *s);

/**
 * @brief Remove leading and trailing whitespace (custom allocator).
 *
 * @param s         Null-terminated string to trim.
 * @param allocator Custom allocator for the output string.
 * @return A newly allocated trimmed copy, or NULL on allocation failure.
 */
char *zenit_str_trim_with_allocator(const char *s, zenit_allocator_t allocator);

/**
 * @brief Split a string into substrings at each delimiter occurrence (default allocator).
 *
 * Splits @p s at every occurrence of @p delim, returning an array of
 * newly allocated (null-terminated) substrings.  Consecutive delimiters
 * produce empty-string entries.  The array itself is malloc'd and the
 * caller must free each element, then free the array.
 *
 * @param s         Null-terminated string to split.
 * @param delim     Null-terminated delimiter string (each character is a separator).
 * @param out_count On success, receives the number of substrings.
 * @return Allocated NULL-terminated array of allocated strings, or NULL
 *         on allocation failure.
 */
char **zenit_str_split(const char *s, const char *delim, size_t *out_count);

/**
 * @brief Split a string into substrings (custom allocator).
 *
 * @param s         Null-terminated string to split.
 * @param delim     Null-terminated delimiter string (each character is a separator).
 * @param out_count On success, receives the number of substrings.
 * @param allocator Custom allocator for all allocations.
 * @return Allocated NULL-terminated array of allocated strings, or NULL
 *         on allocation failure.
 */
char **zenit_str_split_with_allocator(const char *s, const char *delim, size_t *out_count, zenit_allocator_t allocator);

/**
 * @brief Join an array of strings with a delimiter (default allocator).
 *
 * @param parts Array of null-terminated strings.
 * @param count Number of elements in @p parts.
 * @param delim Null-terminated delimiter string.
 * @return Joined string, or NULL on allocation failure.
 */
char *zenit_str_join(const char **parts, size_t count, const char *delim);

/**
 * @brief Join an array of strings with a delimiter (custom allocator).
 *
 * @param parts     Array of null-terminated strings.
 * @param count     Number of elements in @p parts.
 * @param delim     Null-terminated delimiter string.
 * @param allocator Custom allocator for the output string.
 * @return Joined string, or NULL on allocation failure.
 */
char *zenit_str_join_with_allocator(const char **parts, size_t count, const char *delim, zenit_allocator_t allocator);

#endif
