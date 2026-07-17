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

#include <libzenit/bitset.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal bitset state.
 *
 * Bits are stored in a contiguous byte array.  Byte @c i holds bits
 * @c i*8 … i*8+7, with bit 0 being the least significant bit of the
 * byte.  The array grows on demand via realloc.
 */
struct zenit_bitset_t {
    unsigned char *bits;      /**< Contiguous byte storage for bits */
    size_t num_bytes;         /**< Number of bytes currently allocated */
    size_t num_bits;          /**< Number of usable bits (may be less than num_bytes*8) */
    zenit_allocator_t allocator; /**< Memory allocator */
};

/* Growth factor for auto-resize — double the current capacity */
#define BITSET_GROWTH_FACTOR(cap) ((cap) * 2)

/**
 * @brief Return the number of bytes needed to store @p bit_count bits.
 */
static inline size_t bytes_needed(size_t bit_count) {
    return (bit_count + 7) / 8;
}

/**
 * @brief Auto-resize the bitset to accommodate a position.
 *
 * If @p required_pos is greater than or equal to num_bits, the bitset is
 * grown to at least @p required_pos + 1 bits.  Grows by a factor of 2x
 * to amortise reallocation.
 *
 * @return ZENIT_RESULT_OK or ZENIT_ERROR_ALLOC.
 */
static zenit_result_t ensure_capacity(zenit_bitset_t* bs, size_t required_pos) {
    /* Calculate how many bits are needed */
    size_t needed = required_pos + 1;

    /* No resize needed if we already have room */
    if (needed <= bs->num_bits) {
        return ZENIT_RESULT_OK;
    }

    /* Compute the new bit capacity — at least 2x current, or needed if bigger */
    size_t new_cap = bs->num_bits;
    while (new_cap < needed) {
        if (new_cap == 0) {
            new_cap = 64; /* Start with a minimum of 64 bits */
        } else {
            new_cap = BITSET_GROWTH_FACTOR(new_cap);
        }
    }

    return zenit_bitset_resize(bs, new_cap);
}

zenit_bitset_t* zenit_bitset_create(size_t num_bits) {
    return zenit_bitset_create_with_allocator(num_bits, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_bitset_t* zenit_bitset_create_with_allocator(size_t num_bits, zenit_allocator_t allocator) {
    /* Allocate the handle using the custom allocator */
    zenit_bitset_t* bs = allocator.alloc_fn(sizeof(zenit_bitset_t), allocator.ctx);
    if (bs == NULL) {
        return NULL;
    }

    /* Store the allocator early so failure paths can free correctly */
    bs->allocator = allocator;

    /* Compute byte count and allocate the bit storage */
    size_t nb = bytes_needed(num_bits);
    bs->bits = zenit_allocator_alloc_zero(allocator, 1, nb);
    if (bs->bits == NULL && nb > 0) {
        allocator.free_fn(bs, allocator.ctx);
        return NULL;
    }

    bs->num_bytes = nb;
    bs->num_bits = num_bits;

    return bs;
}

void zenit_bitset_destroy(zenit_bitset_t* bs) {
    if (bs == NULL) {
        return;
    }
    zenit_allocator_t a = bs->allocator;
    a.free_fn(bs->bits, a.ctx);
    a.free_fn(bs, a.ctx);
}

zenit_result_t zenit_bitset_set(zenit_bitset_t* bs, size_t pos) {
    if (bs == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Grow if needed */
    zenit_result_t r = ensure_capacity(bs, pos);
    if (r.error != ZENIT_OK) {
        return r;
    }

    /* Compute byte index and bit mask, then set */
    size_t byte = pos / 8;
    unsigned char bit = (unsigned char)(pos % 8);
    bs->bits[byte] |= (unsigned char)(1u << bit);

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_bitset_clear(zenit_bitset_t* bs, size_t pos) {
    if (bs == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Grow if needed */
    zenit_result_t r = ensure_capacity(bs, pos);
    if (r.error != ZENIT_OK) {
        return r;
    }

    /* Compute byte index and bit mask, then clear */
    size_t byte = pos / 8;
    unsigned char bit = (unsigned char)(pos % 8);
    bs->bits[byte] &= (unsigned char)~(1u << bit);

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_bitset_toggle(zenit_bitset_t* bs, size_t pos) {
    if (bs == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Grow if needed */
    zenit_result_t r = ensure_capacity(bs, pos);
    if (r.error != ZENIT_OK) {
        return r;
    }

    /* Compute byte index and bit mask, then toggle */
    size_t byte = pos / 8;
    unsigned char bit = (unsigned char)(pos % 8);
    bs->bits[byte] ^= (unsigned char)(1u << bit);

    return ZENIT_RESULT_OK;
}

int zenit_bitset_test(const zenit_bitset_t* bs, size_t pos) {
    if (bs == NULL) {
        return 0;
    }
    if (pos >= bs->num_bits) {
        return 0;
    }

    /* Compute byte index and bit mask, then extract */
    size_t byte = pos / 8;
    unsigned char bit = (unsigned char)(pos % 8);
    return (bs->bits[byte] >> bit) & 1u;
}

zenit_result_t zenit_bitset_set_all(zenit_bitset_t* bs) {
    if (bs == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Set every byte to 0xFF — all bits become 1 */
    if (bs->num_bytes > 0) {
        memset(bs->bits, 0xFF, bs->num_bytes);
    }

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_bitset_clear_all(zenit_bitset_t* bs) {
    if (bs == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Clear every byte to 0x00 — all bits become 0 */
    if (bs->num_bytes > 0) {
        memset(bs->bits, 0x00, bs->num_bytes);
    }

    return ZENIT_RESULT_OK;
}

size_t zenit_bitset_count(const zenit_bitset_t* bs) {
    if (bs == NULL) {
        return 0;
    }

    size_t count = 0;

    /* Iterate over every byte and accumulate the population count */
    for (size_t i = 0; i < bs->num_bytes; i++) {
        unsigned char byte = bs->bits[i];

#if defined(__GNUC__) || defined(__clang__)
        /* Use the compiler builtin where available — single instruction */
        count += (size_t)__builtin_popcount((unsigned int)byte);
#else
        /* Fallback: Brian Kernighan's algorithm */
        while (byte != 0) {
            byte &= (unsigned char)(byte - 1);
            count++;
        }
#endif
    }

    return count;
}

size_t zenit_bitset_capacity(const zenit_bitset_t* bs) {
    if (bs == NULL) {
        return 0;
    }
    return bs->num_bits;
}

zenit_result_t zenit_bitset_resize(zenit_bitset_t* bs, size_t num_bits) {
    if (bs == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    size_t new_bytes = bytes_needed(num_bits);

    /* If new_bytes is 0, free the buffer */
    if (new_bytes == 0) {
        zenit_allocator_t a = bs->allocator;
        a.free_fn(bs->bits, a.ctx);
        bs->bits = NULL;
        bs->num_bytes = 0;
        bs->num_bits = 0;
        return ZENIT_RESULT_OK;
    }

    /* Reallocate the byte array */
    zenit_allocator_t a = bs->allocator;
    unsigned char *new_bits = zenit_allocator_realloc(a, bs->bits, bs->num_bytes, new_bytes);
    if (new_bits == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    /* Zero-initialise the newly allocated bytes (if growing) */
    if (new_bytes > bs->num_bytes) {
        memset(new_bits + bs->num_bytes, 0, new_bytes - bs->num_bytes);
    }

    bs->bits = new_bits;
    bs->num_bytes = new_bytes;
    bs->num_bits = num_bits;

    return ZENIT_RESULT_OK;
}
