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

#ifndef LIBZENIT_STRING_H
#define LIBZENIT_STRING_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a dynamic string (string builder).
 *
 * Stores a null-terminated C string in a contiguous buffer that grows
 * exponentially (1.5x) on demand.  Internally backed by zenit_vector_t.
 * All operations maintain the null terminator.
 */
typedef struct zenit_string_t zenit_string_t;

/**
 * @brief Create an empty string.
 *
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_string_t *zenit_string_create(void);

/**
 * @brief Create an empty string with a custom allocator.
 *
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_string_t *zenit_string_create_with_allocator(zenit_allocator_t allocator);

/**
 * @brief Create a string from a null-terminated C string.
 *
 * The input @p cstr is copied; the returned handle owns its storage.
 *
 * @param cstr Null-terminated C string to copy (may be NULL, treated as empty).
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_string_t *zenit_string_create_from(const char *cstr);

/**
 * @brief Create a string from a null-terminated C string with a custom allocator.
 *
 * @param cstr      Null-terminated C string to copy (may be NULL, treated as empty).
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_string_t *zenit_string_create_from_with_allocator(const char *cstr, zenit_allocator_t allocator);

/**
 * @brief Destroy a string and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param s String handle, or NULL.
 */
void zenit_string_destroy(zenit_string_t *s);

/**
 * @brief Append raw data to the end of the string.
 *
 * Grows the internal buffer if needed.  The null terminator is
 * automatically maintained.
 *
 * @param s    String handle.
 * @param data Pointer to the data to append (must not be NULL if @p len > 0).
 * @param len  Number of bytes to append.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p s is NULL, or @p data is NULL with @p len > 0
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_string_append(zenit_string_t *s, const void *data, size_t len);

/**
 * @brief Append a null-terminated C string.
 *
 * Convenience wrapper around zenit_string_append() that uses strlen().
 *
 * @param s    String handle.
 * @param cstr Null-terminated C string to append (may be NULL, treated as empty).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p s is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_string_append_cstr(zenit_string_t *s, const char *cstr);

/**
 * @brief Return a pointer to the internal null-terminated C string.
 *
 * The returned pointer becomes invalid after any operation that modifies
 * the string.  The caller must not free it.
 *
 * @param s String handle.
 * @return Pointer to the internal C string, or NULL if @p s is NULL.
 */
const char *zenit_string_cstr(const zenit_string_t *s);

/**
 * @brief Return the length of the string, excluding the null terminator.
 *
 * @param s String handle.
 * @return String length (0 if @p s is NULL or the string is empty).
 */
size_t zenit_string_length(const zenit_string_t *s);

/**
 * @brief Return the current capacity (total allocated bytes, including
 *        space for the null terminator).
 *
 * @param s String handle.
 * @return Capacity in bytes (0 if @p s is NULL).
 */
size_t zenit_string_capacity(const zenit_string_t *s);

/**
 * @brief Reset the string to empty (length 0).
 *
 * The internal buffer is not freed — capacity is preserved for reuse.
 * Passing NULL is safe and is a no-op.
 *
 * @param s String handle, or NULL.
 */
void zenit_string_clear(zenit_string_t *s);

/**
 * @brief Reserve capacity so that at least @p capacity bytes can be stored
 *        without reallocation (including the null terminator).
 *
 * If @p capacity is less than or equal to the current capacity, this is
 * a no-op.  The null terminator is preserved.
 *
 * @param s        String handle.
 * @param capacity Minimum desired capacity in bytes.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p s is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_string_reserve(zenit_string_t *s, size_t capacity);

/**
 * @brief Shrink the internal buffer to exactly fit the current string content
 *        plus the null terminator.
 *
 * If the string is empty, the buffer is freed (capacity becomes 1 byte for
 * the null terminator).
 *
 * @param s String handle.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p s is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_string_shrink_to_fit(zenit_string_t *s);

/**
 * @brief Check whether the string is empty.
 *
 * @param s String handle.
 * @return 1 if empty or @p s is NULL, 0 otherwise.
 */
int zenit_string_empty(const zenit_string_t *s);

#endif
