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

#ifndef LIBZENIT_HASH_COMMON_H
#define LIBZENIT_HASH_COMMON_H

#include <stddef.h>
#include <stdint.h>

/*
 * Shared helpers for open-addressing hash tables (map and set).
 *
 * Both zenit_map_t and zenit_set_t use:
 *   - FNV-1a hashing
 *   - Three-state slots (EMPTY / OCCUPIED / DELETED)
 *   - Power-of-2 capacity with 75 % load-factor threshold and 2x growth
 */

/* ─── Slot states ─── */
enum { HASH_SLOT_EMPTY = 0, HASH_SLOT_OCCUPIED = 1, HASH_SLOT_DELETED = 2 };

/* Default initial capacity (must be a power of two) */
#define HASH_DEFAULT_CAPACITY 16

/* Load-factor threshold: rehash when (occupied + deleted) > capacity * 75 / 100 */
#define HASH_LOAD_PERCENT 75
#define HASH_GROWTH_FACTOR 2

/* ─── Round up to the next power of two ─── */
static inline size_t round_pow2(size_t n) {
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
 * Fowler-Noll-Vo hash function, variant 1a.  Operates on raw byte arrays.
 * The result is truncated to size_t on 32-bit platforms.
 */
static inline size_t hash_fnv1a(const void *data, size_t len) {
    size_t h = 14695981039346656037ULL;
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        h ^= (size_t)bytes[i];
        h *= 1099511628211ULL;
    }
    return h;
}

#endif
