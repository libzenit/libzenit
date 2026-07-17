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

#include <libzenit/deque.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_malloc_fail.h"

void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_realloc(ptr, size);
}

/* ─── Test: deque_create malloc fail ─── */
static int test_create_fail(void) {
    printf("  create malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = 0;
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    if (d != NULL) {
        fprintf(stderr, "FAIL: expected NULL on malloc failure\n");
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS\n");
    return 0;
}

/* ─── Test: deque_create_with_capacity buffer alloc fail ─── */
static int test_create_with_capacity_fail(void) {
    printf("  create_with_capacity buffer alloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    malloc_fail_countdown = 1;
    zenit_deque_t *d = zenit_deque_create_with_capacity(sizeof(int), 8);
    if (d != NULL) {
        fprintf(stderr, "FAIL: expected NULL on buffer malloc failure\n");
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS\n");
    return 0;
}

/* ─── Test: push_back grow fail ─── */
static int test_push_back_grow_fail(void) {
    printf("  push_back grow fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_deque_t *d = zenit_deque_create_with_capacity(sizeof(int), 2);
    if (d == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int v = 42;
    if (zenit_deque_push_back(d, &v).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push 1 failed\n");
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }
    if (zenit_deque_push_back(d, &v).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push 2 failed\n");
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }

    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_deque_push_back(d, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_deque_destroy(d);
    printf("PASS\n");
    return 0;
}

/* ─── Test: push_front grow fail ─── */
static int test_push_front_grow_fail(void) {
    printf("  push_front grow fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_deque_t *d = zenit_deque_create_with_capacity(sizeof(int), 2);
    if (d == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int v = 42;
    if (zenit_deque_push_front(d, &v).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push front 1 failed\n");
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }
    if (zenit_deque_push_front(d, &v).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push front 2 failed\n");
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }

    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_deque_push_front(d, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_deque_destroy(d);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_deque_destroy(d);
    printf("PASS\n");
    return 0;
}

int main(void) {
    printf("=== test_deque_malloc_fail ===\n");

    int failed = 0;
    failed += test_create_fail();
    failed += test_create_with_capacity_fail();
    failed += test_push_back_grow_fail();
    failed += test_push_front_grow_fail();

    if (failed != 0) {
        printf("\n%d test(s) FAILED\n", failed);
        return 1;
    }
    printf("\nAll malloc-failure tests PASSED\n");
    return 0;
}
