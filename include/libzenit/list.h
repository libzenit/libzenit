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

#ifndef LIBZENIT_LIST_H
#define LIBZENIT_LIST_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic doubly linked list.
 *
 * Each element is stored in a heap-allocated node with prev/next pointers.
 * Operations at the ends (push/pop front/back) are O(1).  Indexed access
 * is O(n).  Elements are copied by memcpy — suitable for scalar types
 * and small structs.
 */
typedef struct zenit_list_t zenit_list_t;

/**
 * @brief Visitor callback for zenit_list_foreach().
 *
 * @param elem Pointer to the element data (elem_size bytes).
 * @param ctx  Opaque context pointer supplied by the caller.
 */
typedef void (*zenit_list_visit_fn_t)(const void *elem, void *ctx);

/**
 * @brief Create an empty linked list.
 *
 * @p elem_size must be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_list_t *zenit_list_create(size_t elem_size);

/**
 * @brief Create an empty linked list with a custom memory allocator.
 *
 * Same as zenit_list_create() but uses @p allocator for all memory operations.
 *
 * @param elem_size Size in bytes of each element.
 * @param allocator Custom allocator (use ZENIT_ALLOCATOR_DEFAULT for libc).
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_list_t *zenit_list_create_with_allocator(size_t elem_size, zenit_allocator_t allocator);

/**
 * @brief Destroy a linked list and free all nodes.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param list Handle returned by zenit_list_create(), or NULL.
 */
void zenit_list_destroy(zenit_list_t *list);

/**
 * @brief Prepend an element to the list (O(1)).
 *
 * @param list List handle.
 * @param elem Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if node allocation fails.
 */
zenit_result_t zenit_list_push_front(zenit_list_t *list, const void *elem);

/**
 * @brief Append an element to the list (O(1)).
 *
 * @param list List handle.
 * @param elem Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if node allocation fails.
 */
zenit_result_t zenit_list_push_back(zenit_list_t *list, const void *elem);

/**
 * @brief Remove and retrieve the first element (O(1)).
 *
 * @param list    List handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the list is empty.
 */
zenit_result_t zenit_list_pop_front(zenit_list_t *list, void *out_elem);

/**
 * @brief Remove and retrieve the last element (O(1)).
 *
 * @param list    List handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the list is empty.
 */
zenit_result_t zenit_list_pop_back(zenit_list_t *list, void *out_elem);

/**
 * @brief Insert an element at a given index (O(n)).
 *
 * Inserting at index 0 is equivalent to push_front.
 * Inserting at index == count is equivalent to push_back.
 *
 * @param list  List handle.
 * @param index Position to insert at (0 … count, inclusive).
 * @param elem  Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p elem is NULL
 *         - ZENIT_ERROR_PARAM if @p index > count
 *         - ZENIT_ERROR_ALLOC if node allocation fails.
 */
zenit_result_t zenit_list_insert(zenit_list_t *list, size_t index, const void *elem);

/**
 * @brief Remove and retrieve an element at a given index (O(n)).
 *
 * @param list    List handle.
 * @param index   Position to remove (0 … count - 1).
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p out_elem is NULL
 *         - ZENIT_ERROR_PARAM if @p index >= count.
 */
zenit_result_t zenit_list_remove(zenit_list_t *list, size_t index, void *out_elem);

/**
 * @brief Get a pointer to the element at a given index (O(n)).
 *
 * The pointer becomes invalid after any mutation of the list.
 *
 * @param list  List handle.
 * @param index Element index (0 … count - 1).
 * @return Pointer to the element data, or NULL if @p list is NULL or
 *         @p index is out of bounds.
 */
void *zenit_list_get(const zenit_list_t *list, size_t index);

/**
 * @brief Overwrite the element at a given index with new data (O(n)).
 *
 * @param list  List handle.
 * @param index Element index (0 … count - 1).
 * @param elem  Pointer to the new element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p list or @p elem is NULL
 *         - ZENIT_ERROR_PARAM if @p index >= count.
 */
zenit_result_t zenit_list_set(const zenit_list_t *list, size_t index, const void *elem);

/**
 * @brief Return the number of elements in the list.
 *
 * @param list List handle.
 * @return Element count (0 if @p list is NULL).
 */
size_t zenit_list_count(const zenit_list_t *list);

/**
 * @brief Check whether the list is empty.
 *
 * @param list List handle.
 * @return 1 if empty or @p list is NULL, 0 otherwise.
 */
int zenit_list_empty(const zenit_list_t *list);

/**
 * @brief Remove all elements without destroying the list.
 *
 * All nodes are freed.  Passing NULL is safe and is a no-op.
 *
 * @param list List handle, or NULL.
 */
void zenit_list_clear(zenit_list_t *list);

/**
 * @brief Iterate over all elements in order.
 *
 * @param list  List handle.
 * @param visit Visitor callback (must not be NULL).
 * @param ctx   Opaque context forwarded to @p visit on every call.
 */
void zenit_list_foreach(const zenit_list_t *list, zenit_list_visit_fn_t visit, void *ctx);

/**
 * @brief Get a pointer to the first element (O(1)).
 *
 * @param list List handle.
 * @return Pointer to the first element, or NULL if empty or @p list is NULL.
 */
void *zenit_list_front(const zenit_list_t *list);

/**
 * @brief Get a pointer to the last element (O(1)).
 *
 * @param list List handle.
 * @return Pointer to the last element, or NULL if empty or @p list is NULL.
 */
void *zenit_list_back(const zenit_list_t *list);

/**
 * @brief Create an iterator for the list.
 *
 * The iterator must be advanced with zenit_list_iter_next().
 *
 * @param list List handle.
 * @return An iterator (check is_valid).
 */
zenit_iter_t zenit_list_iter(const zenit_list_t *list);

/**
 * @brief Advance a list iterator to the next element.
 *
 * @param iter Iterator created by zenit_list_iter().
 * @return Pointer to the element data, or NULL if iteration is complete.
 */
void *zenit_list_iter_next(zenit_iter_t *iter);

/**
 * @brief Iterate over all elements in reverse order (tail to head).
 *
 * @param list  List handle.
 * @param visit Visitor callback (must not be NULL).
 * @param ctx   Opaque context forwarded to @p visit on every call.
 */
void zenit_list_reverse_foreach(const zenit_list_t *list, zenit_list_visit_fn_t visit, void *ctx);

#endif
