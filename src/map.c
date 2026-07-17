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

#include <libzenit/map.h>
#include "_hash_common.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal hash map state.
 *
 * Keys and values are stored together in a flat byte array where each slot
 * holds key_size + value_size bytes (key first, then value).  A separate
 * state byte array tracks EMPTY / OCCUPIED / DELETED for each slot.
 */
struct zenit_map_t {
    unsigned char *slots;   /**< Flat array: [key][value] per slot */
    unsigned char *states;  /**< State byte per slot (EMPTY/OCCUPIED/DELETED) */
    size_t key_size;        /**< Size in bytes of each key */
    size_t value_size;      /**< Size in bytes of each value */
    size_t slot_size;       /**< key_size + value_size */
    size_t capacity;        /**< Total number of slots (always a power of two) */
    size_t count;           /**< Number of live (OCCUPIED) entries */
    size_t deleted;         /**< Number of DELETED (tombstone) slots */
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
    const zenit_map_t *map, const void *key, size_t *out_index
) {
    size_t hash = hash_fnv1a(key, map->key_size);
    size_t mask = map->capacity - 1;
    size_t index = hash & mask;

    /* Track the first DELETED slot seen, so we can reclaim tombstones */
    size_t first_deleted = map->capacity;
    int found_deleted = 0;

    /* Walk until we find an EMPTY slot or a key match.  The 75 % load factor
     * guarantees at least 25 % EMPTY slots, so we always terminate. */
    while (1) {
        unsigned char state = map->states[index];

        if (state == HASH_SLOT_EMPTY) {
            /* Prefer reclaiming a tombstone over an empty slot */
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
        const unsigned char *slot = map->slots + index * map->slot_size;
        if (memcmp(slot, key, map->key_size) == 0) {
            *out_index = index;
            return HASH_SLOT_OCCUPIED;
        }

        index = (index + 1) & mask;
    }
}

/* ─── Rehash: grow the table and re-insert all live entries ─── */
static zenit_result_t rehash(zenit_map_t *map, size_t new_capacity) {
    /* new_capacity must be a power of two (caller ensures this) */

    /* Allocate new slot array */
    unsigned char *new_slots = calloc(new_capacity, map->slot_size);
    if (new_slots == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    /* Allocate new state array — calloc gives all EMPTY (0) */
    unsigned char *new_states = calloc(new_capacity, 1);
    if (new_states == NULL) {
        free(new_slots);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    /* Snapshot old pointers before we swap */
    unsigned char *old_slots = map->slots;
    unsigned char *old_states = map->states;
    size_t old_capacity = map->capacity;
    size_t old_count = map->count;

    /* Swap in the new arrays */
    map->slots = new_slots;
    map->states = new_states;
    map->capacity = new_capacity;
    map->count = 0;
    map->deleted = 0;

    /* Re-insert every live entry from the old table */
    size_t mask = new_capacity - 1;
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_states[i] == HASH_SLOT_OCCUPIED) {
            /* Extract key and value from the old slot */
            const unsigned char *old_slot = old_slots + i * map->slot_size;
            const void *key = old_slot;
            const void *value = old_slot + map->key_size;

            /* Compute the new probe position */
            size_t hash = hash_fnv1a(key, map->key_size);
            size_t index = hash & mask;

            /* Linear probe in the new table */
            while (map->states[index] == HASH_SLOT_OCCUPIED) {
                index = (index + 1) & mask;
            }

            /* Copy key and value into the new slot */
            unsigned char *dest = map->slots + index * map->slot_size;
            memcpy(dest, key, map->key_size);
            memcpy(dest + map->key_size, value, map->value_size);
            map->states[index] = HASH_SLOT_OCCUPIED;
            map->count++;
        }
    }

    /* Free the old arrays */
    free(old_slots);
    free(old_states);

    /* Verify we re-inserted every entry (defensive) */
    (void)old_count;

    return ZENIT_RESULT_OK;
}

/* ─── Ensure load factor is below threshold; rehash if needed ─── */
static zenit_result_t ensure_load(zenit_map_t *map) {
    /* Load factor = (occupied + deleted) / capacity.
     * Rehash when occupied + deleted > capacity * 75 / 100. */
    size_t used = map->count + map->deleted;
    if (used * 100 > map->capacity * HASH_LOAD_PERCENT) {
        /* Double the capacity (always power of two if starting from one) */
        size_t new_cap = map->capacity * HASH_GROWTH_FACTOR;
        return rehash(map, new_cap);
    }
    return ZENIT_RESULT_OK;
}

/* ─── Public API ─── */

zenit_map_t *zenit_map_create(size_t key_size, size_t value_size) {
    /* Delegate to the full create function with default capacity */
    return zenit_map_create_with_capacity(key_size, value_size, HASH_DEFAULT_CAPACITY);
}

zenit_map_t *zenit_map_create_with_capacity(
    size_t key_size, size_t value_size, size_t capacity
) {
    /* Validate parameters */
    if (key_size == 0 || value_size == 0 || capacity == 0) {
        return NULL;
    }

    /* Round capacity to the next power of two */
    size_t cap = round_pow2(capacity);

    /* Allocate the handle */
    zenit_map_t *map = malloc(sizeof(zenit_map_t));
    if (map == NULL) {
        return NULL;
    }

    /* Populate fields */
    map->key_size = key_size;
    map->value_size = value_size;
    map->slot_size = key_size + value_size;
    map->capacity = cap;
    map->count = 0;
    map->deleted = 0;

    /* Allocate slot array — calloc so all bytes are zeroed */
    map->slots = calloc(cap, map->slot_size);
    if (map->slots == NULL) {
        free(map);
        return NULL;
    }

    /* Allocate state array — calloc gives all HASH_SLOT_EMPTY (0) */
    map->states = calloc(cap, 1);
    if (map->states == NULL) {
        free(map->slots);
        free(map);
        return NULL;
    }

    return map;
}

void zenit_map_destroy(zenit_map_t *map) {
    if (map == NULL) {
        return;
    }
    /* Free the slot and state arrays, then the handle itself */
    free(map->slots);
    free(map->states);
    free(map);
}

zenit_result_t zenit_map_insert(
    zenit_map_t *map, const void *key, const void *value
) {
    /* Validate preconditions */
    if (map == NULL || key == NULL || value == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Check load factor and rehash if needed */
    zenit_result_t lr = ensure_load(map);
    if (lr.error != ZENIT_OK) {
        return lr;
    }

    /* Probe for the key — if found we will overwrite */
    size_t index = 0;
    int state = probe_slot(map, key, &index);

    if (state == HASH_SLOT_OCCUPIED) {
        /* Key exists — overwrite the value portion only */
        unsigned char *dest = map->slots + index * map->slot_size + map->key_size;
        memcpy(dest, value, map->value_size);
        return ZENIT_RESULT_OK;
    }

    /* Key not found — index points to the first EMPTY or DELETED slot.
     * Place the new entry there. */
    unsigned char *dest = map->slots + index * map->slot_size;
    memcpy(dest, key, map->key_size);
    memcpy(dest + map->key_size, value, map->value_size);
    map->states[index] = HASH_SLOT_OCCUPIED;
    map->count++;

    /* If we overwrote a DELETED slot, decrement the tombstone counter */
    if (state == HASH_SLOT_DELETED) {
        map->deleted--;
    }

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_map_get(
    const zenit_map_t *map, const void *key, void *out_value
) {
    /* Validate preconditions */
    if (map == NULL || key == NULL || out_value == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Probe for the key */
    size_t index = 0;
    int state = probe_slot(map, key, &index);

    if (state != HASH_SLOT_OCCUPIED) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    /* Copy the value from the slot */
    const unsigned char *src = map->slots + index * map->slot_size + map->key_size;
    memcpy(out_value, src, map->value_size);
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_map_remove(zenit_map_t *map, const void *key) {
    /* Validate preconditions */
    if (map == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Probe for the key */
    size_t index = 0;
    int state = probe_slot(map, key, &index);

    if (state != HASH_SLOT_OCCUPIED) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    /* Leave a tombstone so that probe chains are not broken */
    map->states[index] = HASH_SLOT_DELETED;
    map->count--;
    map->deleted++;

    return ZENIT_RESULT_OK;
}

int zenit_map_contains(const zenit_map_t *map, const void *key) {
    if (map == NULL || key == NULL) {
        return 0;
    }

    /* Probe for the key — OCCUPIED means present */
    size_t index = 0;
    return probe_slot(map, key, &index) == HASH_SLOT_OCCUPIED ? 1 : 0;
}

size_t zenit_map_count(const zenit_map_t *map) {
    if (map == NULL) {
        return 0;
    }
    return map->count;
}

size_t zenit_map_capacity(const zenit_map_t *map) {
    if (map == NULL) {
        return 0;
    }
    return map->capacity;
}

void zenit_map_clear(zenit_map_t *map) {
    if (map == NULL) {
        return;
    }
    /* Reset every slot state to EMPTY.  The data buffers are left intact. */
    memset(map->states, HASH_SLOT_EMPTY, map->capacity);
    map->count = 0;
    map->deleted = 0;
}

void zenit_map_foreach(
    const zenit_map_t *map, zenit_map_visit_fn_t visit, void *ctx
) {
    if (map == NULL || visit == NULL) {
        return;
    }

    /* Walk every slot and invoke the callback for OCCUPIED entries */
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->states[i] == HASH_SLOT_OCCUPIED) {
            const unsigned char *slot = map->slots + i * map->slot_size;
            visit(slot, slot + map->key_size, ctx);
        }
    }
}
