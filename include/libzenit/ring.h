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

#ifndef LIBZENIT_RING_H
#define LIBZENIT_RING_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a byte-level ring (circular) buffer.
 *
 * A fixed-capacity FIFO buffer that overwrites nothing — push fails when
 * full, pop fails when empty.  Useful for producer-consumer patterns,
 * buffered I/O, and event queues.
 *
 * Memory is owned internally and freed on destroy.  Thread safety is
 * NOT provided — the caller must synchronise access.
 */
typedef struct zenit_ring_t zenit_ring_t;

/**
 * @brief Create a ring buffer with the given byte capacity.
 *
 * @p capacity must be > 0.
 *
 * @param capacity Maximum number of bytes the buffer can hold.
 * @return Opaque handle, or NULL on allocation failure or invalid capacity.
 */
zenit_ring_t *zenit_ring_create(size_t capacity);

/**
 * @brief Create a ring buffer with a custom allocator.
 *
 * @p capacity must be > 0.
 *
 * @param capacity  Maximum number of bytes the buffer can hold.
 * @param allocator Custom allocator to use for all internal memory.
 * @return Opaque handle, or NULL on allocation failure or invalid capacity.
 */
zenit_ring_t *zenit_ring_create_with_allocator(size_t capacity, zenit_allocator_t allocator);

/**
 * @brief Destroy a ring buffer and free its memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param ring Handle returned by zenit_ring_create(), or NULL.
 */
void zenit_ring_destroy(zenit_ring_t *ring);

/**
 * @brief Push data into the ring buffer.
 *
 * Copies @p size bytes from @p data into the buffer.  If there is not
 * enough room the buffer is left unchanged and ZENIT_ERROR_FULL is returned.
 *
 * @param ring Ring buffer handle.
 * @param data Pointer to the bytes to copy (must not be NULL if size > 0).
 * @param size Number of bytes to push (must be > 0).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p ring or @p data is NULL
 *         - ZENIT_ERROR_PARAM if @p size == 0
 *         - ZENIT_ERROR_FULL if not enough free space remains.
 */
zenit_result_t zenit_ring_push(zenit_ring_t *ring, const void *data, size_t size);

/**
 * @brief Pop data from the ring buffer (oldest bytes first).
 *
 * Copies up to @p size bytes from the buffer into @p data and advances
 * the read cursor.  On success @p *size is updated to the number of bytes
 * actually copied (always > 0).
 *
 * @param ring Ring buffer handle.
 * @param data Buffer to receive the bytes (must not be NULL if *size > 0).
 * @param size On input: the capacity of @p data.  On output: the number
 *             of bytes actually copied.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p ring or @p data is NULL
 *         - ZENIT_ERROR_PARAM if @p size == NULL or *size == 0
 *         - ZENIT_ERROR_EMPTY if the buffer contains no data.
 */
zenit_result_t zenit_ring_pop(zenit_ring_t *ring, void *data, size_t *size);

/**
 * @brief Peek at data without removing it from the buffer.
 *
 * Same as zenit_ring_pop() but the read cursor is not advanced.
 *
 * @param ring Ring buffer handle.
 * @param data Buffer to receive the bytes (must not be NULL if *size > 0).
 * @param size On input: the capacity of @p data.  On output: the number
 *             of bytes actually copied.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p ring or @p data is NULL
 *         - ZENIT_ERROR_PARAM if @p size == NULL or *size == 0
 *         - ZENIT_ERROR_EMPTY if the buffer contains no data.
 */
zenit_result_t zenit_ring_peek(const zenit_ring_t *ring, void *data, size_t *size);

/**
 * @brief Return the number of bytes currently in the buffer.
 *
 * @param ring Ring buffer handle.
 * @return Byte count (0 if @p ring is NULL).
 */
size_t zenit_ring_count(const zenit_ring_t *ring);

/**
 * @brief Return the total byte capacity of the buffer.
 *
 * @param ring Ring buffer handle.
 * @return Capacity (0 if @p ring is NULL).
 */
size_t zenit_ring_capacity(const zenit_ring_t *ring);

/**
 * @brief Remove all data from the buffer.
 *
 * After this call zenit_ring_count() returns 0.
 *
 * @param ring Ring buffer handle (NULL is accepted and ignored).
 */
void zenit_ring_clear(zenit_ring_t *ring);

/**
 * @brief Resize the ring buffer to a new capacity.
 *
 * Existing data is preserved as much as possible (up to @p new_capacity bytes).
 * If @p new_capacity is smaller than the current count, the oldest bytes are
 * discarded and the count is clamped.
 *
 * @param ring         Ring buffer handle.
 * @param new_capacity New desired capacity (must be > 0).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p ring is NULL
 *         - ZENIT_ERROR_PARAM if @p new_capacity == 0
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_ring_reserve(zenit_ring_t *ring, size_t new_capacity);

/**
 * @brief Shrink the internal buffer to exactly fit the current data.
 *
 * After this call the capacity equals the count (or 1 if empty).
 *
 * @param ring Ring buffer handle.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p ring is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_ring_shrink_to_fit(zenit_ring_t *ring);

#endif
