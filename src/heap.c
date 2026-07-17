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

#include <libzenit/heap.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

/* Default initial capacity */
#define HEAP_DEFAULT_CAPACITY 8
/* Growth factor = 1.5x */
#define HEAP_GROWTH_FACTOR(num) ((num) + (num) / 2)

/* ─── Internal heap state ─── */
struct zenit_heap_t {
    unsigned char *buffer;                     /**< Element storage */
    size_t elem_size;                          /**< Size in bytes of one element */
    size_t count;                              /**< Number of elements stored */
    size_t capacity;                           /**< Number of element slots allocated */
    zenit_heap_compare_fn_t compare;           /**< Comparator for heap ordering */
    zenit_allocator_t allocator;               /**< Memory allocator */
};

/* ─── Helper: swap two elements in the buffer by index ─── */
static void swap_elements(unsigned char *buf, size_t elem_size, size_t i, size_t j) {
    unsigned char *a = buf + i * elem_size;
    unsigned char *b = buf + j * elem_size;
    size_t n = elem_size;
    while (n--) {
        unsigned char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

/* ─── Sift-up: swim the element at `idx` up the heap ─── */
static void sift_up(zenit_heap_t *heap, size_t idx) {
    unsigned char *buf = heap->buffer;
    size_t es = heap->elem_size;

    while (idx > 0) {
        size_t parent = (idx - 1) / 2;

        if (heap->compare(buf + idx * es, buf + parent * es) <= 0) {
            break;
        }

        swap_elements(buf, es, idx, parent);
        idx = parent;
    }
}

/* ─── Sift-down: sink the element at `idx` down the heap ─── */
static void sift_down(zenit_heap_t *heap, size_t idx) {
    unsigned char *buf = heap->buffer;
    size_t es = heap->elem_size;
    size_t n = heap->count;

    while (1) {
        size_t largest = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;

        if (left < n && heap->compare(buf + left * es, buf + largest * es) > 0) {
            largest = left;
        }
        if (right < n && heap->compare(buf + right * es, buf + largest * es) > 0) {
            largest = right;
        }

        if (largest == idx) {
            break;
        }

        swap_elements(buf, es, idx, largest);
        idx = largest;
    }
}

/* ─── Helper: reallocate the internal buffer to a new capacity ─── */
static zenit_result_t realloc_buffer(zenit_heap_t *heap, size_t new_cap) {
    zenit_allocator_t a = heap->allocator;

    unsigned char *new_buf = zenit_allocator_realloc(a, heap->buffer, heap->capacity * heap->elem_size, new_cap * heap->elem_size);
    if (new_buf == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    heap->buffer = new_buf;
    heap->capacity = new_cap;
    return ZENIT_RESULT_OK;
}

/* ─── Helper: grow the buffer so at least min_capacity slots exist ─── */
static zenit_result_t grow(zenit_heap_t *heap, size_t min_capacity) {
    size_t new_cap = heap->capacity;

    while (new_cap < min_capacity) {
        new_cap = new_cap ? HEAP_GROWTH_FACTOR(new_cap) : HEAP_DEFAULT_CAPACITY;
    }

    return realloc_buffer(heap, new_cap);
}

/* ─── Public API ─── */

zenit_heap_t *zenit_heap_create_with_allocator(size_t elem_size, zenit_heap_compare_fn_t compare, zenit_allocator_t allocator) {
    return zenit_heap_create_with_capacity_and_allocator(elem_size, compare, HEAP_DEFAULT_CAPACITY, allocator);
}

zenit_heap_t *zenit_heap_create(size_t elem_size, zenit_heap_compare_fn_t compare) {
    return zenit_heap_create_with_allocator(elem_size, compare, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_heap_t *zenit_heap_create_with_capacity_and_allocator(
    size_t elem_size, zenit_heap_compare_fn_t compare, size_t capacity, zenit_allocator_t allocator
) {
    if (elem_size == 0 || compare == NULL || capacity == 0) {
        return NULL;
    }

    zenit_heap_t *heap = allocator.alloc_fn(sizeof(zenit_heap_t), allocator.ctx);
    if (heap == NULL) {
        return NULL;
    }

    heap->buffer = allocator.alloc_fn(capacity * elem_size, allocator.ctx);
    if (heap->buffer == NULL) {
        allocator.free_fn(heap, allocator.ctx);
        return NULL;
    }

    heap->elem_size = elem_size;
    heap->compare = compare;
    heap->count = 0;
    heap->capacity = capacity;
    heap->allocator = allocator;

    return heap;
}

zenit_heap_t *zenit_heap_create_with_capacity(
    size_t elem_size, zenit_heap_compare_fn_t compare, size_t capacity
) {
    return zenit_heap_create_with_capacity_and_allocator(elem_size, compare, capacity, ZENIT_ALLOCATOR_DEFAULT);
}

void zenit_heap_destroy(zenit_heap_t *heap) {
    if (heap == NULL) {
        return;
    }
    zenit_allocator_t a = heap->allocator;
    a.free_fn(heap->buffer, a.ctx);
    a.free_fn(heap, a.ctx);
}

zenit_result_t zenit_heap_push(zenit_heap_t *heap, const void *elem) {
    if (heap == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    if (heap->count == heap->capacity) {
        zenit_result_t r = grow(heap, heap->capacity + 1);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }

    unsigned char *dest = heap->buffer + heap->count * heap->elem_size;
    memcpy(dest, elem, heap->elem_size);
    heap->count++;

    sift_up(heap, heap->count - 1);

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_heap_pop(zenit_heap_t *heap, void *out_elem) {
    if (heap == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (heap->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    memcpy(out_elem, heap->buffer, heap->elem_size);

    heap->count--;
    if (heap->count > 0) {
        memcpy(heap->buffer,
               heap->buffer + heap->count * heap->elem_size,
               heap->elem_size);
        sift_down(heap, 0);
    }

    return ZENIT_RESULT_OK;
}

void *zenit_heap_peek(const zenit_heap_t *heap) {
    if (heap == NULL || heap->count == 0) {
        return NULL;
    }
    return heap->buffer;
}

size_t zenit_heap_count(const zenit_heap_t *heap) {
    if (heap == NULL) {
        return 0;
    }
    return heap->count;
}

size_t zenit_heap_capacity(const zenit_heap_t *heap) {
    if (heap == NULL) {
        return 0;
    }
    return heap->capacity;
}

int zenit_heap_empty(const zenit_heap_t *heap) {
    if (heap == NULL) {
        return 1;
    }
    return heap->count == 0 ? 1 : 0;
}

void zenit_heap_clear(zenit_heap_t *heap) {
    if (heap == NULL) {
        return;
    }
    heap->count = 0;
}

zenit_result_t zenit_heap_reserve(zenit_heap_t *heap, size_t capacity) {
    if (heap == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (capacity <= heap->capacity) {
        return ZENIT_RESULT_OK;
    }
    return realloc_buffer(heap, capacity);
}

zenit_result_t zenit_heap_build(zenit_heap_t *heap, const void *array, size_t count) {
    if (heap == NULL || array == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (count == 0) {
        heap->count = 0;
        return ZENIT_RESULT_OK;
    }
    /* Ensure capacity */
    if (count > heap->capacity) {
        zenit_result_t r = realloc_buffer(heap, count);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }
    /* Copy elements from the source array */
    memcpy(heap->buffer, array, count * heap->elem_size);
    heap->count = count;
    /* Floyd's heapify: sift-down from the last parent to the root */
    size_t idx = count / 2;
    while (idx > 0) {
        idx--;
        sift_down(heap, idx);
    }
    return ZENIT_RESULT_OK;
}
