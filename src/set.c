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
#include <stdlib.h>
#include <string.h>

/* ─── Slot states ───
 * Each slot in the hash table is one of three states:
 *   EMPTY   – never used (initial state)
 *   OCCUPIED – holds a live key
 *   DELETED  – tombstone; key was removed, probe chain continues
 */
enum { SET_SLOT_EMPTY = 0, SET_SLOT_OCCUPIED = 1, SET_SLOT_DELETED = 2 };

/* Default initial capacity (must be a power of two) */
#define SET_DEFAULT_CAPACITY 16

/* Load-factor threshold: when (occupied + deleted) > capacity * LIMIT,
 * the table is rehashed.  Stored as a numerator out of 100 to avoid floats. */
#define SET_LOAD_PERCENT 75
#define SET_GROWTH_FACTOR 2

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
};

/* ─── Helper: round up to the next power of two ─── */
static size_t round_pow2(size_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
#if SIZE_MAX > 0xFFFFFFFFULL
    n |= n >> 32;
#endif
    return n + 1;
}

/* ─── FNV-1a hash (64-bit) ───
 * Fowler–Noll–Vo hash function, variant 1a.  Operates on raw byte arrays.
 * The result is truncated to size_t on 32-bit platforms.
 */
static size_t hash_fnv1a(const void *data, size_t len) {
    size_t h = 14695981039346656037ULL;
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        h ^= (size_t)bytes[i];
        h *= 1099511628211ULL;
    }
    return h;
}

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

        if (state == SET_SLOT_EMPTY) {
            if (found_deleted) {
                *out_index = first_deleted;
                return SET_SLOT_DELETED;
            }
            *out_index = index;
            return SET_SLOT_EMPTY;
        }

        if (state == SET_SLOT_DELETED) {
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
            return SET_SLOT_OCCUPIED;
        }

        index = (index + 1) & mask;
    }
}

/* ─── Rehash: grow the table and re-insert all live entries ─── */
static zenit_result_t rehash(zenit_set_t *set, size_t new_capacity) {
    unsigned char *new_slots = calloc(new_capacity, set->key_size);
    if (new_slots == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    unsigned char *new_states = calloc(new_capacity, 1);
    if (new_states == NULL) {
        free(new_slots);
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
        if (old_states[i] == SET_SLOT_OCCUPIED) {
            const unsigned char *old_slot = old_slots + i * set->key_size;

            size_t hash = hash_fnv1a(old_slot, set->key_size);
            size_t index = hash & mask;

            while (set->states[index] == SET_SLOT_OCCUPIED) {
                index = (index + 1) & mask;
            }

            unsigned char *dest = set->slots + index * set->key_size;
            memcpy(dest, old_slot, set->key_size);
            set->states[index] = SET_SLOT_OCCUPIED;
            set->count++;
        }
    }

    free(old_slots);
    free(old_states);

    (void)old_count;

    return ZENIT_RESULT_OK;
}

/* ─── Ensure load factor is below threshold; rehash if needed ─── */
static zenit_result_t ensure_load(zenit_set_t *set) {
    size_t used = set->count + set->deleted;
    if (used * 100 > set->capacity * SET_LOAD_PERCENT) {
        size_t new_cap = set->capacity * SET_GROWTH_FACTOR;
        return rehash(set, new_cap);
    }
    return ZENIT_RESULT_OK;
}

/* ─── Public API ─── */

zenit_set_t *zenit_set_create(size_t key_size) {
    return zenit_set_create_with_capacity(key_size, SET_DEFAULT_CAPACITY);
}

zenit_set_t *zenit_set_create_with_capacity(size_t key_size, size_t capacity) {
    if (key_size == 0 || capacity == 0) {
        return NULL;
    }

    size_t cap = round_pow2(capacity);

    zenit_set_t *set = malloc(sizeof(zenit_set_t));
    if (set == NULL) {
        return NULL;
    }

    set->key_size = key_size;
    set->capacity = cap;
    set->count = 0;
    set->deleted = 0;

    set->slots = calloc(cap, key_size);
    if (set->slots == NULL) {
        free(set);
        return NULL;
    }

    set->states = calloc(cap, 1);
    if (set->states == NULL) {
        free(set->slots);
        free(set);
        return NULL;
    }

    return set;
}

void zenit_set_destroy(zenit_set_t *set) {
    if (set == NULL) {
        return;
    }
    free(set->slots);
    free(set->states);
    free(set);
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

    if (state == SET_SLOT_OCCUPIED) {
        /* Key already exists — no-op */
        return ZENIT_RESULT_OK;
    }

    unsigned char *dest = set->slots + index * set->key_size;
    memcpy(dest, key, set->key_size);
    set->states[index] = SET_SLOT_OCCUPIED;
    set->count++;

    if (state == SET_SLOT_DELETED) {
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

    if (state != SET_SLOT_OCCUPIED) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    set->states[index] = SET_SLOT_DELETED;
    set->count--;
    set->deleted++;

    return ZENIT_RESULT_OK;
}

int zenit_set_contains(const zenit_set_t *set, const void *key) {
    if (set == NULL || key == NULL) {
        return 0;
    }

    size_t index = 0;
    return probe_slot(set, key, &index) == SET_SLOT_OCCUPIED ? 1 : 0;
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
    memset(set->states, SET_SLOT_EMPTY, set->capacity);
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
        if (set->states[i] == SET_SLOT_OCCUPIED) {
            const unsigned char *slot = set->slots + i * set->key_size;
            visit(slot, ctx);
        }
    }
}
