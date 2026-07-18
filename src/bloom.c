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

#include <libzenit/bloom.h>
#include <libzenit/allocator.h>
#include "_hash_common.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

/* ─── Math constants (precomputed) ─── */

/* ln(2) ≈ 0.6931471805599453 */
#define LN2 0.6931471805599453

/* (ln 2)^2 ≈ 0.4804530139182014 */
#define LN2_SQ 0.4804530139182014

/* ─── Internal structure ─── */

/**
 * @brief Bloom filter internal state.
 *
 * The bit array is stored as an array of uint64_t for efficient bit-level
 * access.  @p num_bits is always a multiple of 64.  @p count tracks the
 * total number of insertion operations performed.
 */
struct zenit_bloom_t {
    uint64_t *bits;              /**< Bit array (aligned to 64-bit words) */
    size_t num_bits;             /**< Total number of bits (multiple of 64) */
    size_t num_hashes;           /**< Number of hash functions */
    size_t count;                /**< Approximate number of inserted items */
    double fpr;                  /**< Configured false positive rate */
    zenit_allocator_t allocator; /**< Custom allocator for this instance */
};

/* ─── Alternative FNV-1a hash for double hashing ─── */

/**
 * @brief Second independent hash function.
 *
 * Uses FNV-1a with a different initial offset (XOR of the standard basis
 * with the golden ratio) to produce a hash value independent of
 * hash_fnv1a().
 *
 * @param data Pointer to the input data.
 * @param len  Length of the input data in bytes.
 * @return 64-bit (or size_t) hash value.
 */
static size_t hash_fnv1a_alt(const void *data, size_t len) {
    /* Use golden ratio constant XORed with standard FNV offset as seed */
    size_t h = 14695981039346656037ULL ^ 0x9e3779b97f4a7c15ULL;
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        h ^= (size_t)bytes[i];
        h *= 1099511628211ULL;
    }
    return h;
}

/* ─── Bit-level helpers ─── */

/* Set the bit at position idx (0-indexed) in the uint64_t array */
static inline void bit_set(uint64_t *bits, size_t idx) {
    bits[idx / 64] |= (1ULL << (idx % 64));
}

/* Test the bit at position idx (0-indexed); returns non-zero if set */
static inline int bit_test(const uint64_t *bits, size_t idx) {
    return (bits[idx / 64] >> (idx % 64)) & 1ULL;
}

/* ─── Common creation helper ─── */

/**
 * @brief Shared constructor that allocates the handle and bit array.
 *
 * Validates that num_bits >= 64 and num_hashes >= 1, rounds num_bits up
 * to the nearest multiple of 64, allocates the struct and bit array via
 * the custom allocator, and initialises all fields.
 *
 * @param num_bits   Desired number of bits (rounded up to 64-multiple).
 * @param num_hashes Number of hash functions (must be >= 1).
 * @param fpr        False positive rate to store.
 * @param allocator  Custom allocator for all internal allocations.
 * @return Opaque handle, or NULL on invalid params or allocation failure.
 */
static zenit_bloom_t *bloom_create_common(size_t num_bits, size_t num_hashes, double fpr, zenit_allocator_t allocator) {
    /* Validate parameters — both must be positive */
    if (num_bits == 0 || num_hashes == 0) {
        return NULL;
    }

    /* Round num_bits up to the nearest multiple of 64 */
    if (num_bits % 64 != 0) {
        num_bits = ((num_bits / 64) + 1) * 64;
    }

    /* Allocate the handle via the custom allocator */
    zenit_bloom_t *bf = allocator.alloc_fn(sizeof(zenit_bloom_t), allocator.ctx);
    if (bf == NULL) {
        return NULL;
    }

    /* Store the allocator early so destroy works if bits allocation fails */
    bf->allocator = allocator;
    bf->num_bits = num_bits;
    bf->num_hashes = num_hashes;
    bf->count = 0;
    bf->fpr = fpr;

    /* Allocate the bit array — zeroed so all bits start cleared */
    size_t arr_size = num_bits / 64;
    bf->bits = zenit_allocator_alloc_zero(allocator, arr_size, sizeof(uint64_t));
    if (bf->bits == NULL) {
        /* Bit array allocation failed — free the handle and propagate NULL */
        allocator.free_fn(bf, allocator.ctx);
        return NULL;
    }

    return bf;
}

/* ─── Public API ─── */

/**
 * @brief Create with default allocator.
 *
 * Delegates to the with-allocator variant.
 */
zenit_bloom_t *zenit_bloom_create(size_t capacity, double false_positive_rate) {
    return zenit_bloom_create_with_allocator(capacity, false_positive_rate, ZENIT_ALLOCATOR_DEFAULT);
}

/**
 * @brief Create with custom allocator and optimal parameter computation.
 *
 * Computes the optimal number of bits and hash functions using the
 * standard Bloom filter formulas:
 *   num_bits   = ceil(-capacity * ln(p) / (ln 2)^2)
 *   num_hashes = round(num_bits / capacity * ln 2)
 */
zenit_bloom_t *zenit_bloom_create_with_allocator(size_t capacity, double false_positive_rate, zenit_allocator_t allocator) {
    /* Validate: capacity must be positive and rate must be in (0, 1) */
    if (capacity == 0 || false_positive_rate <= 0.0 || false_positive_rate >= 1.0) {
        return NULL;
    }

    /* Compute the optimal number of bits using m = -n * ln(p) / (ln 2)^2 */
    double ln_p = log(false_positive_rate);
    double m_d = -(double)capacity * ln_p / LN2_SQ;
    size_t num_bits = (size_t)(m_d + 0.5);
    /* Ensure at least 64 bits (one uint64_t word) */
    if (num_bits < 64) {
        num_bits = 64;
    }

    /* Compute the optimal number of hash functions using k = m/n * ln(2) */
    double k_d = (double)num_bits / (double)capacity * LN2;
    size_t num_hashes = (size_t)(k_d + 0.5);
    if (num_hashes < 1) {
        num_hashes = 1;
    }

    /* Delegate to the common creator, passing the user's desired FPR */
    return bloom_create_common(num_bits, num_hashes, false_positive_rate, allocator);
}

/**
 * @brief Create with explicit parameters and default allocator.
 */
zenit_bloom_t *zenit_bloom_create_explicit(size_t num_bits, size_t num_hashes) {
    return zenit_bloom_create_explicit_with_allocator(num_bits, num_hashes, ZENIT_ALLOCATOR_DEFAULT);
}

/**
 * @brief Create with explicit parameters and custom allocator.
 *
 * Estimates the false positive rate as 0.5^num_hashes (derived from the
 * optimal formula when capacity is chosen to match the parameters).
 */
zenit_bloom_t *zenit_bloom_create_explicit_with_allocator(size_t num_bits, size_t num_hashes, zenit_allocator_t allocator) {
    /* Estimate FPR as 0.5^k — avoids needing log/pow at this call site */
    double fpr = 1.0;
    for (size_t i = 0; i < num_hashes; i++) {
        fpr *= 0.5;
    }

    /* Delegate to the common creator */
    return bloom_create_common(num_bits, num_hashes, fpr, allocator);
}

/**
 * @brief Destroy a Bloom filter.
 *
 * NULL-safe per contract.  Frees the bit array and the handle using
 * the allocator stored at creation time.
 */
void zenit_bloom_destroy(zenit_bloom_t *bf) {
    if (bf == NULL) {
        return;
    }
    /* Read the allocator before freeing — it is needed to free both fields */
    zenit_allocator_t a = bf->allocator;
    /* Free the bit array first, then the handle */
    a.free_fn(bf->bits, a.ctx);
    a.free_fn(bf, a.ctx);
}

/**
 * @brief Insert an element.
 *
 * Computes two independent hash values (h1, h2) via FNV-1a, then generates
 * num_hashes positions using double hashing: pos = (h1 + i * h2) % num_bits.
 * Each position is set in the bit array.  NULL parameters are silently
 * ignored.
 */
void zenit_bloom_insert(zenit_bloom_t *bf, const void *data, size_t len) {
    /* Guard against NULL — matches the pattern used elsewhere in the library */
    if (bf == NULL || data == NULL) {
        return;
    }

    /* Compute the two independent hash values */
    size_t h1 = hash_fnv1a(data, len);
    size_t h2 = hash_fnv1a_alt(data, len) | 1; /* ensure h2 is odd for better distribution */

    /* Generate each hash position and set the corresponding bit */
    for (size_t i = 0; i < bf->num_hashes; i++) {
        size_t idx = (h1 + i * h2) % bf->num_bits;
        bit_set(bf->bits, idx);
    }

    /* Increment the approximate counter */
    bf->count++;
}

/**
 * @brief Test membership.
 *
 * Computes the same hash positions as insert().  If any bit is clear, the
 * element is definitely absent (returns 0).  If all bits are set, the
 * element may be present (returns 1).
 */
int zenit_bloom_contains(const zenit_bloom_t *bf, const void *data, size_t len) {
    /* Guard against NULL */
    if (bf == NULL || data == NULL) {
        return 0;
    }

    /* Compute the two independent hash values */
    size_t h1 = hash_fnv1a(data, len);
    size_t h2 = hash_fnv1a_alt(data, len) | 1;

    /* Check each hash position — early exit on first clear bit */
    for (size_t i = 0; i < bf->num_hashes; i++) {
        size_t idx = (h1 + i * h2) % bf->num_bits;
        if (!bit_test(bf->bits, idx)) {
            /* At least one bit is clear — element is definitely absent */
            return 0;
        }
    }

    /* All bits are set — element may be present */
    return 1;
}

/**
 * @brief Clear all bits.
 *
 * Zeroes the entire uint64_t array and resets the count to 0.
 */
void zenit_bloom_clear(zenit_bloom_t *bf) {
    if (bf == NULL) {
        return;
    }
    size_t arr_size = bf->num_bits / 64;
    /* Zero out all words in the bit array */
    memset(bf->bits, 0, arr_size * sizeof(uint64_t));
    bf->count = 0;
}

/**
 * @brief Return the configured false positive rate.
 */
double zenit_bloom_false_positive_rate(const zenit_bloom_t *bf) {
    if (bf == NULL) {
        return 0.0;
    }
    return bf->fpr;
}

/**
 * @brief Return the approximate insertion count.
 */
size_t zenit_bloom_count(const zenit_bloom_t *bf) {
    if (bf == NULL) {
        return 0;
    }
    return bf->count;
}

/**
 * @brief Return the total number of bits.
 */
size_t zenit_bloom_num_bits(const zenit_bloom_t *bf) {
    if (bf == NULL) {
        return 0;
    }
    return bf->num_bits;
}

/**
 * @brief Return the number of hash functions.
 */
size_t zenit_bloom_num_hashes(const zenit_bloom_t *bf) {
    if (bf == NULL) {
        return 0;
    }
    return bf->num_hashes;
}
