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

#include <libzenit/vector.h>
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

/* ─── Test: vector_create fails on handle malloc ─── */
static int test_create_handle_fail(void) {
    malloc_fail_countdown = 0;
    zenit_vector_t *v = zenit_vector_create(4);
    if (v != NULL) {
        fprintf(stderr, "FAIL: expected NULL on handle malloc failure\n");
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: vector_create returns NULL on handle malloc failure\n");
    return 0;
}

/* ─── Test: vector_create_with_capacity fails on handle malloc ─── */
static int test_create_cap_handle_fail(void) {
    malloc_fail_countdown = 0;
    zenit_vector_t *v = zenit_vector_create_with_capacity(4, 16);
    if (v != NULL) {
        fprintf(stderr, "FAIL: expected NULL on handle malloc failure\n");
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: vector_create_with_capacity returns NULL on handle malloc failure\n");
    return 0;
}

/* ─── Test: vector_create_with_capacity fails on buffer malloc ─── */
static int test_create_cap_buffer_fail(void) {
    malloc_fail_countdown = 1;
    zenit_vector_t *v = zenit_vector_create_with_capacity(4, 16);
    if (v != NULL) {
        fprintf(stderr, "FAIL: expected NULL on buffer malloc failure\n");
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: vector_create_with_capacity returns NULL on buffer malloc failure\n");
    return 0;
}

/* ─── Test: push fails on realloc (grow from empty) ─── */
static int test_push_grow_fail(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) {
        fprintf(stderr, "FAIL: create\n");
        return 1;
    }

    /* First push allocates (grow from 0 to default capacity) */
    int x = 42;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_vector_push(v, &x);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC on push grow fail, got %d\n", r.error);
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_vector_destroy(v);
    printf("PASS: push returns ALLOC on grow from empty realloc failure\n");
    return 0;
}

/* ─── Test: push fails on realloc (grow full buffer) ─── */
static int test_push_grow_full_fail(void) {
    zenit_vector_t *v = zenit_vector_create_with_capacity(4, 2);
    if (v == NULL) {
        fprintf(stderr, "FAIL: create_with_capacity\n");
        return 1;
    }

    int a = 1;
    int b = 2;
    if (zenit_vector_push(v, &a).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push a\n");
        zenit_vector_destroy(v);
        return 1;
    }
    if (zenit_vector_push(v, &b).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: push b\n");
        zenit_vector_destroy(v);
        return 1;
    }

    /* Third push triggers grow */
    int c = 3;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_vector_push(v, &c);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC on grow full fail, got %d\n", r.error);
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_vector_destroy(v);
    printf("PASS: push returns ALLOC on grow full realloc failure\n");
    return 0;
}

/* ─── Test: insert fails on realloc (grow full buffer) ─── */
static int test_insert_grow_fail(void) {
    zenit_vector_t *v = zenit_vector_create_with_capacity(4, 2);
    if (v == NULL) {
        fprintf(stderr, "FAIL: create_with_capacity\n");
        return 1;
    }

    int a = 1;
    int b = 2;
    if (zenit_vector_insert(v, 0, &a).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert a\n");
        zenit_vector_destroy(v);
        return 1;
    }
    if (zenit_vector_insert(v, 1, &b).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert b\n");
        zenit_vector_destroy(v);
        return 1;
    }

    /* Third insert triggers grow */
    int c = 3;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_vector_insert(v, 2, &c);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC on insert grow fail, got %d\n", r.error);
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_vector_destroy(v);
    printf("PASS: insert returns ALLOC on grow full realloc failure\n");
    return 0;
}

/* ─── Test: reserve fails ─── */
static int test_reserve_fail(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) {
        fprintf(stderr, "FAIL: create\n");
        return 1;
    }

    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_vector_reserve(v, 100);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ALLOC on reserve fail, got %d\n", r.error);
        zenit_vector_destroy(v);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    zenit_vector_destroy(v);
    printf("PASS: reserve returns ALLOC on realloc failure\n");
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_handle_fail();
    ret |= test_create_cap_handle_fail();
    ret |= test_create_cap_buffer_fail();
    ret |= test_push_grow_fail();
    ret |= test_push_grow_full_fail();
    ret |= test_insert_grow_fail();
    ret |= test_reserve_fail();
    return ret;
}
