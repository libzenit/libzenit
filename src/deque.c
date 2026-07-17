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

#include <libzenit/deque.h>
#include <stdlib.h>
#include <string.h>

/* Default initial capacity */
#define DEQUE_DEFAULT_CAPACITY 8
/* Growth factor = 1.5x */
#define DEQUE_GROWTH_FACTOR(num) ((num) + (num) / 2)

/* ─── Internal deque state ───
 * Uses a contiguous circular buffer.  head points to the front element;
 * tail points to the slot one past the last element.  Both wrap modulo
 * capacity.  When head == tail the deque is empty.
 */
struct zenit_deque_t {
    unsigned char *buffer;  /**< Element storage (capacity × elem_size bytes) */
    size_t elem_size;       /**< Size in bytes of one element */
    size_t count;           /**< Number of elements stored */
    size_t capacity;        /**< Number of element slots allocated */
    size_t head;            /**< Index (in element units) of the front element */
    size_t tail;            /**< Index (in element units) one past the last element */
};

/* ─── Helper: reallocate the internal buffer to a new capacity ───
 * This is more involved than a simple realloc because the elements may
 * wrap around in the circular buffer.  We linearise them so that head
 * becomes 0 and tail becomes count.
 */
static zenit_result_t realloc_buffer(zenit_deque_t *deque, size_t new_cap) {
    unsigned char *new_buf = malloc(new_cap * deque->elem_size);
    if (new_buf == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    size_t es = deque->elem_size;
    size_t count = deque->count;

    /* Copy elements in order from head → tail, linearising the wrap */
    for (size_t i = 0; i < count; i++) {
        size_t src_idx = (deque->head + i) % deque->capacity;
        memcpy(new_buf + i * es, deque->buffer + src_idx * es, es);
    }

    free(deque->buffer);
    deque->buffer = new_buf;
    deque->capacity = new_cap;
    deque->head = 0;
    deque->tail = count;

    return ZENIT_RESULT_OK;
}

/* ─── Helper: grow the buffer so at least min_capacity slots exist ─── */
static zenit_result_t grow(zenit_deque_t *deque, size_t min_capacity) {
    size_t new_cap = deque->capacity;

    while (new_cap < min_capacity) {
        new_cap = new_cap ? DEQUE_GROWTH_FACTOR(new_cap) : DEQUE_DEFAULT_CAPACITY;
    }

    return realloc_buffer(deque, new_cap);
}

/* ─── Public API ─── */

zenit_deque_t *zenit_deque_create(size_t elem_size) {
    return zenit_deque_create_with_capacity(elem_size, DEQUE_DEFAULT_CAPACITY);
}

zenit_deque_t *zenit_deque_create_with_capacity(size_t elem_size, size_t capacity) {
    if (elem_size == 0 || capacity == 0) {
        return NULL;
    }

    zenit_deque_t *deque = malloc(sizeof(zenit_deque_t));
    if (deque == NULL) {
        return NULL;
    }

    deque->buffer = malloc(capacity * elem_size);
    if (deque->buffer == NULL) {
        free(deque);
        return NULL;
    }

    deque->elem_size = elem_size;
    deque->count = 0;
    deque->capacity = capacity;
    deque->head = 0;
    deque->tail = 0;

    return deque;
}

void zenit_deque_destroy(zenit_deque_t *deque) {
    if (deque == NULL) {
        return;
    }
    free(deque->buffer);
    free(deque);
}

zenit_result_t zenit_deque_push_front(zenit_deque_t *deque, const void *elem) {
    if (deque == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    if (deque->count == deque->capacity) {
        zenit_result_t r = grow(deque, deque->capacity + 1);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }

    /* Move head backward (wrap around) */
    if (deque->head == 0) {
        deque->head = deque->capacity - 1;
    } else {
        deque->head--;
    }

    unsigned char *dest = deque->buffer + deque->head * deque->elem_size;
    memcpy(dest, elem, deque->elem_size);
    deque->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_deque_push_back(zenit_deque_t *deque, const void *elem) {
    if (deque == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    if (deque->count == deque->capacity) {
        zenit_result_t r = grow(deque, deque->capacity + 1);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }

    unsigned char *dest = deque->buffer + deque->tail * deque->elem_size;
    memcpy(dest, elem, deque->elem_size);

    deque->tail = (deque->tail + 1) % deque->capacity;
    deque->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_deque_pop_front(zenit_deque_t *deque, void *out_elem) {
    if (deque == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (deque->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    unsigned char *src = deque->buffer + deque->head * deque->elem_size;
    memcpy(out_elem, src, deque->elem_size);

    deque->head = (deque->head + 1) % deque->capacity;
    deque->count--;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_deque_pop_back(zenit_deque_t *deque, void *out_elem) {
    if (deque == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (deque->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    /* Move tail backward */
    if (deque->tail == 0) {
        deque->tail = deque->capacity - 1;
    } else {
        deque->tail--;
    }

    unsigned char *src = deque->buffer + deque->tail * deque->elem_size;
    memcpy(out_elem, src, deque->elem_size);
    deque->count--;

    return ZENIT_RESULT_OK;
}

void *zenit_deque_get(const zenit_deque_t *deque, size_t index) {
    if (deque == NULL) {
        return NULL;
    }
    if (index >= deque->count) {
        return NULL;
    }

    size_t offset = (deque->head + index) % deque->capacity;
    return deque->buffer + offset * deque->elem_size;
}

void *zenit_deque_front(const zenit_deque_t *deque) {
    if (deque == NULL || deque->count == 0) {
        return NULL;
    }
    return deque->buffer + deque->head * deque->elem_size;
}

void *zenit_deque_back(const zenit_deque_t *deque) {
    if (deque == NULL || deque->count == 0) {
        return NULL;
    }
    size_t back = (deque->tail == 0) ? deque->capacity - 1 : deque->tail - 1;
    return deque->buffer + back * deque->elem_size;
}

size_t zenit_deque_count(const zenit_deque_t *deque) {
    if (deque == NULL) {
        return 0;
    }
    return deque->count;
}

size_t zenit_deque_capacity(const zenit_deque_t *deque) {
    if (deque == NULL) {
        return 0;
    }
    return deque->capacity;
}

int zenit_deque_empty(const zenit_deque_t *deque) {
    if (deque == NULL) {
        return 1;
    }
    return deque->count == 0 ? 1 : 0;
}

zenit_result_t zenit_deque_reserve(zenit_deque_t *deque, size_t capacity) {
    if (deque == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (capacity <= deque->capacity) {
        return ZENIT_RESULT_OK;
    }
    return realloc_buffer(deque, capacity);
}

zenit_result_t zenit_deque_shrink_to_fit(zenit_deque_t *deque) {
    if (deque == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (deque->count == deque->capacity) {
        return ZENIT_RESULT_OK;
    }
    /* realloc_buffer with count will linearise and shrink */
    return realloc_buffer(deque, deque->count);
}

void zenit_deque_clear(zenit_deque_t *deque) {
    if (deque == NULL) {
        return;
    }
    deque->head = 0;
    deque->tail = 0;
    deque->count = 0;
}
