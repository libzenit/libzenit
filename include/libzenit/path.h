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

#ifndef LIBZENIT_PATH_H
#define LIBZENIT_PATH_H

#include <libzenit/allocator.h>
#include <stddef.h>

/**
 * @brief Join two path components with a separator (default allocator).
 *
 * Ensures exactly one '/' separates @p a and @p b, even if one or both
 * already contain trailing/leading slashes.
 *
 * @param a First path component.
 * @param b Second path component.
 * @return Newly allocated joined path, or NULL on allocation failure or NULL input.
 */
char* zenit_path_join(const char *a, const char *b);

/**
 * @brief Join two path components with a separator (custom allocator).
 *
 * @param a         First path component.
 * @param b         Second path component.
 * @param allocator Custom allocator for the output string.
 * @return Newly allocated joined path, or NULL on allocation failure or NULL input.
 */
char* zenit_path_join_with_allocator(const char *a, const char *b, zenit_allocator_t allocator);

/**
 * @brief Return the directory portion of a path (default allocator).
 *
 * Returns everything up to (and including) the last '/', or "." if there
 * is no slash.  For "/foo" returns "/".
 *
 * @param path Null-terminated path string.
 * @return Newly allocated directory string, or NULL on allocation failure or NULL input.
 */
char* zenit_path_dirname(const char *path);

/**
 * @brief Return the directory portion of a path (custom allocator).
 *
 * @param path      Null-terminated path string.
 * @param allocator Custom allocator for the output string.
 * @return Newly allocated directory string, or NULL on allocation failure or NULL input.
 */
char* zenit_path_dirname_with_allocator(const char *path, zenit_allocator_t allocator);

/**
 * @brief Return the final component of a path (default allocator).
 *
 * Returns everything after the last '/', or the whole string if there is
 * no slash.  For "/" returns "/".
 *
 * @param path Null-terminated path string.
 * @return Newly allocated basename string, or NULL on allocation failure or NULL input.
 */
char* zenit_path_basename(const char *path);

/**
 * @brief Return the final component of a path (custom allocator).
 *
 * @param path      Null-terminated path string.
 * @param allocator Custom allocator for the output string.
 * @return Newly allocated basename string, or NULL on allocation failure or NULL input.
 */
char* zenit_path_basename_with_allocator(const char *path, zenit_allocator_t allocator);

/**
 * @brief Return the file extension from a path (default allocator).
 *
 * Returns everything after the last '.' in the basename, including the dot.
 * If there is no dot, returns an empty string.
 *
 * @param path Null-terminated path string.
 * @return Newly allocated extension string (e.g. ".txt"), or NULL on allocation failure or NULL input.
 */
char* zenit_path_extension(const char *path);

/**
 * @brief Return the file extension from a path (custom allocator).
 *
 * @param path      Null-terminated path string.
 * @param allocator Custom allocator for the output string.
 * @return Newly allocated extension string, or NULL on allocation failure or NULL input.
 */
char* zenit_path_extension_with_allocator(const char *path, zenit_allocator_t allocator);

/**
 * @brief Normalise a path by resolving '.' and '..', collapsing double
 *        slashes, and removing trailing slashes (default allocator).
 *
 * Leading ".." components are preserved.  An empty or root-normalised path
 * returns "/".
 *
 * @param path Null-terminated path string.
 * @return Newly allocated normalised path, or NULL on allocation failure or NULL input.
 */
char* zenit_path_normalize(const char *path);

/**
 * @brief Normalise a path with a custom allocator.
 *
 * @param path      Null-terminated path string.
 * @param allocator Custom allocator for the output string.
 * @return Newly allocated normalised path, or NULL on allocation failure or NULL input.
 */
char* zenit_path_normalize_with_allocator(const char *path, zenit_allocator_t allocator);

/**
 * @brief Convert forward slashes to platform-native separators (default allocator).
 *
 * On Windows, replaces '/' with '\\'.  On other platforms, returns a copy
 * of the input unchanged.
 *
 * @param path Null-terminated path string.
 * @return Newly allocated path with native separators, or NULL on allocation failure or NULL input.
 */
char* zenit_path_to_native(const char *path);

/**
 * @brief Convert forward slashes to platform-native separators (custom allocator).
 *
 * @param path      Null-terminated path string.
 * @param allocator Custom allocator for the output string.
 * @return Newly allocated path with native separators, or NULL on allocation failure or NULL input.
 */
char* zenit_path_to_native_with_allocator(const char *path, zenit_allocator_t allocator);

#endif
