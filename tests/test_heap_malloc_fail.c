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

#include <libzenit/heap.h>
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

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (ia > ib) return 1;
    if (ia < ib) return -1;
    return 0;
}

/* ─── Test: heap_create malloc fail ─── */
static int test_create_fail(void) {
    printf("  create malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = 0;
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int);
    if (heap != NULL) {
        fprintf(stderr, "FAIL: expected NULL on malloc failure\n");
        zenit_heap_destroy(heap);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS\n");
    return 0;
}

/* ─── Test: heap_create_with_capacity buffer alloc fail ─── */
static int test_create_with_capacity_fail(void) {
    printf("  create_with_capacity buffer malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    /* First malloc (handle) succeeds, second (buffer) fails */
    malloc_fail_countdown = 1;
    zenit_heap_t *heap = zenit_heap_create_with_capacity(sizeof(int), cmp_int, 8);
    if (heap != NULL) {
        fprintf(stderr, "FAIL: expected NULL on buffer malloc failure\n");
        zenit_heap_destroy(heap);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS\n");
    return 0;
}

/* ─── Test: push grow fail ─── */
static int test_push_grow_fail(void) {
    printf("  push grow (realloc) fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_heap_t *heap = zenit_heap_create_with_capacity(sizeof(int), cmp_int, 2);
    if (heap == NULL) {
        fprintf(stderr, "FAIL: create failed unexpectedly\n");
        return 1;
    }

    int v = 42;
    if (zenit_heap_push(heap, &v).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push 1 failed\n");
        zenit_heap_destroy(heap);
        malloc_fail_countdown = -1;
        return 1;
    }
    if (zenit_heap_push(heap, &v).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push 2 failed\n");
        zenit_heap_destroy(heap);
        malloc_fail_countdown = -1;
        return 1;
    }

    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_heap_push(heap, &v);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC on grow, got %d\n", r.error);
        zenit_heap_destroy(heap);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_heap_destroy(heap);
    printf("PASS\n");
    return 0;
}

/* ─── Test: heap_build realloc fail ─── */
static int test_build_realloc_fail(void) {
    printf("  heap_build realloc fail ... ");
    fflush(stdout);

    zenit_heap_t *heap = zenit_heap_create_with_capacity(sizeof(int), cmp_int, 2);
    if (heap == NULL) {
        fprintf(stderr, "FAIL: create failed\n");
        return 1;
    }

    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_heap_build(heap, arr, 8);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error);
        zenit_heap_destroy(heap);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_heap_destroy(heap);
    printf("PASS\n");
    return 0;
}

int main(void) {
    printf("=== test_heap_malloc_fail ===\n");

    int failed = 0;
    failed += test_create_fail();
    failed += test_create_with_capacity_fail();
    failed += test_push_grow_fail();
    failed += test_build_realloc_fail();

    if (failed != 0) {
        printf("\n%d test(s) FAILED\n", failed);
        return 1;
    }
    printf("\nAll malloc-failure tests PASSED\n");
    return 0;
}
