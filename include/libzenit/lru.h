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

#ifndef LIBZENIT_LRU_H
#define LIBZENIT_LRU_H

#include <libzenit/result.h>
#include <libzenit/allocator.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque handle to a fixed-capacity LRU (Least Recently Used) cache.
 *
 * Each entry stores a copy of both key and value.  When the cache reaches
 * capacity, inserting a new entry evicts the least recently used entry.
 * An optional eviction callback notifies the caller when an entry is
 * evicted.
 */
typedef struct zenit_lru_t zenit_lru_t;

/**
 * @brief Eviction callback invoked when an entry is evicted from the cache.
 *
 * Called just before the entry's storage is reused.  The key and value
 * pointers are valid only during the callback invocation.  The callback
 * must not modify the cache.
 *
 * @param key   Pointer to the evicted entry's key data.
 * @param value Pointer to the evicted entry's value data.
 * @param ctx   User-supplied context from cache creation.
 */
typedef void (*zenit_lru_evict_fn_t)(const void *key, void *value, void *ctx);

/**
 * @brief Create an LRU cache with the default allocator.
 *
 * @param key_size   Size in bytes of each key (must be > 0).
 * @param value_size Size in bytes of each value (must be > 0).
 * @param capacity   Maximum number of entries (must be > 0).
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_lru_t* zenit_lru_create(size_t key_size, size_t value_size, size_t capacity);

/**
 * @brief Create an LRU cache with a custom allocator.
 *
 * @param key_size   Size in bytes of each key.
 * @param value_size Size in bytes of each value.
 * @param capacity   Maximum number of entries.
 * @param allocator  Custom allocator for all internal memory.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_lru_t* zenit_lru_create_with_allocator(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_allocator_t allocator
);

/**
 * @brief Create an LRU cache with an eviction callback (default allocator).
 *
 * @param key_size   Size in bytes of each key.
 * @param value_size Size in bytes of each value.
 * @param capacity   Maximum number of entries.
 * @param evict_fn   Callback invoked on eviction (may be NULL).
 * @param evict_ctx  Context passed to @p evict_fn.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_lru_t* zenit_lru_create_with_evict(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_lru_evict_fn_t evict_fn,
    void *evict_ctx
);

/**
 * @brief Create an LRU cache with both a custom allocator and an eviction callback.
 *
 * @param key_size   Size in bytes of each key.
 * @param value_size Size in bytes of each value.
 * @param capacity   Maximum number of entries.
 * @param evict_fn   Callback invoked on eviction (may be NULL).
 * @param evict_ctx  Context passed to @p evict_fn.
 * @param allocator  Custom allocator for all internal memory.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_lru_t* zenit_lru_create_with_evict_and_allocator(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_lru_evict_fn_t evict_fn,
    void *evict_ctx,
    zenit_allocator_t allocator
);

/**
 * @brief Destroy an LRU cache and free all associated memory.
 *
 * If an eviction callback was registered, it is called once for each
 * entry still present in the cache before the memory is freed.
 *
 * @param cache Cache to destroy (NULL is accepted and ignored).
 */
void zenit_lru_destroy(zenit_lru_t *cache);

/**
 * @brief Insert or update an entry in the cache.
 *
 * If the key already exists, its value is updated and the entry becomes
 * the most recently used.  If the key is new and the cache is full, the
 * least recently used entry is evicted (and the eviction callback, if
 * any, is invoked).
 *
 * @param cache Cache handle.
 * @param key   Pointer to the key data (copied internally).
 * @param value Pointer to the value data (copied internally).
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_lru_put(zenit_lru_t *cache, const void *key, const void *value);

/**
 * @brief Retrieve a value from the cache by key.
 *
 * If the key is found, the entry becomes the most recently used.
 *
 * @param cache    Cache handle.
 * @param key      Pointer to the key to look up.
 * @param out_value Buffer to receive the value copy (may be NULL to check existence only).
 * @return 1 if the key was found, 0 otherwise.
 */
int zenit_lru_get(zenit_lru_t *cache, const void *key, void *out_value);

/**
 * @brief Retrieve a value without updating the LRU order.
 *
 * Like #zenit_lru_get but does NOT promote the entry to most-recently-used.
 *
 * @param cache    Cache handle (const-qualified).
 * @param key      Pointer to the key to look up.
 * @param out_value Buffer to receive the value copy (may be NULL).
 * @return 1 if the key was found, 0 otherwise.
 */
int zenit_lru_peek(const zenit_lru_t *cache, const void *key, void *out_value);

/**
 * @brief Check whether a key exists in the cache.
 *
 * Does not affect LRU order.
 *
 * @param cache Cache handle.
 * @param key   Pointer to the key to look up.
 * @return 1 if the key exists, 0 otherwise.
 */
int zenit_lru_contains(const zenit_lru_t *cache, const void *key);

/**
 * @brief Remove an entry from the cache by key.
 *
 * The eviction callback is NOT invoked for explicit removals.
 *
 * @param cache Cache handle.
 * @param key   Pointer to the key to remove.
 * @return ZENIT_RESULT_OK on success, or an error code (e.g. ZENIT_ERROR_NULL, ZENIT_ERROR_NOT_FOUND).
 */
zenit_result_t zenit_lru_remove(zenit_lru_t *cache, const void *key);

/**
 * @brief Remove all entries from the cache.
 *
 * If an eviction callback was registered, it is called once for each
 * entry before eviction.
 *
 * @param cache Cache handle (NULL is accepted and ignored).
 */
void zenit_lru_clear(zenit_lru_t *cache);

/**
 * @brief Return the number of entries currently in the cache.
 *
 * @param cache Cache handle.
 * @return Entry count (0 if cache is NULL).
 */
size_t zenit_lru_count(const zenit_lru_t *cache);

/**
 * @brief Return the maximum capacity of the cache.
 *
 * @param cache Cache handle.
 * @return Maximum number of entries (0 if cache is NULL).
 */
size_t zenit_lru_capacity(const zenit_lru_t *cache);

#endif
