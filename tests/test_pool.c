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
#include <libzenit/result.h>
#include "test_runner.h"

typedef struct {
    int id;
    double value;
    char name[16];
} test_object_t;

static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_pool_t *p = zenit_pool_create(sizeof(int), 10);
    ASSERT(p != NULL, "expected non-NULL");
    ASSERT(zenit_pool_capacity(p) == 10, "capacity == 10");
    ASSERT(zenit_pool_count(p) == 0, "count == 0");
    ASSERT(zenit_pool_available(p) == 10, "available == 10");
    zenit_pool_destroy(p);
    zenit_pool_destroy(NULL);
    PASS();
    return 0;
}

static int test_create_zero_object_size(void) {
    TEST("create zero object_size returns NULL");
    zenit_pool_t *p = zenit_pool_create(0, 10);
    ASSERT(p == NULL, "expected NULL");
    PASS();
    return 0;
}

static int test_create_zero_capacity(void) {
    TEST("create zero capacity returns NULL");
    zenit_pool_t *p = zenit_pool_create(4, 0);
    ASSERT(p == NULL, "expected NULL");
    PASS();
    return 0;
}

static int test_acquire_release(void) {
    TEST("acquire/release one object");
    zenit_pool_t *p = zenit_pool_create(sizeof(int), 5);
    ASSERT(p != NULL, "create");
    ASSERT(zenit_pool_available(p) == 5, "available == 5");
    ASSERT(zenit_pool_count(p) == 0, "count == 0");
    int *obj = (int *)zenit_pool_acquire(p);
    ASSERT(obj != NULL, "acquire non-NULL");
    ASSERT(zenit_pool_available(p) == 4, "available == 4");
    ASSERT(zenit_pool_count(p) == 1, "count == 1");
    *obj = 42;
    ASSERT(*obj == 42, "value preserved");
    ASSERT(zenit_pool_release(p, obj).error == ZENIT_OK, "release ok");
    ASSERT(zenit_pool_available(p) == 5, "available == 5 after release");
    ASSERT(zenit_pool_count(p) == 0, "count == 0 after release");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_acquire_until_empty(void) {
    TEST("acquire until empty");
    zenit_pool_t *p = zenit_pool_create(8, 3);
    ASSERT(p != NULL, "create");
    void *a = zenit_pool_acquire(p);
    void *b = zenit_pool_acquire(p);
    void *c = zenit_pool_acquire(p);
    ASSERT(a != NULL, "acquire 1");
    ASSERT(b != NULL, "acquire 2");
    ASSERT(c != NULL, "acquire 3");
    ASSERT(zenit_pool_available(p) == 0, "available == 0");
    ASSERT(zenit_pool_count(p) == 3, "count == 3");
    void *d = zenit_pool_acquire(p);
    ASSERT(d == NULL, "4th acquire should return NULL");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_double_release(void) {
    TEST("double release returns DOUBLE_FREE");
    zenit_pool_t *p = zenit_pool_create(sizeof(int), 5);
    ASSERT(p != NULL, "create");
    int *obj = (int *)zenit_pool_acquire(p);
    ASSERT(obj != NULL, "acquire");
    ASSERT(zenit_pool_release(p, obj).error == ZENIT_OK, "first release");
    ASSERT(zenit_pool_release(p, obj).error == ZENIT_ERROR_DOUBLE_FREE, "double release");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_release_non_pool_pointer(void) {
    TEST("release non-pool pointer returns PARAM");
    zenit_pool_t *p = zenit_pool_create(sizeof(int), 5);
    ASSERT(p != NULL, "create");
    int outside = 0;
    ASSERT(zenit_pool_release(p, &outside).error == ZENIT_ERROR_PARAM, "outside pointer");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_release_misaligned(void) {
    TEST("release misaligned pointer returns PARAM");
    zenit_pool_t *p = zenit_pool_create(8, 4);
    ASSERT(p != NULL, "create");
    unsigned char *buf = (unsigned char *)zenit_pool_acquire(p);
    ASSERT(buf != NULL, "acquire");
    ASSERT(zenit_pool_release(p, buf + 1).error == ZENIT_ERROR_PARAM, "misaligned");
    ASSERT(zenit_pool_release(p, buf).error == ZENIT_OK, "release actual");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_null_params(void) {
    TEST("NULL params safe");
    zenit_pool_destroy(NULL);
    ASSERT(zenit_pool_acquire(NULL) == NULL, "acquire(NULL) returns NULL");
    ASSERT(zenit_pool_release(NULL, NULL).error == ZENIT_ERROR_NULL, "release(NULL,NULL)");
    ASSERT(zenit_pool_release(NULL, (void *)1).error == ZENIT_ERROR_NULL, "release(NULL,ptr)");
    zenit_pool_t *p = zenit_pool_create(4, 5);
    ASSERT(p != NULL, "create for null tests");
    ASSERT(zenit_pool_release(p, NULL).error == ZENIT_ERROR_NULL, "release(p,NULL)");
    ASSERT(zenit_pool_count(NULL) == 0, "count(NULL) == 0");
    ASSERT(zenit_pool_capacity(NULL) == 0, "capacity(NULL) == 0");
    ASSERT(zenit_pool_available(NULL) == 0, "available(NULL) == 0");
    zenit_pool_clear(NULL);
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_clear(void) {
    TEST("clear resets pool");
    zenit_pool_t *p = zenit_pool_create(4, 5);
    ASSERT(p != NULL, "create");
    void *a = zenit_pool_acquire(p);
    void *b = zenit_pool_acquire(p);
    ASSERT(a != NULL && b != NULL, "acquire two");
    ASSERT(zenit_pool_count(p) == 2, "count == 2");
    ASSERT(zenit_pool_available(p) == 3, "available == 3");
    zenit_pool_clear(p);
    ASSERT(zenit_pool_count(p) == 0, "count == 0 after clear");
    ASSERT(zenit_pool_available(p) == 5, "available == 5 after clear");
    void *c = zenit_pool_acquire(p);
    ASSERT(c != NULL, "acquire after clear");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_allocator_variant(void) {
    TEST("create with allocator variant");
    zenit_pool_t *p = zenit_pool_create_with_allocator(8, 10, ZENIT_ALLOCATOR_DEFAULT);
    ASSERT(p != NULL, "expected non-NULL");
    ASSERT(zenit_pool_capacity(p) == 10, "capacity == 10");
    void *obj = zenit_pool_acquire(p);
    ASSERT(obj != NULL, "acquire ok");
    ASSERT(zenit_pool_release(p, obj).error == ZENIT_OK, "release ok");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_struct_objects(void) {
    TEST("struct objects in pool");
    zenit_pool_t *p = zenit_pool_create(sizeof(test_object_t), 4);
    ASSERT(p != NULL, "create");
    test_object_t *obj = (test_object_t *)zenit_pool_acquire(p);
    ASSERT(obj != NULL, "acquire");
    obj->id = 1;
    obj->value = 3.14;
    ASSERT(obj->id == 1, "id preserved");
    ASSERT(obj->value > 3.0 && obj->value < 4.0, "value preserved");
    ASSERT(zenit_pool_release(p, obj).error == ZENIT_OK, "release");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

static int test_count_available_consistency(void) {
    TEST("count/available consistency");
    zenit_pool_t *p = zenit_pool_create(4, 5);
    ASSERT(p != NULL, "create");
    ASSERT(zenit_pool_count(p) + zenit_pool_available(p) == zenit_pool_capacity(p),
           "count+avail == cap initially");
    void *a = zenit_pool_acquire(p);
    ASSERT(zenit_pool_count(p) + zenit_pool_available(p) == zenit_pool_capacity(p),
           "after acquire 1");
    void *b = zenit_pool_acquire(p);
    ASSERT(zenit_pool_count(p) + zenit_pool_available(p) == zenit_pool_capacity(p),
           "after acquire 2");
    zenit_pool_release(p, a);
    ASSERT(zenit_pool_count(p) + zenit_pool_available(p) == zenit_pool_capacity(p),
           "after release");
    zenit_pool_destroy(p);
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_create_destroy,
        test_create_zero_object_size,
        test_create_zero_capacity,
        test_acquire_release,
        test_acquire_until_empty,
        test_double_release,
        test_release_non_pool_pointer,
        test_release_misaligned,
        test_null_params,
        test_clear,
        test_allocator_variant,
        test_struct_objects,
        test_count_available_consistency,
    };
    const char *names[] = {
        "create/destroy",
        "create zero object_size returns NULL",
        "create zero capacity returns NULL",
        "acquire/release one object",
        "acquire until empty",
        "double release returns DOUBLE_FREE",
        "release non-pool pointer returns PARAM",
        "release misaligned pointer returns PARAM",
        "NULL params safe",
        "clear resets pool",
        "create with allocator variant",
        "struct objects in pool",
        "count/available consistency",
    };
    ZENIT_RUN_TESTS("pool", tests, names);
}
