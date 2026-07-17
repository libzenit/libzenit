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

#ifndef LIBZENIT_SET_H
#define LIBZENIT_SET_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic hash set (open-addressing, linear probing).
 *
 * Stores keys of a fixed size in a contiguous array, using FNV-1a hashing
 * with power-of-2 capacity and tombstones for deletion.  Automatically
 * rehashes when the load factor exceeds 75 %.
 */
typedef struct zenit_set_t zenit_set_t;

/**
 * @brief Create an empty hash set with default initial capacity (16).
 *
 * @p key_size must be > 0.
 *
 * @param key_size Size in bytes of each key element.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_set_t *zenit_set_create(size_t key_size);

/**
 * @brief Create an empty hash set with a specific initial capacity.
 *
 * The actual capacity is rounded up to the next power of two.
 * Both @p key_size and @p capacity must be > 0.
 *
 * @param key_size Size in bytes of each key element.
 * @param capacity Minimum initial slot count (> 0).
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_set_t *zenit_set_create_with_capacity(size_t key_size, size_t capacity);

/**
 * @brief Create a hash set with a custom memory allocator.
 *
 * Same as zenit_set_create() but uses @p allocator for all memory operations.
 *
 * @param key_size  Size in bytes of each key element.
 * @param allocator Custom allocator (use ZENIT_ALLOCATOR_DEFAULT for libc).
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_set_t *zenit_set_create_with_allocator(size_t key_size, zenit_allocator_t allocator);

/**
 * @brief Create a hash set with initial capacity and a custom allocator.
 *
 * Same as zenit_set_create_with_capacity() but uses @p allocator for all
 * memory operations.
 *
 * @param key_size  Size in bytes of each key element.
 * @param capacity  Minimum initial slot count (> 0).
 * @param allocator Custom allocator (use ZENIT_ALLOCATOR_DEFAULT for libc).
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_set_t *zenit_set_create_with_capacity_and_allocator(
    size_t key_size, size_t capacity, zenit_allocator_t allocator
);

/**
 * @brief Destroy a hash set and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param set Handle returned by zenit_set_create(), or NULL.
 */
void zenit_set_destroy(zenit_set_t *set);

/**
 * @brief Insert a key into the set.
 *
 * If the key already exists, this is a no-op (count unchanged).
 * Grows and rehashes automatically when the load factor exceeds 75 %.
 *
 * @param set Set handle.
 * @param key Pointer to the key data (must not be NULL, must be key_size bytes).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p set or @p key is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_set_insert(zenit_set_t *set, const void *key);

/**
 * @brief Remove a key from the set.
 *
 * Leaves a tombstone so existing probe chains are not broken.
 *
 * @param set Set handle.
 * @param key Pointer to the key data to remove (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p set or @p key is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the key is not in the set.
 */
zenit_result_t zenit_set_remove(zenit_set_t *set, const void *key);

/**
 * @brief Check whether a key is in the set.
 *
 * @param set Set handle.
 * @param key Pointer to the key data (must not be NULL).
 * @return 1 if the key is present, 0 otherwise (also 0 if @p set or @p key is NULL).
 */
int zenit_set_contains(const zenit_set_t *set, const void *key);

/**
 * @brief Return the number of keys stored.
 *
 * @param set Set handle.
 * @return Element count (0 if @p set is NULL).
 */
size_t zenit_set_count(const zenit_set_t *set);

/**
 * @brief Return the current slot capacity of the internal table.
 *
 * Always a power of two.
 *
 * @param set Set handle.
 * @return Capacity (0 if @p set is NULL).
 */
size_t zenit_set_capacity(const zenit_set_t *set);

/**
 * @brief Remove all entries without shrinking the internal table.
 *
 * Resets every slot to EMPTY — tombstones are cleared.  Capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param set Set handle, or NULL.
 */
void zenit_set_clear(zenit_set_t *set);

/**
 * @brief Visitor callback for zenit_set_foreach().
 *
 * @param key Pointer to the key data (key_size bytes).
 * @param ctx Opaque context pointer supplied by the caller.
 */
typedef void (*zenit_set_visit_fn_t)(const void *key, void *ctx);

/**
 * @brief Iterate over all keys in the set.
 *
 * The order is unspecified and depends on the internal slot layout.
 * The set must not be mutated during iteration.
 *
 * @param set   Set handle.
 * @param visit Visitor callback (must not be NULL).
 * @param ctx   Opaque context forwarded to @p visit on every call.
 */
void zenit_set_foreach(
    const zenit_set_t *set, zenit_set_visit_fn_t visit, void *ctx
);

/**
 * @brief Create an iterator for the set.
 *
 * The iterator must be advanced with zenit_set_iter_next().
 *
 * @param set Set handle.
 * @return An iterator (check is_valid).
 */
zenit_iter_t zenit_set_iter(const zenit_set_t *set);

/**
 * @brief Advance a set iterator to the next key.
 *
 * Returns a pointer to the key data (key_size bytes).  Skips empty and
 * deleted slots.
 *
 * @param iter Iterator created by zenit_set_iter().
 * @return Pointer to the key data, or NULL if iteration is complete.
 */
void *zenit_set_iter_next(zenit_iter_t *iter);

/**
 * @brief Collect all keys into a newly allocated array.
 *
 * The caller takes ownership of the returned buffer and must free it via
 * the set's allocator (allocator.free_fn(*out_keys, allocator.ctx)).
 *
 * @param set      Set handle.
 * @param out_keys On success, receives a pointer to the allocated key array.
 * @param out_count On success, receives the number of keys.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if any pointer argument is NULL
 *         - ZENIT_ERROR_ALLOC if allocation fails.
 */
zenit_result_t zenit_set_to_array(const zenit_set_t *set, void **out_keys, size_t *out_count);

#endif
