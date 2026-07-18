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

#include <libzenit/lru.h>
#include "_hash_common.h"
#include <string.h>

/* ─── Sentinel values ─── */

/* No-node sentinel for LRU list prev/next pointers and head/tail */
#define LRU_NONE ((size_t)-1)

/* Hash-table empty slot sentinel */
#define HASH_EMPTY LRU_NONE

/* ─── Internal structure ─── */

/**
 * @brief Internal (opaque) representation of a fixed-capacity LRU cache.
 *
 * Storage layout:
 *   - keys[] and values[] are flat byte arrays indexed by node index.
 *   - prev[] / next[] form a doubly-linked LRU list (head = MRU, tail = LRU).
 *   - next[] doubles as the freelist link when a node is not in the LRU list.
 *   - hash_table[] is an open-addressing hash table mapping key hashes to
 *     node indices.  It uses linear probing.  The hash table is rebuilt
 *     from scratch on every modification (eviction or remove) to avoid
 *     tombstones, keeping the table compact.
 *   - hash_index[i] records which hash-table slot points to node i.
 */
struct zenit_lru_t {
    size_t key_size;
    size_t value_size;
    size_t capacity;
    size_t count;

    unsigned char *keys;       /* [capacity][key_size] */
    unsigned char *values;     /* [capacity][value_size] */
    size_t *prev;              /* [capacity] */
    size_t *next;              /* [capacity] — LRU link or freelist link */
    size_t *hash_index;        /* [capacity] — which hash slot owns this node */

    size_t head;               /* MRU end of the LRU list, LRU_NONE if empty */
    size_t tail;               /* LRU end, LRU_NONE if empty */
    size_t free_head;          /* head of the free-node stack, LRU_NONE if empty */

    size_t *hash_table;        /* [hash_capacity] */
    size_t hash_capacity;

    zenit_lru_evict_fn_t evict_fn;
    void *evict_ctx;
    zenit_allocator_t allocator;
};

/* ─── Free-list helpers ─── */

/* Pop a free node from the freelist.  The caller must guarantee the freelist is non-empty. */
static size_t freelist_pop(zenit_lru_t *cache) {
    size_t node = cache->free_head;
    cache->free_head = cache->next[node];
    return node;
}

/* Push a node onto the freelist.  The node must NOT be in the LRU list. */
static void freelist_push(zenit_lru_t *cache, size_t node) {
    cache->next[node] = cache->free_head;
    cache->free_head = node;
}

/* ─── LRU-list helpers ─── */

/* Remove a node from the LRU list. */
static void lru_unlink(zenit_lru_t *cache, size_t node) {
    size_t p = cache->prev[node];
    size_t n = cache->next[node];

    if (p != LRU_NONE) {
        cache->next[p] = n;
    } else {
        cache->head = n;
    }

    if (n != LRU_NONE) {
        cache->prev[n] = p;
    } else {
        cache->tail = p;
    }

    cache->prev[node] = LRU_NONE;
    cache->next[node] = LRU_NONE;
}

/* Insert a node at the MRU end (head) of the LRU list. */
static void lru_push_front(zenit_lru_t *cache, size_t node) {
    cache->prev[node] = LRU_NONE;
    cache->next[node] = cache->head;

    if (cache->head != LRU_NONE) {
        cache->prev[cache->head] = node;
    } else {
        cache->tail = node;
    }

    cache->head = node;
}

/* Move an existing node to the MRU position. */
static void lru_move_to_front(zenit_lru_t *cache, size_t node) {
    if (cache->head == node) {
        return;
    }
    lru_unlink(cache, node);
    lru_push_front(cache, node);
}

/* ─── Hash-table helpers ─── */

/*
 * Rebuild the entire hash table from the current LRU list (cache->head).
 *
 * This is O(capacity) and is called after every modification (eviction,
 * removal) to keep the table compact — no tombstones, no fragmentation.
 * Linear probing works correctly because every entry has a clean slot.
 */
static void hash_rebuild(zenit_lru_t *cache) {
    /* Reset every slot to EMPTY */
    for (size_t i = 0; i < cache->hash_capacity; i++) {
        cache->hash_table[i] = HASH_EMPTY;
    }

    /* Walk the LRU list and re-insert every live entry */
    size_t n = cache->head;
    while (n != LRU_NONE) {
        /* FNV-1a hash of the key bytes */
        size_t h = hash_fnv1a(cache->keys + n * cache->key_size, cache->key_size);
        size_t mask = cache->hash_capacity - 1;
        size_t idx = h & mask;

        /* Linear probe until we find an empty slot */
        while (cache->hash_table[idx] != HASH_EMPTY) {
            idx = (idx + 1) & mask;
        }

        cache->hash_table[idx] = n;
        cache->hash_index[n] = idx;

        n = cache->next[n];
    }
}

/*
 * Find the hash-table slot for 'key'.
 *
 * Returns the slot index and sets *found = 1 if the key exists.
 * Returns an EMPTY slot index and sets *found = 0 if the key is absent.
 * Returns SIZE_MAX and sets *found = 0 if the table is completely full.
 */
static size_t hash_find_slot(const zenit_lru_t *cache, const void *key, int *found) {
    size_t h = hash_fnv1a(key, cache->key_size);
    size_t mask = cache->hash_capacity - 1;
    size_t idx = h & mask;

    for (size_t i = 0; i < cache->hash_capacity; i++) {
        size_t slot = cache->hash_table[idx];

        if (slot == HASH_EMPTY) {
            /* End of probe chain — key not present; return this slot for insertion */
            *found = 0;
            return idx;
        }

        /* Occupied — compare the actual key bytes */
        const unsigned char *stored = cache->keys + slot * cache->key_size;
        if (memcmp(stored, key, cache->key_size) == 0) {
            *found = 1;
            return idx;
        }

        /* Continue probing */
        idx = (idx + 1) & mask;
    }

    /* Table completely full — should not happen since we rebuild on modification */
    *found = 0;
    return SIZE_MAX;
}

/* ─── Allocation helper ─── */

/*
 * Allocate a single flat array through the cache's allocator.
 * Returns NULL on failure.
 */
static void* alloc_array(zenit_allocator_t a, size_t nmemb, size_t elem_size) {
    if (nmemb == 0 || elem_size == 0) {
        return NULL;
    }
    return zenit_allocator_alloc_zero(a, nmemb, elem_size);
}

/*
 * Free all cache-owned memory.  This is the common teardown for both
 * failed construction and destroy.  The struct itself is NOT freed here.
 */
static void lru_free_internals(zenit_lru_t *cache) {
    zenit_allocator_t a = cache->allocator;
    a.free_fn(cache->keys, a.ctx);
    a.free_fn(cache->values, a.ctx);
    a.free_fn(cache->prev, a.ctx);
    a.free_fn(cache->next, a.ctx);
    a.free_fn(cache->hash_index, a.ctx);
    a.free_fn(cache->hash_table, a.ctx);
}

/* ─── Constructor core ─── */

static zenit_lru_t* lru_create_impl(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_lru_evict_fn_t evict_fn,
    void *evict_ctx,
    zenit_allocator_t allocator
) {
    /* Validate parameter ranges */
    if (key_size == 0 || value_size == 0 || capacity == 0) {
        return NULL;
    }

    /* Allocate the handle itself via the user's allocator */
    zenit_lru_t *cache = allocator.alloc_fn(sizeof(zenit_lru_t), allocator.ctx);
    if (cache == NULL) {
        return NULL;
    }

    /* Zero the struct so every field starts in a safe state */
    memset(cache, 0, sizeof(zenit_lru_t));

    cache->key_size = key_size;
    cache->value_size = value_size;
    cache->capacity = capacity;
    cache->evict_fn = evict_fn;
    cache->evict_ctx = evict_ctx;
    cache->allocator = allocator;

    cache->head = LRU_NONE;
    cache->tail = LRU_NONE;
    cache->free_head = 0;

    /* Compute hash-table capacity: next power of two >= 2 * capacity.
     * This gives at most 50% load factor, well within linear-probe range. */
    cache->hash_capacity = round_pow2(capacity * 2);

    /* Allocate all internal arrays */
    cache->keys = alloc_array(allocator, capacity, key_size);
    cache->values = alloc_array(allocator, capacity, value_size);
    cache->prev = alloc_array(allocator, capacity, sizeof(size_t));
    cache->next = alloc_array(allocator, capacity, sizeof(size_t));
    cache->hash_index = alloc_array(allocator, capacity, sizeof(size_t));
    cache->hash_table = alloc_array(allocator, cache->hash_capacity, sizeof(size_t));

    /* Check all allocations succeeded */
    if (cache->keys == NULL || cache->values == NULL ||
        cache->prev == NULL || cache->next == NULL ||
        cache->hash_index == NULL || cache->hash_table == NULL) {
        lru_free_internals(cache);
        allocator.free_fn(cache, allocator.ctx);
        return NULL;
    }

    /* Initialise hash-table slots to EMPTY */
    for (size_t i = 0; i < cache->hash_capacity; i++) {
        cache->hash_table[i] = HASH_EMPTY;
    }

    /* Initialise the free list: all node indices linked sequentially */
    for (size_t i = 0; i < capacity; i++) {
        cache->next[i] = i + 1;
    }
    cache->next[capacity - 1] = LRU_NONE;

    return cache;
}

/* ─── Public API ─── */

zenit_lru_t* zenit_lru_create(size_t key_size, size_t value_size, size_t capacity) {
    return lru_create_impl(key_size, value_size, capacity, NULL, NULL, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_lru_t* zenit_lru_create_with_allocator(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_allocator_t allocator
) {
    return lru_create_impl(key_size, value_size, capacity, NULL, NULL, allocator);
}

zenit_lru_t* zenit_lru_create_with_evict(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_lru_evict_fn_t evict_fn,
    void *evict_ctx
) {
    return lru_create_impl(key_size, value_size, capacity, evict_fn, evict_ctx, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_lru_t* zenit_lru_create_with_evict_and_allocator(
    size_t key_size,
    size_t value_size,
    size_t capacity,
    zenit_lru_evict_fn_t evict_fn,
    void *evict_ctx,
    zenit_allocator_t allocator
) {
    return lru_create_impl(key_size, value_size, capacity, evict_fn, evict_ctx, allocator);
}

void zenit_lru_destroy(zenit_lru_t *cache) {
    if (cache == NULL) {
        return;
    }

    /* If an eviction callback is registered, notify for every live entry */
    if (cache->evict_fn != NULL) {
        size_t node = cache->head;
        while (node != LRU_NONE) {
            cache->evict_fn(
                cache->keys + node * cache->key_size,
                cache->values + node * cache->value_size,
                cache->evict_ctx
            );
            node = cache->next[node];
        }
    }

    zenit_allocator_t a = cache->allocator;
    lru_free_internals(cache);
    a.free_fn(cache, a.ctx);
}

zenit_result_t zenit_lru_put(zenit_lru_t *cache, const void *key, const void *value) {
    if (cache == NULL || key == NULL || value == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    int found = 0;
    size_t slot = hash_find_slot(cache, key, &found);

    if (found) {
        /* Key exists — update the value and promote to MRU */
        size_t node = cache->hash_table[slot];
        memcpy(cache->values + node * cache->value_size, value, cache->value_size);
        lru_move_to_front(cache, node);
        return ZENIT_RESULT_OK;
    }

    size_t node;
    int need_rebuild = 0;

    if (cache->count == cache->capacity) {
        /* Cache is full — evict the LRU entry (tail) and reuse its node slot */
        node = cache->tail;

        /* Notify the caller via the eviction callback */
        if (cache->evict_fn != NULL) {
            cache->evict_fn(
                cache->keys + node * cache->key_size,
                cache->values + node * cache->value_size,
                cache->evict_ctx
            );
        }

        /* Unlink from the LRU list */
        lru_unlink(cache, node);

        /* Mark that we need to rebuild the hash table */
        need_rebuild = 1;
    } else {
        /* Pop a free node from the freelist */
        node = freelist_pop(cache);
        /* A new entry is being added */
        cache->count++;
    }

    /* Copy the key and value into the node */
    memcpy(cache->keys + node * cache->key_size, key, cache->key_size);
    memcpy(cache->values + node * cache->value_size, value, cache->value_size);

    /* Link at the MRU end of the LRU list */
    lru_push_front(cache, node);

    /* Rebuild the hash table from the current LRU list.
     * This is O(capacity) but avoids tombstone complexity. */
    if (need_rebuild) {
        hash_rebuild(cache);
    } else {
        /* No eviction: just insert the new node into the hash table */
        size_t h = hash_fnv1a(key, cache->key_size);
        size_t mask = cache->hash_capacity - 1;
        size_t idx = h & mask;

        /* Linear probe */
        while (cache->hash_table[idx] != HASH_EMPTY) {
            idx = (idx + 1) & mask;
        }

        cache->hash_table[idx] = node;
        cache->hash_index[node] = idx;
    }

    return ZENIT_RESULT_OK;
}

int zenit_lru_get(zenit_lru_t *cache, const void *key, void *out_value) {
    /* NULL cache or key is a miss */
    if (cache == NULL || key == NULL) {
        return 0;
    }

    int found = 0;
    size_t slot = hash_find_slot(cache, key, &found);

    if (!found) {
        return 0;
    }

    size_t node = cache->hash_table[slot];

    /* Copy the value out if the caller provided a buffer */
    if (out_value != NULL) {
        memcpy(out_value, cache->values + node * cache->value_size, cache->value_size);
    }

    /* Promote to MRU */
    lru_move_to_front(cache, node);

    return 1;
}

int zenit_lru_peek(const zenit_lru_t *cache, const void *key, void *out_value) {
    /* NULL cache or key is a miss */
    if (cache == NULL || key == NULL) {
        return 0;
    }

    int found = 0;
    size_t slot = hash_find_slot(cache, key, &found);

    if (!found) {
        return 0;
    }

    size_t node = cache->hash_table[slot];

    if (out_value != NULL) {
        memcpy(out_value, cache->values + node * cache->value_size, cache->value_size);
    }

    return 1;
}

int zenit_lru_contains(const zenit_lru_t *cache, const void *key) {
    if (cache == NULL || key == NULL) {
        return 0;
    }

    int found = 0;
    hash_find_slot(cache, key, &found);
    return found;
}

zenit_result_t zenit_lru_remove(zenit_lru_t *cache, const void *key) {
    if (cache == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    int found = 0;
    hash_find_slot(cache, key, &found);

    if (!found) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    /* Re-find the slot to get the node index (hash_find_slot returned the slot) */
    size_t slot = hash_find_slot(cache, key, &found);
    size_t node = cache->hash_table[slot];

    /* Unlink from the LRU list */
    lru_unlink(cache, node);

    /* Return the node to the freelist */
    freelist_push(cache, node);

    cache->count--;

    /* Rebuild the hash table without the removed entry */
    hash_rebuild(cache);

    return ZENIT_RESULT_OK;
}

void zenit_lru_clear(zenit_lru_t *cache) {
    if (cache == NULL) {
        return;
    }

    /* Invoke the eviction callback for every live entry, if registered */
    if (cache->evict_fn != NULL) {
        size_t node = cache->head;
        while (node != LRU_NONE) {
            cache->evict_fn(
                cache->keys + node * cache->key_size,
                cache->values + node * cache->value_size,
                cache->evict_ctx
            );
            node = cache->next[node];
        }
    }

    /* Reset all hash-table slots to EMPTY */
    for (size_t i = 0; i < cache->hash_capacity; i++) {
        cache->hash_table[i] = HASH_EMPTY;
    }

    /* Reset the LRU list to empty */
    cache->head = LRU_NONE;
    cache->tail = LRU_NONE;

    /* Rebuild the freelist: all node indices are free */
    cache->free_head = 0;
    for (size_t i = 0; i < cache->capacity; i++) {
        cache->next[i] = i + 1;
    }
    cache->next[cache->capacity - 1] = LRU_NONE;

    cache->count = 0;
}

size_t zenit_lru_count(const zenit_lru_t *cache) {
    if (cache == NULL) {
        return 0;
    }
    return cache->count;
}

size_t zenit_lru_capacity(const zenit_lru_t *cache) {
    if (cache == NULL) {
        return 0;
    }
    return cache->capacity;
}
