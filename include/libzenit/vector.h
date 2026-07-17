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

#ifndef LIBZENIT_VECTOR_H
#define LIBZENIT_VECTOR_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic dynamic array (vector).
 *
 * Stores elements of a fixed size in a contiguous buffer, growing
 * exponentially (1.5x factor) on demand.  Elements are copied by
 * memcpy — suitable for scalar types and small structs.
 */
typedef struct zenit_vector_t zenit_vector_t;

/**
 * @brief Create an empty vector with default initial capacity (8).
 *
 * @p elem_size must be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_vector_t *zenit_vector_create(size_t elem_size);

/**
 * @brief Create an empty vector with a specific initial capacity.
 *
 * @p elem_size and @p capacity must both be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @param capacity  Initial number of element slots to allocate.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_vector_t *zenit_vector_create_with_capacity(size_t elem_size, size_t capacity);

/**
 * @brief Create an empty vector with a custom allocator.
 *
 * The allocator is used for all memory operations (handle, buffer,
 * grow, shrink, destroy).
 *
 * @param elem_size Size in bytes of each element.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_vector_t *zenit_vector_create_with_allocator(size_t elem_size, zenit_allocator_t allocator);

/**
 * @brief Create an empty vector with a specific initial capacity and a custom allocator.
 *
 * @param elem_size Size in bytes of each element.
 * @param capacity  Initial number of element slots to allocate.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_vector_t *zenit_vector_create_with_capacity_and_allocator(size_t elem_size, size_t capacity, zenit_allocator_t allocator);

/**
 * @brief Destroy a vector and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param vector Handle returned by zenit_vector_create(), or NULL.
 */
void zenit_vector_destroy(zenit_vector_t *vector);

/**
 * @brief Append an element to the end of the vector.
 *
 * Grows the internal buffer if needed (1.5x factor).  The element is
 * copied by value via memcpy.
 *
 * @param vector Vector handle.
 * @param elem   Pointer to the element data to append (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_vector_push(zenit_vector_t *vector, const void *elem);

/**
 * @brief Remove and retrieve the last element.
 *
 * Copies the last element into @p out_elem and decrements the count.
 *
 * @param vector  Vector handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the vector is empty.
 */
zenit_result_t zenit_vector_pop(zenit_vector_t *vector, void *out_elem);

/**
 * @brief Insert an element at a given index.
 *
 * Shifts elements from @p index onward to the right by one slot.
 * Inserting at index == count is equivalent to push.
 *
 * @param vector Vector handle.
 * @param index  Position to insert at (0 … count, inclusive).
 * @param elem   Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector or @p elem is NULL
 *         - ZENIT_ERROR_PARAM if @p index > count
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_vector_insert(zenit_vector_t *vector, size_t index, const void *elem);

/**
 * @brief Remove and retrieve an element at a given index.
 *
 * Copies the element at @p index into @p out_elem and shifts remaining
 * elements left by one slot.
 *
 * @param vector  Vector handle.
 * @param index   Position to remove (0 … count - 1).
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector or @p out_elem is NULL
 *         - ZENIT_ERROR_PARAM if @p index >= count.
 */
zenit_result_t zenit_vector_remove(zenit_vector_t *vector, size_t index, void *out_elem);

/**
 * @brief Get a pointer to the element at a given index.
 *
 * Returns a direct pointer into the internal buffer — the caller may
 * read or write through it.  The pointer becomes invalid after any
 * operation that modifies the vector size or capacity.
 *
 * @param vector Vector handle.
 * @param index  Element index (0 … count - 1).
 * @return Pointer to the element, or NULL if @p vector is NULL or
 *         @p index is out of bounds.
 */
void *zenit_vector_get(const zenit_vector_t *vector, size_t index);

/**
 * @brief Overwrite the element at a given index with new data.
 *
 * @param vector Vector handle.
 * @param index  Element index (0 … count - 1).
 * @param elem   Pointer to the new element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector or @p elem is NULL
 *         - ZENIT_ERROR_PARAM if @p index >= count.
 */
zenit_result_t zenit_vector_set(zenit_vector_t *vector, size_t index, const void *elem);

/**
 * @brief Return the number of elements currently stored.
 *
 * @param vector Vector handle.
 * @return Element count (0 if @p vector is NULL).
 */
size_t zenit_vector_count(const zenit_vector_t *vector);

/**
 * @brief Return the current capacity (number of element slots allocated).
 *
 * @param vector Vector handle.
 * @return Capacity (0 if @p vector is NULL).
 */
size_t zenit_vector_capacity(const zenit_vector_t *vector);

/**
 * @brief Reserve capacity so that at least @p capacity elements can be
 *        stored without reallocation.
 *
 * If @p capacity is less than or equal to the current capacity, this is
 * a no-op.
 *
 * @param vector   Vector handle.
 * @param capacity Minimum desired capacity.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_vector_reserve(zenit_vector_t *vector, size_t capacity);

/**
 * @brief Shrink the internal buffer to exactly fit the current element count.
 *
 * If the vector is empty, this frees the buffer (capacity becomes 0).
 *
 * @param vector Vector handle.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p vector is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_vector_shrink_to_fit(zenit_vector_t *vector);

/**
 * @brief Remove all elements without freeing the internal buffer.
 *
 * After this call count is 0 but capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param vector Vector handle, or NULL.
 */
void zenit_vector_clear(zenit_vector_t *vector);

/**
 * @brief Check whether the vector is empty.
 *
 * @param vector Vector handle.
 * @return 1 if empty or @p vector is NULL, 0 otherwise.
 */
int zenit_vector_empty(const zenit_vector_t *vector);

#endif
