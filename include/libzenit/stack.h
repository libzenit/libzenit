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

#ifndef LIBZENIT_STACK_H
#define LIBZENIT_STACK_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a generic LIFO stack.
 *
 * A thin wrapper around zenit_vector_t that exposes only stack operations:
 * push (append), pop (remove last), and peek (read last without removing).
 * All memory management is inherited from the underlying vector.
 */
typedef struct zenit_stack_t zenit_stack_t;

/**
 * @brief Create an empty stack with default initial capacity (8).
 *
 * @p elem_size must be > 0.
 *
 * @param elem_size Size in bytes of each element.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_stack_t *zenit_stack_create(size_t elem_size);

/**
 * @brief Create an empty stack with a custom allocator.
 *
 * @param elem_size Size in bytes of each element.
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on invalid parameter or allocation failure.
 */
zenit_stack_t *zenit_stack_create_with_allocator(size_t elem_size, zenit_allocator_t allocator);

/**
 * @brief Destroy a stack and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param stack Handle returned by zenit_stack_create(), or NULL.
 */
void zenit_stack_destroy(zenit_stack_t *stack);

/**
 * @brief Push an element onto the top of the stack.
 *
 * Grows the internal buffer if needed (1.5x factor).
 *
 * @param stack Stack handle.
 * @param elem  Pointer to the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p stack or @p elem is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_stack_push(zenit_stack_t *stack, const void *elem);

/**
 * @brief Pop the top element from the stack.
 *
 * Copies the top element into @p out_elem and removes it.
 *
 * @param stack    Stack handle.
 * @param out_elem Buffer to receive the element data (must not be NULL).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p stack or @p out_elem is NULL
 *         - ZENIT_ERROR_EMPTY if the stack is empty.
 */
zenit_result_t zenit_stack_pop(zenit_stack_t *stack, void *out_elem);

/**
 * @brief Get a pointer to the top element without removing it.
 *
 * @param stack Stack handle.
 * @return Pointer to the top element, or NULL if empty or @p stack is NULL.
 */
void *zenit_stack_peek(const zenit_stack_t *stack);

/**
 * @brief Return the number of elements on the stack.
 *
 * @param stack Stack handle.
 * @return Element count (0 if @p stack is NULL).
 */
size_t zenit_stack_count(const zenit_stack_t *stack);

/**
 * @brief Check whether the stack is empty.
 *
 * @param stack Stack handle.
 * @return 1 if empty or @p stack is NULL, 0 otherwise.
 */
int zenit_stack_empty(const zenit_stack_t *stack);

/**
 * @brief Remove all elements without freeing the internal buffer.
 *
 * After this call count is 0 but capacity is unchanged.
 * Passing NULL is safe and is a no-op.
 *
 * @param stack Stack handle, or NULL.
 */
void zenit_stack_clear(zenit_stack_t *stack);

#endif
