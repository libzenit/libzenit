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

#include <libzenit/queue.h>
#include <libzenit/deque.h>
#include <libzenit/allocator.h>
#include <stdlib.h>

struct zenit_queue_t {
    zenit_deque_t *deque;
    size_t elem_size;
    zenit_allocator_t allocator;
};

zenit_queue_t *zenit_queue_create_with_allocator(size_t elem_size, zenit_allocator_t allocator) {
    if (elem_size == 0) {
        return NULL;
    }

    zenit_queue_t *queue = allocator.alloc_fn(sizeof(zenit_queue_t), allocator.ctx);
    if (queue == NULL) {
        return NULL;
    }

    queue->deque = zenit_deque_create_with_allocator(elem_size, allocator);
    if (queue->deque == NULL) {
        allocator.free_fn(queue, allocator.ctx);
        return NULL;
    }

    queue->elem_size = elem_size;
    queue->allocator = allocator;

    return queue;
}

zenit_queue_t *zenit_queue_create(size_t elem_size) {
    return zenit_queue_create_with_allocator(elem_size, ZENIT_ALLOCATOR_DEFAULT);
}

void zenit_queue_destroy(zenit_queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    zenit_allocator_t a = queue->allocator;
    zenit_deque_destroy(queue->deque);
    a.free_fn(queue, a.ctx);
}

zenit_result_t zenit_queue_enqueue(zenit_queue_t *queue, const void *elem) {
    if (queue == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_deque_push_back(queue->deque, elem);
}

zenit_result_t zenit_queue_dequeue(zenit_queue_t *queue, void *out_elem) {
    if (queue == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_deque_pop_front(queue->deque, out_elem);
}

void *zenit_queue_peek(const zenit_queue_t *queue) {
    if (queue == NULL) {
        return NULL;
    }
    return zenit_deque_front(queue->deque);
}

size_t zenit_queue_count(const zenit_queue_t *queue) {
    if (queue == NULL) {
        return 0;
    }
    return zenit_deque_count(queue->deque);
}

int zenit_queue_empty(const zenit_queue_t *queue) {
    if (queue == NULL) {
        return 1;
    }
    return zenit_deque_empty(queue->deque);
}

void zenit_queue_clear(zenit_queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    zenit_deque_clear(queue->deque);
}
