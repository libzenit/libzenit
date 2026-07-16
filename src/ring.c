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
};

/**
 * @brief Create a ring buffer.
 *
 * Allocates the handle and the internal byte array in one call.
 * Returns NULL if either allocation fails or capacity == 0.
 */
zenit_ring_t *zenit_ring_create(size_t capacity) {
    /* Capacity must be at least 1 — zero makes no sense for a buffer */
    if (capacity == 0) {
        return NULL;
    }

    /* Allocate the handle */
    zenit_ring_t *ring = malloc(sizeof(zenit_ring_t));
    if (ring == NULL) {
        return NULL;
    }

    /* Allocate the byte storage — calloc so it is zeroed initially */
    ring->buffer = calloc(1, capacity);
    if (ring->buffer == NULL) {
        /* Handle allocation failed — free the handle we just allocated */
        free(ring);
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
    /* Free the byte array first, then the handle */
    free(ring->buffer);
    free(ring);
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
