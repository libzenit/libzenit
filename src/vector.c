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

#include <libzenit/vector.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

/* Default initial capacity when none is specified */
#define VECTOR_DEFAULT_CAPACITY 8
/* Growth factor = 1.5x */
#define VECTOR_GROWTH_FACTOR(num) ((num) + (num) / 2)

/**
 * @brief Internal vector state.
 *
 * Stores elements as raw bytes in a contiguous heap buffer.
 * Capacity grows by 1.5x on demand.  No automatic shrinking.
 */
struct zenit_vector_t {
    unsigned char *buffer; /**< Element storage */
    size_t elem_size;      /**< Size in bytes of one element */
    size_t count;          /**< Number of elements stored */
    size_t capacity;       /**< Number of element slots allocated */
    zenit_allocator_t allocator; /**< Memory allocator */
};

/**
 * @brief Helper: reallocate the internal buffer to a new capacity.
 *
 * If the new capacity is 0, the buffer is freed (which allows shrink
 * to zero).  On failure the old buffer is preserved.
 *
 * @return ZENIT_RESULT_OK or ZENIT_ERROR_ALLOC.
 */
static zenit_result_t realloc_buffer(zenit_vector_t *vector, size_t new_cap) {
    zenit_allocator_t a = vector->allocator;

    /* If we are asked to shrink to zero, free the buffer */
    if (new_cap == 0) {
        a.free_fn(vector->buffer, a.ctx);
        vector->buffer = NULL;
        vector->capacity = 0;
        return ZENIT_RESULT_OK;
    }

    unsigned char *new_buf = zenit_allocator_realloc(a, vector->buffer, vector->capacity * vector->elem_size, new_cap * vector->elem_size);
    if (new_buf == NULL) {
        /* Old buffer is still valid; caller may retry with smaller capacity */
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    vector->buffer = new_buf;
    vector->capacity = new_cap;
    return ZENIT_RESULT_OK;
}

/**
 * @brief Helper: grow the buffer so at least @p min_capacity slots exist.
 *
 * Grows exponentially (>= 1.5x) to amortise reallocation cost.
 */
static zenit_result_t grow(zenit_vector_t *vector, size_t min_capacity) {
    size_t new_cap = vector->capacity;

    /* Grow exponentially until we meet or exceed min_capacity */
    while (new_cap < min_capacity) {
        if (new_cap == 0) {
            new_cap = VECTOR_DEFAULT_CAPACITY;
        } else {
            new_cap = VECTOR_GROWTH_FACTOR(new_cap);
        }
    }

    return realloc_buffer(vector, new_cap);
}

zenit_vector_t *zenit_vector_create_with_allocator(size_t elem_size, zenit_allocator_t allocator) {
    /* Element size must be positive — zero would break all operations */
    if (elem_size == 0) {
        return NULL;
    }

    /* Allocate the handle using the custom allocator */
    zenit_vector_t *vector = allocator.alloc_fn(sizeof(zenit_vector_t), allocator.ctx);
    if (vector == NULL) {
        return NULL;
    }

    /* Start with default capacity — no allocation until first element */
    vector->buffer = NULL;
    vector->elem_size = elem_size;
    vector->count = 0;
    vector->capacity = 0;
    vector->allocator = allocator;

    return vector;
}

zenit_vector_t *zenit_vector_create(size_t elem_size) {
    return zenit_vector_create_with_allocator(elem_size, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_vector_t *zenit_vector_create_with_capacity_and_allocator(size_t elem_size, size_t capacity, zenit_allocator_t allocator) {
    /* Reject zero for either parameter */
    if (elem_size == 0 || capacity == 0) {
        return NULL;
    }

    /* Allocate the handle using the custom allocator */
    zenit_vector_t *vector = allocator.alloc_fn(sizeof(zenit_vector_t), allocator.ctx);
    if (vector == NULL) {
        return NULL;
    }

    /* Pre-allocate the element buffer */
    vector->buffer = allocator.alloc_fn(capacity * elem_size, allocator.ctx);
    if (vector->buffer == NULL) {
        allocator.free_fn(vector, allocator.ctx);
        return NULL;
    }

    vector->elem_size = elem_size;
    vector->count = 0;
    vector->capacity = capacity;
    vector->allocator = allocator;

    return vector;
}

zenit_vector_t *zenit_vector_create_with_capacity(size_t elem_size, size_t capacity) {
    return zenit_vector_create_with_capacity_and_allocator(elem_size, capacity, ZENIT_ALLOCATOR_DEFAULT);
}

void zenit_vector_destroy(zenit_vector_t *vector) {
    if (vector == NULL) {
        return;
    }
    /* Free the element buffer first, then the handle */
    zenit_allocator_t a = vector->allocator;
    a.free_fn(vector->buffer, a.ctx);
    a.free_fn(vector, a.ctx);
}

zenit_result_t zenit_vector_push(zenit_vector_t *vector, const void *elem) {
    /* Validate preconditions */
    if (vector == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Grow if the buffer is full */
    if (vector->count == vector->capacity) {
        zenit_result_t r = grow(vector, vector->capacity + 1);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }

    /* Copy the element into the next slot */
    unsigned char *dest = vector->buffer + vector->count * vector->elem_size;
    memcpy(dest, elem, vector->elem_size);
    vector->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_vector_pop(zenit_vector_t *vector, void *out_elem) {
    if (vector == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (vector->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    vector->count--;
    const unsigned char *src = vector->buffer + vector->count * vector->elem_size;
    memcpy(out_elem, src, vector->elem_size);

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_vector_insert(zenit_vector_t *vector, size_t index, const void *elem) {
    if (vector == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (index > vector->count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    if (vector->count == vector->capacity) {
        zenit_result_t r = grow(vector, vector->count + 1);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }

    if (index < vector->count) {
        unsigned char *dest = vector->buffer + (index + 1) * vector->elem_size;
        const unsigned char *src = vector->buffer + index * vector->elem_size;
        size_t bytes = (vector->count - index) * vector->elem_size;
        memmove(dest, src, bytes);
    }

    unsigned char *dest = vector->buffer + index * vector->elem_size;
    memcpy(dest, elem, vector->elem_size);
    vector->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_vector_remove(zenit_vector_t *vector, size_t index, void *out_elem) {
    if (vector == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (index >= vector->count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    const unsigned char *src = vector->buffer + index * vector->elem_size;
    memcpy(out_elem, src, vector->elem_size);

    if (index + 1 < vector->count) {
        unsigned char *dest = vector->buffer + index * vector->elem_size;
        const unsigned char *src_shift = vector->buffer + (index + 1) * vector->elem_size;
        size_t bytes = (vector->count - index - 1) * vector->elem_size;
        memmove(dest, src_shift, bytes);
    }

    vector->count--;

    return ZENIT_RESULT_OK;
}

void *zenit_vector_get(const zenit_vector_t *vector, size_t index) {
    if (vector == NULL) {
        return NULL;
    }
    if (index >= vector->count) {
        return NULL;
    }
    return vector->buffer + index * vector->elem_size;
}

zenit_result_t zenit_vector_set(zenit_vector_t *vector, size_t index, const void *elem) {
    if (vector == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (index >= vector->count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    unsigned char *dest = vector->buffer + index * vector->elem_size;
    memcpy(dest, elem, vector->elem_size);

    return ZENIT_RESULT_OK;
}

size_t zenit_vector_count(const zenit_vector_t *vector) {
    if (vector == NULL) {
        return 0;
    }
    return vector->count;
}

size_t zenit_vector_capacity(const zenit_vector_t *vector) {
    if (vector == NULL) {
        return 0;
    }
    return vector->capacity;
}

zenit_result_t zenit_vector_reserve(zenit_vector_t *vector, size_t capacity) {
    if (vector == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (capacity <= vector->capacity) {
        return ZENIT_RESULT_OK;
    }
    return realloc_buffer(vector, capacity);
}

zenit_result_t zenit_vector_shrink_to_fit(zenit_vector_t *vector) {
    if (vector == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (vector->count == vector->capacity) {
        return ZENIT_RESULT_OK;
    }
    return realloc_buffer(vector, vector->count);
}

void zenit_vector_clear(zenit_vector_t *vector) {
    if (vector == NULL) {
        return;
    }
    vector->count = 0;
}

int zenit_vector_empty(const zenit_vector_t *vector) {
    if (vector == NULL) {
        return 1;
    }
    return vector->count == 0 ? 1 : 0;
}

zenit_iter_t zenit_vector_iter(const zenit_vector_t *vector) {
    zenit_iter_t iter;
    iter.container = (void*)vector;
    iter.index = 0;
    iter.count = vector ? vector->count : 0;
    iter.is_valid = (vector != NULL) ? 1 : 0;
    iter.internal = NULL;
    return iter;
}

void *zenit_vector_iter_next(zenit_iter_t *iter) {
    if (iter == NULL || !iter->is_valid) {
        return NULL;
    }
    if (iter->index >= iter->count) {
        return NULL;
    }
    const zenit_vector_t *v = (const zenit_vector_t*)iter->container;
    void *elem = v->buffer + iter->index * v->elem_size;
    iter->index++;
    return elem;
}

void *zenit_vector_data(zenit_vector_t *vector) {
    if (vector == NULL) {
        return NULL;
    }
    return vector->buffer;
}
