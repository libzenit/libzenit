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

#ifndef LIBZENIT_SPSC_H
#define LIBZENIT_SPSC_H

#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Lock-free single-producer single-consumer ring buffer.
 *
 * Designed for thread-safe communication between two threads (one producer,
 * one consumer).  Uses volatile and memory ordering to ensure correctness
 * without mutexes.
 *
 * Capacity is fixed at creation time.  Push fails on full, pop fails on empty.
 * All operations are O(1) and wait-free.
 */
typedef struct {
    unsigned char *buffer;  /**< Internal data buffer */
    size_t capacity;        /**< Maximum number of elements */
    size_t elem_size;       /**< Size of each element in bytes */
    volatile uint32_t head; /**< Write index (producer-only) */
    volatile uint32_t tail; /**< Read index (consumer-only) */
} zenit_spsc_t;

/**
 * @brief Create a fixed-capacity SPSC ring buffer.
 *
 * The capacity is rounded up to the nearest power of two for efficient
 * masking.
 *
 * @param elem_size Size of each element in bytes (must be > 0).
 * @param capacity  Maximum number of elements (must be > 0).
 * @return New SPSC buffer, or NULL on invalid params or OOM.
 */
zenit_spsc_t* zenit_spsc_create(size_t elem_size, size_t capacity);

/**
 * @brief Destroy an SPSC ring buffer.  NULL-safe.
 *
 * @param spsc Buffer to destroy.
 */
void zenit_spsc_destroy(zenit_spsc_t *spsc);

/**
 * @brief Push an element (producer side).
 *
 * Copies @p elem into the buffer.  Fails if the buffer is full.
 * Thread-safe — only one thread (the producer) should call this.
 *
 * @param spsc Buffer handle.
 * @param elem Pointer to the element to push.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_FULL if the buffer is full,
 *         or ZENIT_ERROR_NULL on NULL params.
 */
zenit_result_t zenit_spsc_push(zenit_spsc_t *spsc, const void *elem);

/**
 * @brief Pop an element (consumer side).
 *
 * Copies the oldest element into @p out.  Fails if the buffer is empty.
 * Thread-safe — only one thread (the consumer) should call this.
 *
 * @param spsc Buffer handle.
 * @param out  Pointer to receive the element.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_EMPTY if the buffer is empty,
 *         or ZENIT_ERROR_NULL on NULL params.
 */
zenit_result_t zenit_spsc_pop(zenit_spsc_t *spsc, void *out);

/**
 * @brief Get the number of elements currently in the buffer.
 *
 * @param spsc Buffer handle.  If NULL, returns 0.
 * @return Number of elements stored.
 */
size_t zenit_spsc_count(const zenit_spsc_t *spsc);

/**
 * @brief Get the maximum capacity of the buffer.
 *
 * @param spsc Buffer handle.  If NULL, returns 0.
 * @return Maximum number of elements.
 */
size_t zenit_spsc_capacity(const zenit_spsc_t *spsc);

/**
 * @brief Check if the buffer is full.
 *
 * @param spsc Buffer handle.
 * @return 1 if full, 0 otherwise (or if NULL).
 */
int zenit_spsc_full(const zenit_spsc_t *spsc);

/**
 * @brief Check if the buffer is empty.
 *
 * @param spsc Buffer handle.
 * @return 1 if empty, 0 otherwise (or if NULL).
 */
int zenit_spsc_empty(const zenit_spsc_t *spsc);

#endif
