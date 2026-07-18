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

/**
 * @brief Default malloc-based allocator function.
 *
 * Defined in src/allocator.c.
 *
 * @param size Number of bytes to allocate.
 * @param ctx  Opaque context (unused by the default allocator).
 * @return Pointer to the allocated memory, or NULL on failure.
 */
void *zenit_default_alloc(size_t size, void *ctx);

/**
 * @brief Default realloc-based allocator function.
 *
 * Defined in src/allocator.c.
 *
 * @param ptr  Pointer previously returned by zenit_default_alloc, or NULL.
 * @param size New size in bytes.
 * @param ctx  Opaque context (unused by the default allocator).
 * @return Pointer to the reallocated memory, or NULL on failure.
 */
void *zenit_default_realloc(void *ptr, size_t size, void *ctx);

/**
 * @brief Default free-based allocator function.
 *
 * Defined in src/allocator.c.
 *
 * @param ptr Pointer previously returned by zenit_default_alloc/realloc, or NULL.
 * @param ctx Opaque context (unused by the default allocator).
 */
void zenit_default_free(void *ptr, void *ctx);

#define ZENIT_ALLOCATOR_DEFAULT ((zenit_allocator_t){ \
    .alloc_fn = zenit_default_alloc,                  \
    .realloc_fn = zenit_default_realloc,               \
    .free_fn = zenit_default_free,                     \
    .ctx = NULL                                        \
})

/**
 * @brief Allocate memory through an allocator (zeroed).
 *
 * Returns zeroed memory (calloc equivalent).
 *
 * @param a     Allocator to use.
 * @param nmemb Number of elements.
 * @param size  Size in bytes of each element.
 * @return Pointer to the allocated zeroed memory, or NULL on failure.
 */
static inline void *zenit_allocator_alloc_zero(zenit_allocator_t a, size_t nmemb, size_t size) {
    void *ptr = a.alloc_fn(nmemb * size, a.ctx);
    if (ptr != NULL) {
        memset(ptr, 0, nmemb * size);
    }
    return ptr;
}

/**
 * @brief Reallocate memory through an allocator, handling NULL realloc_fn.
 *
 * Defined in src/allocator.c.
 *
 * @param a        Allocator to use.
 * @param ptr      Pointer to the previously allocated memory, or NULL.
 * @param old_size Previous allocation size in bytes (ignored if @p ptr is NULL).
 * @param new_size New desired size in bytes.
 * @return Pointer to the reallocated memory, or NULL on failure.
 */
void *zenit_allocator_realloc(zenit_allocator_t a, void *ptr, size_t old_size, size_t new_size);

#endif
