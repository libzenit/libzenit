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

#ifndef LIBZENIT_ALLOCATOR_H
#define LIBZENIT_ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Custom memory allocator interface.
 *
 * All LibZenit containers accept a custom allocator via `_with_allocator`
 * constructor variants.  Passing #ZENIT_ALLOCATOR_DEFAULT uses the standard
 * malloc / realloc / free.
 *
 * If @p realloc_fn is NULL, the library falls back to allocate-copy-free
 * for reallocation operations (slower but compatible with allocators that
 * do not support realloc, such as arena allocators).
 */
typedef struct {
    /** @brief Allocate @p size bytes.  Must return NULL on failure. */
    void *(*alloc_fn)(size_t size, void *ctx);
    /** @brief Reallocate @p ptr to @p size bytes, or NULL if unsupported. */
    void *(*realloc_fn)(void *ptr, size_t size, void *ctx);
    /** @brief Free a pointer previously returned by alloc_fn or realloc_fn. */
    void (*free_fn)(void *ptr, void *ctx);
    /** @brief Opaque context forwarded to every function call. */
    void *ctx;
} zenit_allocator_t;

/** @brief Default allocator using libc malloc / realloc / free. */
static inline void *zenit_default_alloc(size_t size, void *ctx) {
    (void)ctx;
    return malloc(size);
}

static inline void *zenit_default_realloc(void *ptr, size_t size, void *ctx) {
    (void)ctx;
    return realloc(ptr, size);
}

static inline void zenit_default_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

#define ZENIT_ALLOCATOR_DEFAULT ((zenit_allocator_t){ \
    .alloc_fn = zenit_default_alloc,                  \
    .realloc_fn = zenit_default_realloc,               \
    .free_fn = zenit_default_free,                     \
    .ctx = NULL                                        \
})

/**
 * @brief Allocate memory through an allocator.
 *
 * Returns zeroed memory (calloc equivalent).
 */
static inline void *zenit_allocator_alloc_zero(zenit_allocator_t a, size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    /* Check for overflow */
    if (nmemb > 0 && total / nmemb != size) {
        return NULL;
    }
    void *ptr = a.alloc_fn(total, a.ctx);
    if (ptr != NULL) {
        memset(ptr, 0, total);
    }
    return ptr;
}

/**
 * @brief Reallocate memory through an allocator, handling NULL realloc_fn.
 */
static inline void *zenit_allocator_realloc(zenit_allocator_t a, void *ptr, size_t old_size, size_t new_size) {
    if (a.realloc_fn != NULL) {
        return a.realloc_fn(ptr, new_size, a.ctx);
    }
    /* Fallback: alloc-copy-free */
    void *new_ptr = a.alloc_fn(new_size, a.ctx);
    if (new_ptr == NULL) {
        return NULL;
    }
    if (ptr != NULL) {
        size_t copy_size = old_size < new_size ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);
        a.free_fn(ptr, a.ctx);
    }
    return new_ptr;
}

#endif
