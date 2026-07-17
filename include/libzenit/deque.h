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

#ifndef LIBZENIT_DEQUE_H
#define LIBZENIT_DEQUE_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic double-ended queue (deque).
 *
 * Elements are stored in a contiguous circular buffer.  Push and pop at
 * both ends are amortized O(1).  The buffer grows by 1.5x on demand.
 */
typedef struct zenit_deque_t zenit_deque_t;

/**
 * @brief Create an empty deque with default initial capacity (8).
 *
 * @p elem_size must be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_deque_t *zenit_deque_create(size_t elem_size);

/**
 * @brief Create an empty deque with a specific initial capacity.
 *
 * @p elem_size and @p capacity must both be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @param capacity  Initial number of element slots to allocate.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_deque_t *zenit_deque_create_with_capacity(size_t elem_size, size_t capacity);

/**
 * @brief Create an empty deque with a custom allocator.
 *
 * Default initial capacity (8).
 *
 * @param elem_size Size in bytes of each element.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_deque_t *zenit_deque_create_with_allocator(size_t elem_size, zenit_allocator_t allocator);

/**
 * @brief Create an empty deque with a specific initial capacity and a custom allocator.
 *
 * @param elem_size Size in bytes of each element.
 * @param capacity  Initial number of element slots to allocate.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_deque_t *zenit_deque_create_with_capacity_and_allocator(size_t elem_size, size_t capacity, zenit_allocator_t allocator);

/**
 * @brief Destroy a deque and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param deque Handle returned by zenit_deque_create(), or NULL.
 */
void zenit_deque_destroy(zenit_deque_t *deque);

/**
 * @brief Prepend an element to the deque (amortized O(1)).
 *
 * Grows the internal buffer if full.
 *
 * @param deque Deque handle.
 * @param elem  Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p deque or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_deque_push_front(zenit_deque_t *deque, const void *elem);

/**
 * @brief Append an element to the deque (amortized O(1)).
 *
 * Grows the internal buffer if full.
 *
 * @param deque Deque handle.
 * @param elem  Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p deque or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_deque_push_back(zenit_deque_t *deque, const void *elem);

/**
 * @brief Remove and retrieve the first element (amortized O(1)).
 *
 * @param deque    Deque handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p deque or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the deque is empty.
 */
zenit_result_t zenit_deque_pop_front(zenit_deque_t *deque, void *out_elem);

/**
 * @brief Remove and retrieve the last element (amortized O(1)).
 *
 * @param deque    Deque handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p deque or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the deque is empty.
 */
zenit_result_t zenit_deque_pop_back(zenit_deque_t *deque, void *out_elem);

/**
 * @brief Get a pointer to the element at a given index (O(1)).
 *
 * Index 0 is the front, index count-1 is the back.  The pointer becomes
 * invalid after any mutation that changes the deque size or capacity.
 *
 * @param deque Deque handle.
 * @param index Element index (0 … count - 1).
 * @return Pointer to the element data, or NULL if @p deque is NULL or
 *         @p index is out of bounds.
 */
void *zenit_deque_get(const zenit_deque_t *deque, size_t index);

/**
 * @brief Get a pointer to the first element (O(1)).
 *
 * @param deque Deque handle.
 * @return Pointer to the first element, or NULL if empty or @p deque is NULL.
 */
void *zenit_deque_front(const zenit_deque_t *deque);

/**
 * @brief Get a pointer to the last element (O(1)).
 *
 * @param deque Deque handle.
 * @return Pointer to the last element, or NULL if empty or @p deque is NULL.
 */
void *zenit_deque_back(const zenit_deque_t *deque);

/**
 * @brief Return the number of elements stored.
 *
 * @param deque Deque handle.
 * @return Element count (0 if @p deque is NULL).
 */
size_t zenit_deque_count(const zenit_deque_t *deque);

/**
 * @brief Return the current capacity.
 *
 * @param deque Deque handle.
 * @return Capacity (0 if @p deque is NULL).
 */
size_t zenit_deque_capacity(const zenit_deque_t *deque);

/**
 * @brief Check whether the deque is empty.
 *
 * @param deque Deque handle.
 * @return 1 if empty or @p deque is NULL, 0 otherwise.
 */
int zenit_deque_empty(const zenit_deque_t *deque);

/**
 * @brief Reserve capacity so that at least @p capacity elements can be
 *        stored without reallocation.
 *
 * If @p capacity is less than or equal to the current capacity, this is
 * a no-op.
 *
 * @param deque    Deque handle.
 * @param capacity Minimum desired capacity.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p deque is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_deque_reserve(zenit_deque_t *deque, size_t capacity);

/**
 * @brief Shrink the internal buffer to exactly fit the current element count.
 *
 * If the deque is empty, this frees the buffer (capacity becomes 0).
 *
 * @param deque Deque handle.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p deque is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_deque_shrink_to_fit(zenit_deque_t *deque);

/**
 * @brief Remove all elements without freeing the internal buffer.
 *
 * After this call count is 0 but capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param deque Deque handle, or NULL.
 */
void zenit_deque_clear(zenit_deque_t *deque);

#endif
