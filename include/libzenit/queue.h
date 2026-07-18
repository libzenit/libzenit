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

#ifndef LIBZENIT_QUEUE_H
#define LIBZENIT_QUEUE_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic FIFO queue.
 *
 * A thin wrapper around zenit_deque_t that exposes only queue operations:
 * enqueue (push back) and dequeue (pop front).  All memory management
 * is inherited from the underlying deque.
 */
typedef struct zenit_queue_t zenit_queue_t;

/**
 * @brief Create an empty queue with default initial capacity (8).
 *
 * @p elem_size must be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_queue_t *zenit_queue_create(size_t elem_size);

/**
 * @brief Create an empty queue with a custom allocator.
 *
 * @param elem_size Size in bytes of each element.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_queue_t *zenit_queue_create_with_allocator(size_t elem_size, zenit_allocator_t allocator);

/**
 * @brief Destroy a queue and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param queue Handle returned by zenit_queue_create(), or NULL.
 */
void zenit_queue_destroy(zenit_queue_t *queue);

/**
 * @brief Enqueue an element at the back of the queue.
 *
 * Grows the internal buffer if needed (1.5x factor).
 *
 * @param queue Queue handle.
 * @param elem  Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p queue or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_queue_enqueue(zenit_queue_t *queue, const void *elem);

/**
 * @brief Dequeue the front element from the queue.
 *
 * Copies the front element into @p out_elem and removes it.
 *
 * @param queue    Queue handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p queue or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the queue is empty.
 */
zenit_result_t zenit_queue_dequeue(zenit_queue_t *queue, void *out_elem);

/**
 * @brief Get a pointer to the front element without removing it.
 *
 * @param queue Queue handle.
 * @return Pointer to the front element, or NULL if empty or @p queue is NULL.
 */
void *zenit_queue_peek(const zenit_queue_t *queue);

/**
 * @brief Return the number of elements in the queue.
 *
 * @param queue Queue handle.
 * @return Element count (0 if @p queue is NULL).
 */
size_t zenit_queue_count(const zenit_queue_t *queue);

/**
 * @brief Check whether the queue is empty.
 *
 * @param queue Queue handle.
 * @return 1 if empty or @p queue is NULL, 0 otherwise.
 */
int zenit_queue_empty(const zenit_queue_t *queue);

/**
 * @brief Remove all elements without freeing the internal buffer.
 *
 * After this call count is 0 but capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param queue Queue handle, or NULL.
 */
void zenit_queue_clear(zenit_queue_t *queue);

#endif
