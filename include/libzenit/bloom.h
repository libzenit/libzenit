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

#ifndef LIBZENIT_BLOOM_H
#define LIBZENIT_BLOOM_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque handle for a Bloom filter.
 *
 * A probabilistic data structure that tests set membership with a tunable
 * false positive rate.  False negatives are impossible: if contains() returns
 * 0 the element is guaranteed not to have been inserted.
 *
 * Memory is owned internally and freed on destroy.  Thread safety is
 * NOT provided — the caller must synchronise access.
 */
typedef struct zenit_bloom_t zenit_bloom_t;

/**
 * @brief Create a Bloom filter with an expected capacity and false positive rate.
 *
 * The optimal number of bits and hash functions are computed automatically
 * using the formulas:
 *   num_bits   = ceil(-capacity * ln(p) / (ln 2)^2)
 *   num_hashes = round(num_bits / capacity * ln 2)
 *
 * The bit array size is rounded up to the nearest multiple of 64.
 *
 * @param capacity            Expected number of items to insert.
 * @param false_positive_rate Desired false positive probability (0 < p < 1).
 * @return Opaque handle, or NULL on allocation failure or invalid parameters.
 */
zenit_bloom_t* zenit_bloom_create(size_t capacity, double false_positive_rate);

/**
 * @brief Create a Bloom filter with a custom allocator.
 *
 * @param capacity            Expected number of items to insert.
 * @param false_positive_rate Desired false positive probability (0 < p < 1).
 * @param allocator           Custom allocator to use for all internal memory.
 * @return Opaque handle, or NULL on allocation failure or invalid parameters.
 */
zenit_bloom_t* zenit_bloom_create_with_allocator(size_t capacity, double false_positive_rate, zenit_allocator_t allocator);

/**
 * @brief Create a Bloom filter with explicit parameters.
 *
 * @param num_bits   Number of bits in the bit array (rounded up to multiple of 64).
 * @param num_hashes Number of hash functions to use (must be > 0).
 * @return Opaque handle, or NULL on allocation failure or invalid parameters.
 */
zenit_bloom_t* zenit_bloom_create_explicit(size_t num_bits, size_t num_hashes);

/**
 * @brief Create a Bloom filter with explicit parameters and a custom allocator.
 *
 * @param num_bits   Number of bits in the bit array (rounded up to multiple of 64).
 * @param num_hashes Number of hash functions to use (must be > 0).
 * @param allocator  Custom allocator to use for all internal memory.
 * @return Opaque handle, or NULL on allocation failure or invalid parameters.
 */
zenit_bloom_t* zenit_bloom_create_explicit_with_allocator(size_t num_bits, size_t num_hashes, zenit_allocator_t allocator);

/**
 * @brief Destroy a Bloom filter and free its memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param bf Handle returned by zenit_bloom_create(), or NULL.
 */
void zenit_bloom_destroy(zenit_bloom_t *bf);

/**
 * @brief Insert an element into the Bloom filter.
 *
 * Sets the bits at the hash positions for the given data.  NULL pointers
 * are silently ignored.
 *
 * @param bf   Bloom filter handle (must not be NULL).
 * @param data Pointer to the element data (must not be NULL if len > 0).
 * @param len  Length of the element data in bytes.
 */
void zenit_bloom_insert(zenit_bloom_t *bf, const void *data, size_t len);

/**
 * @brief Test whether an element may be in the Bloom filter.
 *
 * Returns 1 if the element may be present (all hash bits are set), 0 if it
 * is definitely absent (at least one hash bit is clear).
 *
 * @param bf   Bloom filter handle (must not be NULL).
 * @param data Pointer to the element data (must not be NULL if len > 0).
 * @param len  Length of the element data in bytes.
 * @return 1 if possibly present, 0 if definitely absent, 0 if NULL params.
 */
int zenit_bloom_contains(const zenit_bloom_t *bf, const void *data, size_t len);

/**
 * @brief Clear all bits in the Bloom filter.
 *
 * After this call count() returns 0.
 *
 * @param bf Bloom filter handle (NULL is accepted and ignored).
 */
void zenit_bloom_clear(zenit_bloom_t *bf);

/**
 * @brief Return the configured false positive rate.
 *
 * For filters created with zenit_bloom_create() or
 * zenit_bloom_create_with_allocator(), this is the rate passed at
 * construction.  For filters created with the explicit variants, this
 * is an estimate computed as 0.5^num_hashes.
 *
 * @param bf Bloom filter handle.
 * @return The false positive rate, or 0.0 if @p bf is NULL.
 */
double zenit_bloom_false_positive_rate(const zenit_bloom_t *bf);

/**
 * @brief Return the approximate number of inserted items.
 *
 * This is the total number of zenit_bloom_insert() calls performed,
 * which may over-count if the same element is inserted multiple times.
 *
 * @param bf Bloom filter handle.
 * @return Insertion count, or 0 if @p bf is NULL.
 */
size_t zenit_bloom_count(const zenit_bloom_t *bf);

/**
 * @brief Return the total number of bits in the bit array.
 *
 * @param bf Bloom filter handle.
 * @return Number of bits, or 0 if @p bf is NULL.
 */
size_t zenit_bloom_num_bits(const zenit_bloom_t *bf);

/**
 * @brief Return the number of hash functions used.
 *
 * @param bf Bloom filter handle.
 * @return Number of hash functions, or 0 if @p bf is NULL.
 */
size_t zenit_bloom_num_hashes(const zenit_bloom_t *bf);

#endif
