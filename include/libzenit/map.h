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

#ifndef LIBZENIT_MAP_H
#define LIBZENIT_MAP_H

#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic hash map (open-addressing, linear probing).
 *
 * Stores key-value pairs of fixed-size keys and values in a contiguous array.
 * Uses FNV-1a hashing with power-of-2 capacity and tombstones for deletion.
 * Automatically rehashes when the load factor exceeds 75 %.
 */
typedef struct zenit_map_t zenit_map_t;

/**
 * @brief Create an empty hash map with default initial capacity (16).
 *
 * Both @p key_size and @p value_size must be > 0.
 *
 * @param key_size   Size in bytes of each key element.
 * @param value_size Size in bytes of each value element.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_map_t *zenit_map_create(size_t key_size, size_t value_size);

/**
 * @brief Create an empty hash map with a specific initial capacity.
 *
 * The actual capacity is rounded up to the next power of two.
 * Both @p key_size and @p value_size must be > 0; @p capacity must be > 0.
 *
 * @param key_size   Size in bytes of each key element.
 * @param value_size Size in bytes of each value element.
 * @param capacity   Minimum initial slot count (> 0).
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_map_t *zenit_map_create_with_capacity(
    size_t key_size, size_t value_size, size_t capacity
);

/**
 * @brief Destroy a hash map and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param map Handle returned by zenit_map_create(), or NULL.
 */
void zenit_map_destroy(zenit_map_t *map);

/**
 * @brief Insert or overwrite a key-value pair.
 *
 * If the key already exists, its value is overwritten.  Grows and rehashes
 * automatically when the load factor exceeds 75 %.
 *
 * @param map   Map handle.
 * @param key   Pointer to the key data (must not be NULL, must be key_size bytes).
 * @param value Pointer to the value data (must not be NULL, must be value_size bytes).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p map, @p key, or @p value is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_map_insert(
    zenit_map_t *map, const void *key, const void *value
);

/**
 * @brief Retrieve the value for a key.
 *
 * @param map      Map handle.
 * @param key      Pointer to the key data to look up (must not be NULL).
 * @param out_value Buffer to receive the value (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p map, @p key, or @p out_value is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the key is not in the map.
 */
zenit_result_t zenit_map_get(
    const zenit_map_t *map, const void *key, void *out_value
);

/**
 * @brief Remove a key-value pair.
 *
 * Leaves a tombstone so existing probe chains are not broken.
 *
 * @param map Map handle.
 * @param key Pointer to the key data to remove (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p map or @p key is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the key is not in the map.
 */
zenit_result_t zenit_map_remove(zenit_map_t *map, const void *key);

/**
 * @brief Check whether a key exists in the map.
 *
 * @param map Map handle.
 * @param key Pointer to the key data (must not be NULL).
 * @return 1 if the key is present, 0 otherwise (also 0 if @p map or @p key is NULL).
 */
int zenit_map_contains(const zenit_map_t *map, const void *key);

/**
 * @brief Return the number of key-value pairs stored.
 *
 * @param map Map handle.
 * @return Element count (0 if @p map is NULL).
 */
size_t zenit_map_count(const zenit_map_t *map);

/**
 * @brief Return the current slot capacity of the internal table.
 *
 * Always a power of two.
 *
 * @param map Map handle.
 * @return Capacity (0 if @p map is NULL).
 */
size_t zenit_map_capacity(const zenit_map_t *map);

/**
 * @brief Remove all entries without shrinking the internal table.
 *
 * Resets every slot to EMPTY — tombstones are cleared.  Capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param map Map handle, or NULL.
 */
void zenit_map_clear(zenit_map_t *map);

/**
 * @brief Visitor callback for zenit_map_foreach().
 *
 * @param key   Pointer to the key data (key_size bytes).
 * @param value Pointer to the value data (value_size bytes).
 * @param ctx   Opaque context pointer supplied by the caller.
 */
typedef void (*zenit_map_visit_fn_t)(const void *key, const void *value, void *ctx);

/**
 * @brief Iterate over all key-value pairs in the map.
 *
 * The order is unspecified and depends on the internal slot layout.
 * The map must not be mutated during iteration.
 *
 * @param map   Map handle.
 * @param visit Visitor callback (must not be NULL).
 * @param ctx   Opaque context forwarded to @p visit on every call.
 */
void zenit_map_foreach(
    const zenit_map_t *map, zenit_map_visit_fn_t visit, void *ctx
);

#endif
