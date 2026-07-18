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

/**
 * @brief Internal queue state.
 *
 * Wraps a zenit_deque_t to provide FIFO semantics.  The allocator is stored
 * separately so we can free the handle in destroy (the deque stores its own
 * allocator internally).
 */
struct zenit_queue_t {
    zenit_deque_t *deque;    /**< Backing double-ended queue */
    size_t elem_size;        /**< Size of each element in bytes */
    zenit_allocator_t allocator; /**< Allocator used for this handle */
};

/**
 * @brief Create a queue with a custom allocator.
 *
 * Allocates the handle and the backing deque, returning NULL if either
 * allocation fails or elem_size is 0.
 */
zenit_queue_t *zenit_queue_create_with_allocator(size_t elem_size, zenit_allocator_t allocator) {
    /* elem_size must be positive — zero makes no sense */
    if (elem_size == 0) {
        return NULL;
    }

    /* Allocate the handle using the custom allocator */
    zenit_queue_t *queue = allocator.alloc_fn(sizeof(zenit_queue_t), allocator.ctx);
    if (queue == NULL) {
        return NULL;
    }

    /* Create the backing deque with the same allocator */
    queue->deque = zenit_deque_create_with_allocator(elem_size, allocator);
    if (queue->deque == NULL) {
        /* Deque creation failed — free the handle before returning */
        allocator.free_fn(queue, allocator.ctx);
        return NULL;
    }

    /* Populate the rest of the struct fields */
    queue->elem_size = elem_size;
    queue->allocator = allocator;

    return queue;
}

/**
 * @brief Create a queue with the default allocator.
 */
zenit_queue_t *zenit_queue_create(size_t elem_size) {
    return zenit_queue_create_with_allocator(elem_size, ZENIT_ALLOCATOR_DEFAULT);
}

/**
 * @brief Destroy a queue and free all owned memory.
 *
 * Destroys the backing deque first (which frees its buffer and handle),
 * then frees the queue handle using the stored allocator.  NULL-safe.
 */
void zenit_queue_destroy(zenit_queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    /* Read the allocator before freeing — we need it to free the handle */
    zenit_allocator_t a = queue->allocator;
    zenit_deque_destroy(queue->deque);
    a.free_fn(queue, a.ctx);
}

/**
 * @brief Enqueue an element at the back of the queue.
 *
 * Delegates to zenit_deque_push_back on the backing deque.
 */
zenit_result_t zenit_queue_enqueue(zenit_queue_t *queue, const void *elem) {
    if (queue == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_deque_push_back(queue->deque, elem);
}

/**
 * @brief Dequeue the front element from the queue.
 *
 * Delegates to zenit_deque_pop_front on the backing deque.
 */
zenit_result_t zenit_queue_dequeue(zenit_queue_t *queue, void *out_elem) {
    if (queue == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_deque_pop_front(queue->deque, out_elem);
}

/**
 * @brief Return a pointer to the front element without removing it.
 *
 * Returns NULL if the queue is empty or the handle is NULL.
 */
void *zenit_queue_peek(const zenit_queue_t *queue) {
    if (queue == NULL) {
        return NULL;
    }
    return zenit_deque_front(queue->deque);
}

/**
 * @brief Return the number of elements in the queue.
 */
size_t zenit_queue_count(const zenit_queue_t *queue) {
    if (queue == NULL) {
        return 0;
    }
    return zenit_deque_count(queue->deque);
}

/**
 * @brief Check whether the queue is empty.
 */
int zenit_queue_empty(const zenit_queue_t *queue) {
    if (queue == NULL) {
        return 1;
    }
    return zenit_deque_empty(queue->deque);
}

/**
 * @brief Remove all elements without destroying the queue.
 *
 * Delegates to zenit_deque_clear on the backing deque.
 */
void zenit_queue_clear(zenit_queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    zenit_deque_clear(queue->deque);
}
