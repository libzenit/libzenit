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

#include <libzenit/set.h>
#include <libzenit/allocator.h>
#include <string.h>
#include "_hash_common.h"

/**
 * @brief Internal hash set state.
 *
 * Keys are stored in a flat byte array where each slot holds key_size bytes.
 * A separate state byte array tracks EMPTY / OCCUPIED / DELETED for each slot.
 */
struct zenit_set_t {
    unsigned char *slots;   /**< Flat array of keys: key_size bytes per slot */
    unsigned char *states;  /**< State byte per slot (EMPTY/OCCUPIED/DELETED) */
    size_t key_size;        /**< Size in bytes of each key */
    size_t capacity;        /**< Total number of slots (always a power of two) */
    size_t count;           /**< Number of live (OCCUPIED) entries */
    size_t deleted;         /**< Number of DELETED (tombstone) slots */
    zenit_allocator_t allocator; /**< Allocator used for all memory operations */
};

/* ─── Probe: walk the slot array looking for a key ───
 * Returns the slot state (OCCUPIED, DELETED, or EMPTY) and writes the slot
 * index into *out_index.
 *
 * - If an OCCUPIED slot with a matching key is found, returns OCCUPIED.
 * - If the key is not found, returns the first available slot for insertion
 *   (preferring an earlier DELETED tombstone over a later EMPTY slot so that
 *   tombstones are reclaimed and deleted_count decreases).
 * - Returns EMPTY only when no DELETED slot was encountered along the chain.
 */
static int probe_slot(
    const zenit_set_t *set, const void *key, size_t *out_index
) {
    size_t hash = hash_fnv1a(key, set->key_size);
    size_t mask = set->capacity - 1;
    size_t index = hash & mask;

    size_t first_deleted = set->capacity;
    int found_deleted = 0;

    while (1) {
        unsigned char state = set->states[index];

        if (state == HASH_SLOT_EMPTY) {
            if (found_deleted) {
                *out_index = first_deleted;
                return HASH_SLOT_DELETED;
            }
            *out_index = index;
            return HASH_SLOT_EMPTY;
        }

        if (state == HASH_SLOT_DELETED) {
            if (!found_deleted) {
                first_deleted = index;
                found_deleted = 1;
            }
            index = (index + 1) & mask;
            continue;
        }

        /* State == OCCUPIED */
        const unsigned char *slot = set->slots + index * set->key_size;
        if (memcmp(slot, key, set->key_size) == 0) {
            *out_index = index;
            return HASH_SLOT_OCCUPIED;
        }

        index = (index + 1) & mask;
    }
}

/* ─── Rehash: grow the table and re-insert all live entries ─── */
static zenit_result_t rehash(zenit_set_t *set, size_t new_capacity) {
    unsigned char *new_slots = zenit_allocator_alloc_zero(set->allocator, new_capacity, set->key_size);
    if (new_slots == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    unsigned char *new_states = zenit_allocator_alloc_zero(set->allocator, new_capacity, 1);
    if (new_states == NULL) {
        set->allocator.free_fn(new_slots, set->allocator.ctx);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    unsigned char *old_slots = set->slots;
    unsigned char *old_states = set->states;
    size_t old_capacity = set->capacity;
    size_t old_count = set->count;

    set->slots = new_slots;
    set->states = new_states;
    set->capacity = new_capacity;
    set->count = 0;
    set->deleted = 0;

    size_t mask = new_capacity - 1;
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_states[i] == HASH_SLOT_OCCUPIED) {
            const unsigned char *old_slot = old_slots + i * set->key_size;

            size_t hash = hash_fnv1a(old_slot, set->key_size);
            size_t index = hash & mask;

            while (set->states[index] == HASH_SLOT_OCCUPIED) {
                index = (index + 1) & mask;
            }

            unsigned char *dest = set->slots + index * set->key_size;
            memcpy(dest, old_slot, set->key_size);
            set->states[index] = HASH_SLOT_OCCUPIED;
            set->count++;
        }
    }

    set->allocator.free_fn(old_slots, set->allocator.ctx);
    set->allocator.free_fn(old_states, set->allocator.ctx);

    (void)old_count;

    return ZENIT_RESULT_OK;
}

/* ─── Ensure load factor is below threshold; rehash if needed ─── */
static zenit_result_t ensure_load(zenit_set_t *set) {
    size_t used = set->count + set->deleted;
    if (used * 100 > set->capacity * HASH_LOAD_PERCENT) {
        size_t new_cap = set->capacity * HASH_GROWTH_FACTOR;
        return rehash(set, new_cap);
    }
    return ZENIT_RESULT_OK;
}

/* ─── Public API ─── */

zenit_set_t *zenit_set_create(size_t key_size) {
    return zenit_set_create_with_capacity(key_size, HASH_DEFAULT_CAPACITY);
}

zenit_set_t *zenit_set_create_with_capacity(size_t key_size, size_t capacity) {
    return zenit_set_create_with_capacity_and_allocator(
        key_size, capacity, ZENIT_ALLOCATOR_DEFAULT
    );
}

zenit_set_t *zenit_set_create_with_allocator(size_t key_size, zenit_allocator_t allocator) {
    return zenit_set_create_with_capacity_and_allocator(
        key_size, HASH_DEFAULT_CAPACITY, allocator
    );
}

zenit_set_t *zenit_set_create_with_capacity_and_allocator(
    size_t key_size, size_t capacity, zenit_allocator_t allocator
) {
    if (key_size == 0 || capacity == 0) {
        return NULL;
    }

    size_t cap = round_pow2(capacity);

    zenit_set_t *set = allocator.alloc_fn(sizeof(zenit_set_t), allocator.ctx);
    if (set == NULL) {
        return NULL;
    }

    set->key_size = key_size;
    set->capacity = cap;
    set->count = 0;
    set->deleted = 0;
    set->allocator = allocator;

    set->slots = zenit_allocator_alloc_zero(allocator, cap, key_size);
    if (set->slots == NULL) {
        allocator.free_fn(set, allocator.ctx);
        return NULL;
    }

    set->states = zenit_allocator_alloc_zero(allocator, cap, 1);
    if (set->states == NULL) {
        allocator.free_fn(set->slots, allocator.ctx);
        allocator.free_fn(set, allocator.ctx);
        return NULL;
    }

    return set;
}

void zenit_set_destroy(zenit_set_t *set) {
    if (set == NULL) {
        return;
    }
    set->allocator.free_fn(set->slots, set->allocator.ctx);
    set->allocator.free_fn(set->states, set->allocator.ctx);
    set->allocator.free_fn(set, set->allocator.ctx);
}

zenit_result_t zenit_set_insert(zenit_set_t *set, const void *key) {
    if (set == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    zenit_result_t lr = ensure_load(set);
    if (lr.error != ZENIT_OK) {
        return lr;
    }

    size_t index = 0;
    int state = probe_slot(set, key, &index);

    if (state == HASH_SLOT_OCCUPIED) {
        /* Key already exists — no-op */
        return ZENIT_RESULT_OK;
    }

    unsigned char *dest = set->slots + index * set->key_size;
    memcpy(dest, key, set->key_size);
    set->states[index] = HASH_SLOT_OCCUPIED;
    set->count++;

    if (state == HASH_SLOT_DELETED) {
        set->deleted--;
    }

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_set_remove(zenit_set_t *set, const void *key) {
    if (set == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    size_t index = 0;
    int state = probe_slot(set, key, &index);

    if (state != HASH_SLOT_OCCUPIED) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    set->states[index] = HASH_SLOT_DELETED;
    set->count--;
    set->deleted++;

    return ZENIT_RESULT_OK;
}

int zenit_set_contains(const zenit_set_t *set, const void *key) {
    if (set == NULL || key == NULL) {
        return 0;
    }

    size_t index = 0;
    return probe_slot(set, key, &index) == HASH_SLOT_OCCUPIED ? 1 : 0;
}

size_t zenit_set_count(const zenit_set_t *set) {
    if (set == NULL) {
        return 0;
    }
    return set->count;
}

size_t zenit_set_capacity(const zenit_set_t *set) {
    if (set == NULL) {
        return 0;
    }
    return set->capacity;
}

void zenit_set_clear(zenit_set_t *set) {
    if (set == NULL) {
        return;
    }
    memset(set->states, HASH_SLOT_EMPTY, set->capacity);
    set->count = 0;
    set->deleted = 0;
}

void zenit_set_foreach(
    const zenit_set_t *set, zenit_set_visit_fn_t visit, void *ctx
) {
    if (set == NULL || visit == NULL) {
        return;
    }

    for (size_t i = 0; i < set->capacity; i++) {
        if (set->states[i] == HASH_SLOT_OCCUPIED) {
            const unsigned char *slot = set->slots + i * set->key_size;
            visit(slot, ctx);
        }
    }
}

zenit_iter_t zenit_set_iter(zenit_set_t *set) {
    zenit_iter_t iter;
    iter.container = set;
    iter.index = 0;
    iter.count = set ? set->count : 0;
    iter.is_valid = (set != NULL) ? 1 : 0;
    iter.internal = NULL;
    return iter;
}

void *zenit_set_iter_next(zenit_iter_t *iter) {
    if (iter == NULL || !iter->is_valid) {
        return NULL;
    }
    const zenit_set_t *set = (const zenit_set_t*)iter->container;
    while (iter->index < set->capacity) {
        size_t idx = iter->index;
        iter->index++;
        if (set->states[idx] == HASH_SLOT_OCCUPIED) {
            return set->slots + idx * set->key_size;
        }
    }
    return NULL;
}

zenit_result_t zenit_set_to_array(const zenit_set_t *set, void **out_keys, size_t *out_count) {
    if (set == NULL || out_keys == NULL || out_count == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    *out_keys = NULL;
    *out_count = 0;
    if (set->count == 0) {
        return ZENIT_RESULT_OK;
    }
    unsigned char *keys = set->allocator.alloc_fn(set->count * set->key_size, set->allocator.ctx);
    if (keys == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    size_t pos = 0;
    for (size_t i = 0; i < set->capacity; i++) {
        if (set->states[i] == HASH_SLOT_OCCUPIED) {
            const unsigned char *slot = set->slots + i * set->key_size;
            memcpy(keys + pos * set->key_size, slot, set->key_size);
            pos++;
        }
    }
    *out_keys = keys;
    *out_count = pos;
    return ZENIT_RESULT_OK;
}
