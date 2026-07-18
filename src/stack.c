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

struct zenit_stack_t {
    zenit_vector_t *vector;
    size_t elem_size;
    zenit_allocator_t allocator;
};

zenit_stack_t *zenit_stack_create_with_allocator(size_t elem_size, zenit_allocator_t allocator) {
    if (elem_size == 0) {
        return NULL;
    }

    zenit_stack_t *stack = allocator.alloc_fn(sizeof(zenit_stack_t), allocator.ctx);
    if (stack == NULL) {
        return NULL;
    }

    stack->vector = zenit_vector_create_with_allocator(elem_size, allocator);
    if (stack->vector == NULL) {
        allocator.free_fn(stack, allocator.ctx);
        return NULL;
    }

    stack->elem_size = elem_size;
    stack->allocator = allocator;

    return stack;
}

zenit_stack_t *zenit_stack_create(size_t elem_size) {
    return zenit_stack_create_with_allocator(elem_size, ZENIT_ALLOCATOR_DEFAULT);
}

void zenit_stack_destroy(zenit_stack_t *stack) {
    if (stack == NULL) {
        return;
    }
    zenit_allocator_t a = stack->allocator;
    zenit_vector_destroy(stack->vector);
    a.free_fn(stack, a.ctx);
}

zenit_result_t zenit_stack_push(zenit_stack_t *stack, const void *elem) {
    if (stack == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_vector_push(stack->vector, elem);
}

zenit_result_t zenit_stack_pop(zenit_stack_t *stack, void *out_elem) {
    if (stack == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_vector_pop(stack->vector, out_elem);
}

void *zenit_stack_peek(const zenit_stack_t *stack) {
    if (stack == NULL) {
        return NULL;
    }
    size_t count = zenit_vector_count(stack->vector);
    if (count == 0) {
        return NULL;
    }
    return zenit_vector_get(stack->vector, count - 1);
}

size_t zenit_stack_count(const zenit_stack_t *stack) {
    if (stack == NULL) {
        return 0;
    }
    return zenit_vector_count(stack->vector);
}

int zenit_stack_empty(const zenit_stack_t *stack) {
    if (stack == NULL) {
        return 1;
    }
    return zenit_vector_empty(stack->vector);
}

void zenit_stack_clear(zenit_stack_t *stack) {
    if (stack == NULL) {
        return;
    }
    zenit_vector_clear(stack->vector);
}
