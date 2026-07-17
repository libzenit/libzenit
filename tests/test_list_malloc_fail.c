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

#include <libzenit/list.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_malloc_fail.h"

/* ─── Test: list_create fails on malloc ─── */
static int test_create_fail(void) {
    printf("  create malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = 0;
    zenit_list_t *list = zenit_list_create(sizeof(int));
    if (list != NULL) {
        fprintf(stderr, "FAIL: expected NULL on malloc failure\n");
        zenit_list_destroy(list);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS\n");
    return 0;
}

/* ─── Test: push_front fails on node alloc ─── */
static int test_push_front_fail(void) {
    printf("  push_front node alloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_list_t *list = zenit_list_create(sizeof(int));
    if (list == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int v = 42;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_list_push_front(list, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_list_destroy(list);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_list_destroy(list);
    printf("PASS\n");
    return 0;
}

/* ─── Test: push_back fails on node alloc ─── */
static int test_push_back_fail(void) {
    printf("  push_back node alloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_list_t *list = zenit_list_create(sizeof(int));
    if (list == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int v = 42;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_list_push_back(list, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_list_destroy(list);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_list_destroy(list);
    printf("PASS\n");
    return 0;
}

/* ─── Test: insert at front fails on node alloc ─── */
static int test_insert_front_fail(void) {
    printf("  insert at front node alloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_list_t *list = zenit_list_create(sizeof(int));
    if (list == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int v = 1;
    zenit_list_push_back(list, &v);

    v = 99;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_list_insert(list, 0, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_list_destroy(list);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_list_destroy(list);
    printf("PASS\n");
    return 0;
}

/* ─── Test: insert in middle fails on node alloc ─── */
static int test_insert_middle_fail(void) {
    printf("  insert in middle node alloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_list_t *list = zenit_list_create(sizeof(int));
    if (list == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int vals[] = {10, 20};
    zenit_list_push_back(list, &vals[0]);
    zenit_list_push_back(list, &vals[1]);

    int v = 99;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_list_insert(list, 1, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_list_destroy(list);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_list_destroy(list);
    printf("PASS\n");
    return 0;
}

int main(void) {
    printf("=== test_list_malloc_fail ===\n");

    int failed = 0;
    failed += test_create_fail();
    failed += test_push_front_fail();
    failed += test_push_back_fail();
    failed += test_insert_front_fail();
    failed += test_insert_middle_fail();

    if (failed != 0) {
        printf("\n%d test(s) FAILED\n", failed);
        return 1;
    }
    printf("\nAll malloc-failure tests PASSED\n");
    return 0;
}
