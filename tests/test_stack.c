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
#include "test_runner.h"

static int test_create_destroy(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    ASSERT(s != NULL, "create returned NULL");
    ASSERT(zenit_stack_count(s) == 0, "count not 0");
    ASSERT(zenit_stack_empty(s), "not empty");
    ASSERT(zenit_stack_peek(s) == NULL, "peek not NULL");
    zenit_stack_destroy(s);
    return 0;
}

static int test_create_invalid(void) {
    ASSERT(zenit_stack_create(0) == NULL, "zero elem_size not NULL");
    return 0;
}

static int test_destroy_null(void) {
    zenit_stack_destroy(NULL);
    return 0;
}

static int test_push_pop(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    ASSERT(s != NULL, "create returned NULL");

    int v = 42;
    ASSERT(zenit_stack_push(s, &v).error == ZENIT_OK, "push failed");
    ASSERT(zenit_stack_count(s) == 1, "count not 1");
    ASSERT(!zenit_stack_empty(s), "empty after push");

    int out;
    ASSERT(zenit_stack_pop(s, &out).error == ZENIT_OK, "pop failed");
    ASSERT(out == 42, "popped wrong value");
    ASSERT(zenit_stack_count(s) == 0, "count not 0 after pop");
    ASSERT(zenit_stack_empty(s), "not empty after pop");

    zenit_stack_destroy(s);
    return 0;
}

static int test_pop_empty(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    int out;
    ASSERT(zenit_stack_pop(s, &out).error == ZENIT_ERROR_EMPTY, "pop empty not EMPTY");
    zenit_stack_destroy(s);
    return 0;
}

static int test_peek(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    ASSERT(zenit_stack_peek(s) == NULL, "peek empty not NULL");

    int v = 99;
    zenit_stack_push(s, &v);
    ASSERT(*(int *)zenit_stack_peek(s) == 99, "peek wrong value");
    ASSERT(zenit_stack_count(s) == 1, "count changed after peek");

    zenit_stack_destroy(s);
    return 0;
}

static int test_clear(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    int v;
    for (int i = 0; i < 10; i++) {
        v = i;
        zenit_stack_push(s, &v);
    }
    ASSERT(zenit_stack_count(s) == 10, "count before clear");

    zenit_stack_clear(s);
    ASSERT(zenit_stack_count(s) == 0, "count after clear");
    ASSERT(zenit_stack_empty(s), "not empty after clear");

    zenit_stack_clear(NULL);
    zenit_stack_destroy(s);
    return 0;
}

static int test_null_params(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    int v = 0;
    int out;

    ASSERT(zenit_stack_push(NULL, &v).error == ZENIT_ERROR_NULL, "push NULL stack");
    ASSERT(zenit_stack_push(s, NULL).error == ZENIT_ERROR_NULL, "push NULL elem");
    ASSERT(zenit_stack_pop(NULL, &out).error == ZENIT_ERROR_NULL, "pop NULL stack");
    ASSERT(zenit_stack_pop(s, NULL).error == ZENIT_ERROR_NULL, "pop NULL out");
    ASSERT(zenit_stack_peek(NULL) == NULL, "peek NULL not NULL");
    ASSERT(zenit_stack_count(NULL) == 0, "count NULL not 0");
    ASSERT(zenit_stack_empty(NULL) == 1, "empty NULL not 1");

    zenit_stack_destroy(s);
    return 0;
}

static int test_many_elements(void) {
    int n = 1000;
    zenit_stack_t *s = zenit_stack_create(sizeof(int));
    ASSERT(s != NULL, "create returned NULL");

    for (int i = 0; i < n; i++) {
        ASSERT(zenit_stack_push(s, &i).error == ZENIT_OK, "push failed");
    }
    ASSERT(zenit_stack_count(s) == (size_t)n, "count mismatch");

    for (int i = n - 1; i >= 0; i--) {
        int out;
        ASSERT(zenit_stack_pop(s, &out).error == ZENIT_OK, "pop failed");
        ASSERT(out == i, "LIFO order broken");
    }
    ASSERT(zenit_stack_empty(s), "not empty after all pops");

    zenit_stack_destroy(s);
    return 0;
}

typedef struct {
    int x;
    double y;
} point_t;

static int test_struct_elements(void) {
    zenit_stack_t *s = zenit_stack_create(sizeof(point_t));
    ASSERT(s != NULL, "create returned NULL");

    point_t p1 = {1, 1.5};
    point_t p2 = {2, 2.5};
    ASSERT(zenit_stack_push(s, &p1).error == ZENIT_OK, "push p1");
    ASSERT(zenit_stack_push(s, &p2).error == ZENIT_OK, "push p2");

    point_t out;
    ASSERT(zenit_stack_pop(s, &out).error == ZENIT_OK, "pop p2");
    ASSERT(out.x == 2 && out.y == 2.5, "p2 values wrong");
    ASSERT(zenit_stack_pop(s, &out).error == ZENIT_OK, "pop p1");
    ASSERT(out.x == 1 && out.y == 1.5, "p1 values wrong");

    zenit_stack_destroy(s);
    return 0;
}

static int test_allocator_variant(void) {
    zenit_stack_t *s = zenit_stack_create_with_allocator(sizeof(int), ZENIT_ALLOCATOR_DEFAULT);
    ASSERT(s != NULL, "create_with_allocator returned NULL");
    int v = 7;
    ASSERT(zenit_stack_push(s, &v).error == ZENIT_OK, "push with allocator");
    int out;
    ASSERT(zenit_stack_pop(s, &out).error == ZENIT_OK, "pop with allocator");
    ASSERT(out == 7, "wrong value with allocator");
    zenit_stack_destroy(s);
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_create_destroy,
        test_create_invalid,
        test_destroy_null,
        test_push_pop,
        test_pop_empty,
        test_peek,
        test_clear,
        test_null_params,
        test_many_elements,
        test_struct_elements,
        test_allocator_variant,
    };
    const char *names[] = {
        "create_destroy",
        "create_invalid",
        "destroy_null",
        "push_pop",
        "pop_empty",
        "peek",
        "clear",
        "null_params",
        "many_elements",
        "struct_elements",
        "allocator_variant",
    };
    ZENIT_RUN_TESTS("stack", tests, names);
}
