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
#include "test_runner.h"

static int test_create_destroy(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    ASSERT(q != NULL, "create returned NULL");
    ASSERT(zenit_queue_count(q) == 0, "count not 0");
    ASSERT(zenit_queue_empty(q), "not empty");
    ASSERT(zenit_queue_peek(q) == NULL, "peek not NULL");
    zenit_queue_destroy(q);
    return 0;
}

static int test_create_invalid(void) {
    ASSERT(zenit_queue_create(0) == NULL, "zero elem_size not NULL");
    return 0;
}

static int test_destroy_null(void) {
    zenit_queue_destroy(NULL);
    return 0;
}

static int test_enqueue_dequeue(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    ASSERT(q != NULL, "create returned NULL");

    int v = 42;
    ASSERT(zenit_queue_enqueue(q, &v).error == ZENIT_OK, "enqueue failed");
    ASSERT(zenit_queue_count(q) == 1, "count not 1");
    ASSERT(!zenit_queue_empty(q), "empty after enqueue");

    int out;
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK, "dequeue failed");
    ASSERT(out == 42, "dequeued wrong value");
    ASSERT(zenit_queue_count(q) == 0, "count not 0 after dequeue");
    ASSERT(zenit_queue_empty(q), "not empty after dequeue");

    zenit_queue_destroy(q);
    return 0;
}

static int test_dequeue_empty(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    int out;
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_ERROR_EMPTY, "dequeue empty not EMPTY");
    zenit_queue_destroy(q);
    return 0;
}

static int test_peek(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    ASSERT(zenit_queue_peek(q) == NULL, "peek empty not NULL");

    int v = 77;
    zenit_queue_enqueue(q, &v);
    ASSERT(*(int *)zenit_queue_peek(q) == 77, "peek wrong value");
    ASSERT(zenit_queue_count(q) == 1, "count changed after peek");

    zenit_queue_destroy(q);
    return 0;
}

static int test_clear(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    int v;
    for (int i = 0; i < 10; i++) {
        v = i;
        zenit_queue_enqueue(q, &v);
    }
    ASSERT(zenit_queue_count(q) == 10, "count before clear");

    zenit_queue_clear(q);
    ASSERT(zenit_queue_count(q) == 0, "count after clear");
    ASSERT(zenit_queue_empty(q), "not empty after clear");

    zenit_queue_clear(NULL);
    zenit_queue_destroy(q);
    return 0;
}

static int test_null_params(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    int v = 0;
    int out;

    ASSERT(zenit_queue_enqueue(NULL, &v).error == ZENIT_ERROR_NULL, "enqueue NULL queue");
    ASSERT(zenit_queue_enqueue(q, NULL).error == ZENIT_ERROR_NULL, "enqueue NULL elem");
    ASSERT(zenit_queue_dequeue(NULL, &out).error == ZENIT_ERROR_NULL, "dequeue NULL queue");
    ASSERT(zenit_queue_dequeue(q, NULL).error == ZENIT_ERROR_NULL, "dequeue NULL out");
    ASSERT(zenit_queue_peek(NULL) == NULL, "peek NULL not NULL");
    ASSERT(zenit_queue_count(NULL) == 0, "count NULL not 0");
    ASSERT(zenit_queue_empty(NULL) == 1, "empty NULL not 1");

    zenit_queue_destroy(q);
    return 0;
}

static int test_many_elements(void) {
    int n = 1000;
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    ASSERT(q != NULL, "create returned NULL");

    for (int i = 0; i < n; i++) {
        ASSERT(zenit_queue_enqueue(q, &i).error == ZENIT_OK, "enqueue failed");
    }
    ASSERT(zenit_queue_count(q) == (size_t)n, "count mismatch");

    for (int i = 0; i < n; i++) {
        int out;
        ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK, "dequeue failed");
        ASSERT(out == i, "FIFO order broken");
    }
    ASSERT(zenit_queue_empty(q), "not empty after all dequeues");

    zenit_queue_destroy(q);
    return 0;
}

static int test_mixed_ordering(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(int));
    ASSERT(q != NULL, "create returned NULL");

    int v;
    v = 10; zenit_queue_enqueue(q, &v);
    v = 20; zenit_queue_enqueue(q, &v);

    int out;
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK && out == 10, "first dequeue");

    v = 30; zenit_queue_enqueue(q, &v);
    v = 40; zenit_queue_enqueue(q, &v);

    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK && out == 20, "second dequeue");
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK && out == 30, "third dequeue");
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK && out == 40, "fourth dequeue");

    ASSERT(zenit_queue_empty(q), "not empty after mixed ops");

    zenit_queue_destroy(q);
    return 0;
}

typedef struct {
    int id;
    double val;
} item_t;

static int test_struct_elements(void) {
    zenit_queue_t *q = zenit_queue_create(sizeof(item_t));
    ASSERT(q != NULL, "create returned NULL");

    item_t i1 = {1, 1.5};
    item_t i2 = {2, 2.5};
    ASSERT(zenit_queue_enqueue(q, &i1).error == ZENIT_OK, "enqueue i1");
    ASSERT(zenit_queue_enqueue(q, &i2).error == ZENIT_OK, "enqueue i2");

    item_t out;
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK, "dequeue i1");
    ASSERT(out.id == 1 && out.val == 1.5, "i1 values wrong");
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK, "dequeue i2");
    ASSERT(out.id == 2 && out.val == 2.5, "i2 values wrong");

    zenit_queue_destroy(q);
    return 0;
}

static int test_allocator_variant(void) {
    zenit_queue_t *q = zenit_queue_create_with_allocator(sizeof(int), ZENIT_ALLOCATOR_DEFAULT);
    ASSERT(q != NULL, "create_with_allocator returned NULL");
    int v = 7;
    ASSERT(zenit_queue_enqueue(q, &v).error == ZENIT_OK, "enqueue with allocator");
    int out;
    ASSERT(zenit_queue_dequeue(q, &out).error == ZENIT_OK, "dequeue with allocator");
    ASSERT(out == 7, "wrong value with allocator");
    zenit_queue_destroy(q);
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_create_destroy,
        test_create_invalid,
        test_destroy_null,
        test_enqueue_dequeue,
        test_dequeue_empty,
        test_peek,
        test_clear,
        test_null_params,
        test_many_elements,
        test_mixed_ordering,
        test_struct_elements,
        test_allocator_variant,
    };
    const char *names[] = {
        "create_destroy",
        "create_invalid",
        "destroy_null",
        "enqueue_dequeue",
        "dequeue_empty",
        "peek",
        "clear",
        "null_params",
        "many_elements",
        "mixed_ordering",
        "struct_elements",
        "allocator_variant",
    };
    ZENIT_RUN_TESTS("queue", tests, names);
}
