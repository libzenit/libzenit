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

#include <libzenit/ring.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal ring-buffer state.
 *
 * Uses a contiguous byte array with head (write) and tail (read) cursors.
 * Both cursors wrap around modulo @p capacity.  @p count is maintained
 * separately so we can distinguish full from empty without wasting a slot.
 */
struct zenit_ring_t {
    unsigned char *buffer; /**< Contiguous byte storage */
    size_t capacity;       /**< Total number of bytes the buffer can hold */
    size_t head;           /**< Next write offset (0 … capacity-1) */
    size_t tail;           /**< Next read offset  (0 … capacity-1) */
    size_t count;          /**< Number of bytes currently stored */
    zenit_allocator_t allocator; /**< Custom allocator used for this instance */
};

/**
 * @brief Create a ring buffer.
 *
 * Allocates the handle and the internal byte array in one call.
 * Returns NULL if either allocation fails or capacity == 0.
 */
zenit_ring_t *zenit_ring_create(size_t capacity) {
    /* Delegate to the with-allocator variant using the default allocator */
    return zenit_ring_create_with_allocator(capacity, ZENIT_ALLOCATOR_DEFAULT);
}

/**
 * @brief Create a ring buffer with a custom allocator.
 *
 * Allocates the handle and the internal byte array via the provided allocator.
 * Returns NULL if either allocation fails or capacity == 0.
 */
zenit_ring_t *zenit_ring_create_with_allocator(size_t capacity, zenit_allocator_t allocator) {
    /* Capacity must be at least 1 — zero makes no sense for a buffer */
    if (capacity == 0) {
        return NULL;
    }

    /* Allocate the handle — use the allocator directly, not the struct field */
    zenit_ring_t *ring = allocator.alloc_fn(sizeof(zenit_ring_t), allocator.ctx);
    if (ring == NULL) {
        return NULL;
    }

    /* Store the allocator so destroy uses the correct free function */
    ring->allocator = allocator;

    /* Allocate the byte storage — use alloc_zero so it is zeroed initially */
    ring->buffer = zenit_allocator_alloc_zero(allocator, 1, capacity);
    if (ring->buffer == NULL) {
        /* Buffer allocation failed — free the handle we just allocated */
        allocator.free_fn(ring, allocator.ctx);
        return NULL;
    }

    /* Initialise cursor and counter */
    ring->capacity = capacity;
    ring->head = 0;
    ring->tail = 0;
    ring->count = 0;

    return ring;
}

/**
 * @brief Destroy a ring buffer.
 *
 * Frees both the internal buffer and the handle itself.
 * NULL-safe per contract.
 */
void zenit_ring_destroy(zenit_ring_t *ring) {
    /* free(NULL) is safe; this guard lets us keep the comment explicit */
    if (ring == NULL) {
        return;
    }
    /* Read the allocator before freeing — we need it to free the buffer and handle */
    zenit_allocator_t a = ring->allocator;
    /* Free the byte array first, then the handle */
    a.free_fn(ring->buffer, a.ctx);
    a.free_fn(ring, a.ctx);
}

/**
 * @brief Push bytes into the ring buffer.
 *
 * Copies @p size bytes from @p data into the buffer at the current head
 * position.  Advances head modulo capacity.  Fails with ZENIT_ERROR_FULL
 * if there is not enough room.
 */
zenit_result_t zenit_ring_push(zenit_ring_t *ring, const void *data, size_t size) {
    /* Validate all preconditions before touching state */
    if (ring == NULL || data == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (size == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    /* Check whether there is enough free space */
    if (ring->count + size > ring->capacity) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_FULL);
    }

    /* We have to handle wrap-around: the data may need two memcpy calls
     * if head is near the end of the buffer. */

    /* First segment: from head to end-of-buffer (or as much as needed) */
    size_t first_chunk = ring->capacity - ring->head;
    if (first_chunk > size) {
        first_chunk = size;
    }
    memcpy(ring->buffer + ring->head, data, first_chunk);

    /* Second segment: wrap around to the beginning (if needed) */
    size_t second_chunk = size - first_chunk;
    if (second_chunk > 0) {
        memcpy(ring->buffer, (const unsigned char *)data + first_chunk, second_chunk);
    }

    /* Advance head (wrap modulo capacity) */
    ring->head = (ring->head + size) % ring->capacity;
    ring->count += size;

    return ZENIT_RESULT_OK;
}

/**
 * @brief Pop bytes from the ring buffer.
 *
 * Copies up to @p size bytes from tail into @p data, advances tail,
 * and writes the actual byte count into @p *size.  Fails with
 * ZENIT_ERROR_EMPTY when no data is available.
 */
zenit_result_t zenit_ring_pop(zenit_ring_t *ring, void *data, size_t *size) {
    /* Validate preconditions */
    if (ring == NULL || data == NULL || size == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (*size == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    /* Nothing to read */
    if (ring->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    /* Clamp requested bytes to what is actually available */
    size_t to_read = *size;
    if (to_read > ring->count) {
        to_read = ring->count;
    }

    /* Handle wrap-around same as push */
    size_t first_chunk = ring->capacity - ring->tail;
    if (first_chunk > to_read) {
        first_chunk = to_read;
    }
    memcpy(data, ring->buffer + ring->tail, first_chunk);

    size_t second_chunk = to_read - first_chunk;
    if (second_chunk > 0) {
        memcpy((unsigned char *)data + first_chunk, ring->buffer, second_chunk);
    }

    /* Advance tail (wrap modulo capacity) */
    ring->tail = (ring->tail + to_read) % ring->capacity;
    ring->count -= to_read;

    /* Report how many bytes we actually copied */
    *size = to_read;
    return ZENIT_RESULT_OK;
}

/**
 * @brief Peek at data without consuming it.
 *
 * Identical to zenit_ring_pop() except that tail and count are not
 * modified — the data remains available for a subsequent read.
 */
zenit_result_t zenit_ring_peek(const zenit_ring_t *ring, void *data, size_t *size) {
    /* Validate preconditions */
    if (ring == NULL || data == NULL || size == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (*size == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    if (ring->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    /* Clamp to available data */
    size_t to_read = *size;
    if (to_read > ring->count) {
        to_read = ring->count;
    }

    /* Peek uses the same wrap logic but operates on a local copy of tail */
    size_t local_tail = ring->tail;

    size_t first_chunk = ring->capacity - local_tail;
    if (first_chunk > to_read) {
        first_chunk = to_read;
    }
    memcpy(data, ring->buffer + local_tail, first_chunk);

    size_t second_chunk = to_read - first_chunk;
    if (second_chunk > 0) {
        memcpy((unsigned char *)data + first_chunk, ring->buffer, second_chunk);
    }

    /* Do NOT advance tail or decrement count — peek is read-only */

    *size = to_read;
    return ZENIT_RESULT_OK;
}

/**
 * @brief Return the number of bytes stored.
 */
size_t zenit_ring_count(const zenit_ring_t *ring) {
    if (ring == NULL) {
        return 0;
    }
    return ring->count;
}

/**
 * @brief Return the total capacity.
 */
size_t zenit_ring_capacity(const zenit_ring_t *ring) {
    if (ring == NULL) {
        return 0;
    }
    return ring->capacity;
}

/**
 * @brief Clear all data — reset cursors without freeing memory.
 */
void zenit_ring_clear(zenit_ring_t *ring) {
    if (ring == NULL) {
        return;
    }
    /* Reset head, tail, and count.  The buffer itself is left untouched. */
    ring->head = 0;
    ring->tail = 0;
    ring->count = 0;
}

/**
 * @brief Resize the ring buffer to a new capacity.
 *
 * Allocates a new buffer of @p new_capacity bytes, copies as much existing
 * data as fits (oldest first), then frees the old buffer.  If the new capacity
 * is smaller than the current count, the oldest bytes are dropped.
 */
zenit_result_t zenit_ring_reserve(zenit_ring_t *ring, size_t new_capacity) {
    if (ring == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (new_capacity == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    zenit_allocator_t a = ring->allocator;

    /* Allocate the new buffer */
    unsigned char *new_buf = a.alloc_fn(new_capacity, a.ctx);
    if (new_buf == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    /* Copy as much data as fits, starting from tail (oldest) */
    size_t to_copy = ring->count;
    if (to_copy > new_capacity) {
        to_copy = new_capacity;
    }

    if (to_copy > 0) {
        /* First segment: from tail to end of old buffer */
        size_t first = ring->capacity - ring->tail;
        if (first > to_copy) {
            first = to_copy;
        }
        memcpy(new_buf, ring->buffer + ring->tail, first);

        /* Second segment: wrap around if needed */
        size_t second = to_copy - first;
        if (second > 0) {
            memcpy(new_buf + first, ring->buffer, second);
        }
    }

    /* Free the old buffer */
    a.free_fn(ring->buffer, a.ctx);

    /* Update state */
    ring->buffer = new_buf;
    ring->capacity = new_capacity;
    ring->tail = 0;
    ring->head = to_copy;
    ring->count = to_copy;

    return ZENIT_RESULT_OK;
}

/**
 * @brief Shrink the internal buffer to exactly fit the current data.
 *
 * If the ring is empty, capacity becomes 1.
 */
zenit_result_t zenit_ring_shrink_to_fit(zenit_ring_t *ring) {
    if (ring == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    size_t target = ring->count;
    if (target == 0) {
        target = 1;
    }
    return zenit_ring_reserve(ring, target);
}
