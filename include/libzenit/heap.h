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

#ifndef LIBZENIT_HEAP_H
#define LIBZENIT_HEAP_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic binary heap (priority queue).
 *
 * Elements are stored in a contiguous array and arranged via a user-provided
 * comparison function (signed, same contract as qsort).  The comparator
 * determines the ordering:
 *   - return > 0  → a goes before b (max-heap behaviour)
 *   - return < 0  → b goes before a (min-heap behaviour)
 *   - return 0    → equal priority
 *
 * Uses 1.5x exponential growth on demand.
 */
typedef struct zenit_heap_t zenit_heap_t;

/**
 * @brief Comparison function for heap ordering.
 *
 * Must return negative, zero, or positive depending on whether @p a is
 * considered to have lower, equal, or higher priority than @p b.
 *
 * @param a Pointer to the first element.
 * @param b Pointer to the second element.
 * @return > 0 if a has higher priority than b, < 0 if lower, 0 if equal.
 */
typedef int (*zenit_heap_compare_fn_t)(const void *a, const void *b);

/**
 * @brief Create an empty heap with default initial capacity (8).
 *
 * @p elem_size must be > 0.  @p compare must not be NULL.
 *
 * @param elem_size Size in bytes of each element.
 * @param compare   Comparator function (must not be NULL).
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_heap_t *zenit_heap_create(size_t elem_size, zenit_heap_compare_fn_t compare);

/**
 * @brief Create an empty heap with a specific initial capacity.
 *
 * @param elem_size Size in bytes of each element.
 * @param compare   Comparator function (must not be NULL).
 * @param capacity  Initial number of element slots to allocate (> 0).
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_heap_t *zenit_heap_create_with_capacity(size_t elem_size, zenit_heap_compare_fn_t compare, size_t capacity);

/**
 * @brief Create an empty heap with a custom allocator.
 *
 * Default initial capacity (8).
 *
 * @param elem_size Size in bytes of each element.
 * @param compare   Comparator function (must not be NULL).
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_heap_t *zenit_heap_create_with_allocator(size_t elem_size, zenit_heap_compare_fn_t compare, zenit_allocator_t allocator);

/**
 * @brief Create an empty heap with a specific initial capacity and a custom allocator.
 *
 * @param elem_size Size in bytes of each element.
 * @param compare   Comparator function (must not be NULL).
 * @param capacity  Initial number of element slots to allocate (> 0).
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameters or allocation failure.
 */
zenit_heap_t *zenit_heap_create_with_capacity_and_allocator(size_t elem_size, zenit_heap_compare_fn_t compare, size_t capacity, zenit_allocator_t allocator);

/**
 * @brief Destroy a heap and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param heap Handle returned by zenit_heap_create(), or NULL.
 */
void zenit_heap_destroy(zenit_heap_t *heap);

/**
 * @brief Insert an element into the heap (O(log n)).
 *
 * The element is appended and sifted up to restore the heap property.
 *
 * @param heap Heap handle.
 * @param elem Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p heap or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_heap_push(zenit_heap_t *heap, const void *elem);

/**
 * @brief Remove and retrieve the root element (O(log n)).
 *
 * The root (highest priority element) is replaced with the last element
 * and sifted down to restore the heap property.
 *
 * @param heap     Heap handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p heap or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the heap is empty.
 */
zenit_result_t zenit_heap_pop(zenit_heap_t *heap, void *out_elem);

/**
 * @brief Get a pointer to the root element without removing it (O(1)).
 *
 * @param heap Heap handle.
 * @return Pointer to the root element, or NULL if empty or @p heap is NULL.
 */
void *zenit_heap_peek(const zenit_heap_t *heap);

/**
 * @brief Return the number of elements in the heap.
 *
 * @param heap Heap handle.
 * @return Element count (0 if @p heap is NULL).
 */
size_t zenit_heap_count(const zenit_heap_t *heap);

/**
 * @brief Return the current capacity.
 *
 * @param heap Heap handle.
 * @return Capacity (0 if @p heap is NULL).
 */
size_t zenit_heap_capacity(const zenit_heap_t *heap);

/**
 * @brief Check whether the heap is empty.
 *
 * @param heap Heap handle.
 * @return 1 if empty or @p heap is NULL, 0 otherwise.
 */
int zenit_heap_empty(const zenit_heap_t *heap);

/**
 * @brief Remove all elements without freeing the internal buffer.
 *
 * After this call count is 0 but capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param heap Heap handle, or NULL.
 */
void zenit_heap_clear(zenit_heap_t *heap);

/**
 * @brief Reserve capacity so that at least @p capacity elements can be
 *        stored without reallocation.
 *
 * If @p capacity is less than or equal to the current capacity, this is
 * a no-op.
 *
 * @param heap     Heap handle.
 * @param capacity Minimum desired capacity.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p heap is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_heap_reserve(zenit_heap_t *heap, size_t capacity);

#endif
