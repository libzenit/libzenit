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

#include <libzenit/stack.h>
#include <libzenit/vector.h>
#include <libzenit/allocator.h>
#include <stdlib.h>

/**
 * @brief Internal stack state.
 *
 * Wraps a zenit_vector_t to provide LIFO semantics.  The allocator is stored
 * separately so we can free the handle in destroy (the vector stores its own
 * allocator internally).
 */
struct zenit_stack_t {
    zenit_vector_t *vector;    /**< Backing dynamic array */
    size_t elem_size;          /**< Size of each element in bytes */
    zenit_allocator_t allocator; /**< Allocator used for this handle */
};

/**
 * @brief Create a stack with a custom allocator.
 *
 * Allocates the handle and the backing vector, returning NULL if either
 * allocation fails or elem_size is 0.
 */
zenit_stack_t *zenit_stack_create_with_allocator(size_t elem_size, zenit_allocator_t allocator) {
    /* elem_size must be positive — zero makes no sense */
    if (elem_size == 0) {
        return NULL;
    }

    /* Allocate the handle using the custom allocator */
    zenit_stack_t *stack = allocator.alloc_fn(sizeof(zenit_stack_t), allocator.ctx);
    if (stack == NULL) {
        return NULL;
    }

    /* Create the backing vector with the same allocator */
    stack->vector = zenit_vector_create_with_allocator(elem_size, allocator);
    if (stack->vector == NULL) {
        /* Vector creation failed — free the handle before returning */
        allocator.free_fn(stack, allocator.ctx);
        return NULL;
    }

    /* Populate the rest of the struct fields */
    stack->elem_size = elem_size;
    stack->allocator = allocator;

    return stack;
}

/**
 * @brief Create a stack with the default allocator.
 */
zenit_stack_t *zenit_stack_create(size_t elem_size) {
    return zenit_stack_create_with_allocator(elem_size, ZENIT_ALLOCATOR_DEFAULT);
}

/**
 * @brief Destroy a stack and free all owned memory.
 *
 * Destroys the backing vector first (which frees its buffer and handle),
 * then frees the stack handle using the stored allocator.  NULL-safe.
 */
void zenit_stack_destroy(zenit_stack_t *stack) {
    if (stack == NULL) {
        return;
    }
    /* Read the allocator before freeing — we need it to free the handle */
    zenit_allocator_t a = stack->allocator;
    zenit_vector_destroy(stack->vector);
    a.free_fn(stack, a.ctx);
}

/**
 * @brief Push an element onto the top of the stack.
 *
 * Delegates to zenit_vector_push on the backing vector.
 */
zenit_result_t zenit_stack_push(zenit_stack_t *stack, const void *elem) {
    if (stack == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_vector_push(stack->vector, elem);
}

/**
 * @brief Pop the top element from the stack.
 *
 * Delegates to zenit_vector_pop on the backing vector.
 */
zenit_result_t zenit_stack_pop(zenit_stack_t *stack, void *out_elem) {
    if (stack == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_vector_pop(stack->vector, out_elem);
}

/**
 * @brief Return a pointer to the top element without removing it.
 *
 * Returns NULL if the stack is empty or the handle is NULL.
 */
void *zenit_stack_peek(const zenit_stack_t *stack) {
    if (stack == NULL) {
        return NULL;
    }
    size_t count = zenit_vector_count(stack->vector);
    if (count == 0) {
        return NULL;
    }
    /* The top element is the last element in the backing vector */
    return zenit_vector_get(stack->vector, count - 1);
}

/**
 * @brief Return the number of elements on the stack.
 */
size_t zenit_stack_count(const zenit_stack_t *stack) {
    if (stack == NULL) {
        return 0;
    }
    return zenit_vector_count(stack->vector);
}

/**
 * @brief Check whether the stack is empty.
 */
int zenit_stack_empty(const zenit_stack_t *stack) {
    if (stack == NULL) {
        return 1;
    }
    return zenit_vector_empty(stack->vector);
}

/**
 * @brief Remove all elements without destroying the stack.
 *
 * Delegates to zenit_vector_clear on the backing vector.
 */
void zenit_stack_clear(zenit_stack_t *stack) {
    if (stack == NULL) {
        return;
    }
    zenit_vector_clear(stack->vector);
}
