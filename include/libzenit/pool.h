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

#ifndef LIBZENIT_POOL_H
#define LIBZENIT_POOL_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a fixed-capacity object pool.
 *
 * Pre-allocates a contiguous array of @p capacity objects, each of
 * @p object_size bytes.  Objects are acquired on demand and returned
 * to the pool when released.  No heap allocation occurs after creation
 * (all memory is allocated up front).
 *
 * Internally uses a free-list (array of indices) to track available slots.
 * Acquire is O(1), release is O(1).  Double-release is detected and
 * rejected.
 */
typedef struct zenit_pool_t zenit_pool_t;

/**
 * @brief Create an object pool with the given capacity.
 *
 * Allocates a contiguous block for @p capacity objects of @p object_size
 * bytes each, plus internal free-list metadata.
 *
 * Both @p object_size and @p capacity must be > 0.
 *
 * @param object_size Size in bytes of each pooled object.
 * @param capacity    Maximum number of objects the pool can hold.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_pool_t *zenit_pool_create(size_t object_size, size_t capacity);

/**
 * @brief Create an object pool with a custom allocator.
 *
 * @param object_size Size in bytes of each pooled object.
 * @param capacity    Maximum number of objects the pool can hold.
 * @param allocator   Custom allocator for all internal memory.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_pool_t *zenit_pool_create_with_allocator(size_t object_size, size_t capacity, zenit_allocator_t allocator);

/**
 * @brief Destroy a pool and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param pool Pool handle, or NULL.
 */
void zenit_pool_destroy(zenit_pool_t *pool);

/**
 * @brief Acquire an object from the pool.
 *
 * Returns a pointer to a pre-allocated object.  The caller may read/write
 * through this pointer.  The object's contents are unspecified — the caller
 * should initialise before use.
 *
 * @param pool Pool handle.
 * @return Pointer to the acquired object, or NULL if the pool is exhausted
 *         or @p pool is NULL.
 */
void *zenit_pool_acquire(zenit_pool_t *pool);

/**
 * @brief Release an object back to the pool.
 *
 * The object pointer must have been returned by a previous call to
 * zenit_pool_acquire() and must not have been released already.
 *
 * @param pool Pool handle.
 * @param obj  Pointer previously returned by zenit_pool_acquire().
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p pool or @p obj is NULL
 *         - ZENIT_ERROR_DOUBLE_FREE if the object was already released
 *         - ZENIT_ERROR_PARAM if @p obj does not belong to this pool.
 */
zenit_result_t zenit_pool_release(zenit_pool_t *pool, void *obj);

/**
 * @brief Return the number of objects currently acquired.
 *
 * @param pool Pool handle.
 * @return Number of acquired objects (0 if @p pool is NULL).
 */
size_t zenit_pool_count(const zenit_pool_t *pool);

/**
 * @brief Return the maximum number of objects the pool can hold.
 *
 * @param pool Pool handle.
 * @return Total capacity (0 if @p pool is NULL).
 */
size_t zenit_pool_capacity(const zenit_pool_t *pool);

/**
 * @brief Return the number of objects currently available (not acquired).
 *
 * @param pool Pool handle.
 * @return Available count (0 if @p pool is NULL).
 */
size_t zenit_pool_available(const zenit_pool_t *pool);

/**
 * @brief Release all acquired objects, resetting the pool to its initial state.
 *
 * After this call, all objects are available for acquisition again.
 * Passing NULL is safe and is a no-op.
 *
 * @param pool Pool handle, or NULL.
 */
void zenit_pool_clear(zenit_pool_t *pool);

#endif
