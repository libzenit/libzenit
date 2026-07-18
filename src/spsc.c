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

#include <libzenit/spsc.h>
#include <stdlib.h>
#include <string.h>

/* Round up to the nearest power of two */
static uint32_t round_pow2_u32(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

zenit_spsc_t* zenit_spsc_create(size_t elem_size, size_t capacity) {
    if (elem_size == 0 || capacity == 0) {
        return NULL;
    }

    /* Round capacity to power of two */
    uint32_t cap = round_pow2_u32((uint32_t)capacity);
    if (cap == 0) cap = (uint32_t)capacity; /* overflow */

    zenit_spsc_t *spsc = malloc(sizeof(zenit_spsc_t));
    if (spsc == NULL) {
        return NULL;
    }

    spsc->buffer = malloc(elem_size * cap);
    if (spsc->buffer == NULL) {
        free(spsc);
        return NULL;
    }

    spsc->capacity = (size_t)cap;
    spsc->elem_size = elem_size;
    spsc->head = 0;
    spsc->tail = 0;

    return spsc;
}

void zenit_spsc_destroy(zenit_spsc_t *spsc) {
    if (spsc == NULL) {
        return;
    }
    free(spsc->buffer);
    free(spsc);
}

zenit_result_t zenit_spsc_push(zenit_spsc_t *spsc, const void *elem) {
    if (spsc == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    uint32_t head = spsc->head;
    uint32_t tail = spsc->tail;
    uint32_t mask = (uint32_t)(spsc->capacity - 1);

    /* Check if full: head - tail == capacity */
    if ((head - tail) >= (uint32_t)spsc->capacity) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_FULL);
    }

    memcpy(spsc->buffer + (head & mask) * spsc->elem_size, elem, spsc->elem_size);

    /* Write barrier: ensure data is visible before updating head */
    __sync_synchronize();
    spsc->head = head + 1;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_spsc_pop(zenit_spsc_t *spsc, void *out) {
    if (spsc == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    uint32_t head = spsc->head;
    uint32_t tail = spsc->tail;
    uint32_t mask = (uint32_t)(spsc->capacity - 1);

    /* Check if empty */
    if (head == tail) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    memcpy(out, spsc->buffer + (tail & mask) * spsc->elem_size, spsc->elem_size);

    /* Write barrier: ensure data is read before updating tail */
    __sync_synchronize();
    spsc->tail = tail + 1;

    return ZENIT_RESULT_OK;
}

size_t zenit_spsc_count(const zenit_spsc_t *spsc) {
    if (spsc == NULL) {
        return 0;
    }
    return (size_t)(spsc->head - spsc->tail);
}

size_t zenit_spsc_capacity(const zenit_spsc_t *spsc) {
    if (spsc == NULL) {
        return 0;
    }
    return spsc->capacity;
}

int zenit_spsc_full(const zenit_spsc_t *spsc) {
    if (spsc == NULL) {
        return 0;
    }
    return (spsc->head - spsc->tail) >= (uint32_t)spsc->capacity ? 1 : 0;
}

int zenit_spsc_empty(const zenit_spsc_t *spsc) {
    if (spsc == NULL) {
        return 0;
    }
    return spsc->head == spsc->tail ? 1 : 0;
}
