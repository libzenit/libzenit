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

#ifndef LIBZENIT_BITSET_H
#define LIBZENIT_BITSET_H

#include <libzenit/result.h>
#include <libzenit/allocator.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a dynamically-sized bit set.
 *
 * Stores bits in a contiguous byte array, growing automatically on
 * demand.  Supports setting, clearing, toggling, and testing individual
 * bits, as well as bulk operations (set_all, clear_all, count).
 */
typedef struct zenit_bitset_t zenit_bitset_t;

/**
 * @brief Create a bitset with an initial capacity of @p num_bits.
 *
 * Uses the default allocator (malloc / realloc / free).
 *
 * @param num_bits Initial number of bits (the bitset may grow beyond this).
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_bitset_t* zenit_bitset_create(size_t num_bits);

/**
 * @brief Create a bitset with a custom allocator.
 *
 * All memory for the handle and the internal byte array is allocated
 * via the provided allocator.
 *
 * @param num_bits  Initial number of bits.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_bitset_t* zenit_bitset_create_with_allocator(size_t num_bits, zenit_allocator_t allocator);

/**
 * @brief Destroy a bitset and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param bs Bitset handle, or NULL.
 */
void zenit_bitset_destroy(zenit_bitset_t* bs);

/**
 * @brief Set the bit at position @p pos to 1.
 *
 * If @p pos is beyond the current capacity, the bitset is automatically
 * grown to accommodate it.
 *
 * @param bs Bitset handle (must not be NULL).
 * @param pos Bit position (0-based).
 * @return ZENIT_RESULT_OK on success, or:
 *         - ZENIT_ERROR_NULL if @p bs is NULL
 *         - ZENIT_ERROR_ALLOC if growing the internal buffer fails.
 */
zenit_result_t zenit_bitset_set(zenit_bitset_t* bs, size_t pos);

/**
 * @brief Clear the bit at position @p pos to 0.
 *
 * If @p pos is beyond the current capacity, the bitset is automatically
 * grown to accommodate it.
 *
 * @param bs Bitset handle (must not be NULL).
 * @param pos Bit position (0-based).
 * @return ZENIT_RESULT_OK on success, or:
 *         - ZENIT_ERROR_NULL if @p bs is NULL
 *         - ZENIT_ERROR_ALLOC if growing the internal buffer fails.
 */
zenit_result_t zenit_bitset_clear(zenit_bitset_t* bs, size_t pos);

/**
 * @brief Toggle the bit at position @p pos (0→1, 1→0).
 *
 * If @p pos is beyond the current capacity, the bitset is automatically
 * grown to accommodate it.
 *
 * @param bs Bitset handle (must not be NULL).
 * @param pos Bit position (0-based).
 * @return ZENIT_RESULT_OK on success, or:
 *         - ZENIT_ERROR_NULL if @p bs is NULL
 *         - ZENIT_ERROR_ALLOC if growing the internal buffer fails.
 */
zenit_result_t zenit_bitset_toggle(zenit_bitset_t* bs, size_t pos);

/**
 * @brief Test whether the bit at position @p pos is set (1).
 *
 * Returns 0 if @p pos is out of range (greater than or equal to capacity)
 * or if @p bs is NULL.
 *
 * @param bs Bitset handle.
 * @param pos Bit position (0-based).
 * @return 1 if the bit is set, 0 if it is clear, out of range, or @p bs is NULL.
 */
int zenit_bitset_test(const zenit_bitset_t* bs, size_t pos);

/**
 * @brief Set all bits in the bitset to 1.
 *
 * @param bs Bitset handle (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or ZENIT_ERROR_NULL if @p bs is NULL.
 */
zenit_result_t zenit_bitset_set_all(zenit_bitset_t* bs);

/**
 * @brief Clear all bits in the bitset to 0.
 *
 * @param bs Bitset handle (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or ZENIT_ERROR_NULL if @p bs is NULL.
 */
zenit_result_t zenit_bitset_clear_all(zenit_bitset_t* bs);

/**
 * @brief Count the number of bits that are set to 1.
 *
 * @param bs Bitset handle.
 * @return Number of 1-bits, or 0 if @p bs is NULL.
 */
size_t zenit_bitset_count(const zenit_bitset_t* bs);

/**
 * @brief Return the total capacity in bits.
 *
 * @param bs Bitset handle.
 * @return Number of bits the bitset can currently hold, or 0 if @p bs is NULL.
 */
size_t zenit_bitset_capacity(const zenit_bitset_t* bs);

/**
 * @brief Grow (or shrink) the bitset to hold at least @p num_bits.
 *
 * If the new capacity is smaller than the current capacity, the trailing
 * bits are discarded.  New bytes (if growing) are zero-initialised.
 *
 * @param bs      Bitset handle (must not be NULL).
 * @param num_bits Minimum number of bits the bitset must hold.
 * @return ZENIT_RESULT_OK on success, or:
 *         - ZENIT_ERROR_NULL if @p bs is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_bitset_resize(zenit_bitset_t* bs, size_t num_bits);

#endif
