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

#include <libzenit/pool.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

struct zenit_pool_t {
    unsigned char *buffer;          /**< Contiguous block of object_size * capacity bytes */
    size_t *free_list;              /**< Array of free slot indices */
    size_t capacity;                /**< Total number of slots */
    size_t object_size;             /**< Size in bytes of each object */
    size_t free_count;              /**< Number of slots currently available */
    zenit_allocator_t allocator;    /**< Custom allocator used for this instance */
};

zenit_pool_t *zenit_pool_create_with_allocator(size_t object_size, size_t capacity, zenit_allocator_t allocator) {
    if (object_size == 0 || capacity == 0) {
        return NULL;
    }

    zenit_pool_t *pool = allocator.alloc_fn(sizeof(zenit_pool_t), allocator.ctx);
    if (pool == NULL) {
        return NULL;
    }

    pool->buffer = zenit_allocator_alloc_zero(allocator, capacity, object_size);
    if (pool->buffer == NULL) {
        allocator.free_fn(pool, allocator.ctx);
        return NULL;
    }

    pool->free_list = allocator.alloc_fn(capacity * sizeof(size_t), allocator.ctx);
    if (pool->free_list == NULL) {
        allocator.free_fn(pool->buffer, allocator.ctx);
        allocator.free_fn(pool, allocator.ctx);
        return NULL;
    }

    for (size_t i = 0; i < capacity; i++) {
        pool->free_list[i] = i;
    }

    pool->capacity = capacity;
    pool->object_size = object_size;
    pool->free_count = capacity;
    pool->allocator = allocator;

    return pool;
}

zenit_pool_t *zenit_pool_create(size_t object_size, size_t capacity) {
    return zenit_pool_create_with_allocator(object_size, capacity, ZENIT_ALLOCATOR_DEFAULT);
}

void zenit_pool_destroy(zenit_pool_t *pool) {
    if (pool == NULL) {
        return;
    }
    zenit_allocator_t a = pool->allocator;
    a.free_fn(pool->free_list, a.ctx);
    a.free_fn(pool->buffer, a.ctx);
    a.free_fn(pool, a.ctx);
}

void *zenit_pool_acquire(zenit_pool_t *pool) {
    if (pool == NULL || pool->free_count == 0) {
        return NULL;
    }

    pool->free_count--;
    size_t slot = pool->free_list[pool->free_count];
    return pool->buffer + slot * pool->object_size;
}

zenit_result_t zenit_pool_release(zenit_pool_t *pool, void *obj) {
    if (pool == NULL || obj == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    ptrdiff_t offset = (unsigned char *)obj - pool->buffer;
    if (offset < 0 || (size_t)offset >= pool->capacity * pool->object_size) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    size_t slot = (size_t)offset / pool->object_size;
    if ((size_t)offset % pool->object_size != 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    for (size_t i = 0; i < pool->free_count; i++) {
        if (pool->free_list[i] == slot) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_DOUBLE_FREE);
        }
    }

    pool->free_list[pool->free_count] = slot;
    pool->free_count++;
    return ZENIT_RESULT_OK;
}

size_t zenit_pool_count(const zenit_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }
    return pool->capacity - pool->free_count;
}

size_t zenit_pool_capacity(const zenit_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }
    return pool->capacity;
}

size_t zenit_pool_available(const zenit_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }
    return pool->free_count;
}

void zenit_pool_clear(zenit_pool_t *pool) {
    if (pool == NULL) {
        return;
    }
    for (size_t i = 0; i < pool->capacity; i++) {
        pool->free_list[i] = i;
    }
    pool->free_count = pool->capacity;
}
