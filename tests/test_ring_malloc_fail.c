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

#include <libzenit/ring.h>
#include <stdio.h>
#include "test_malloc_fail.h"

/* ─── Test: ring_create fails on handle malloc ─── */
static int test_create_handle_fail(void) {
    malloc_fail_countdown = 0;
    zenit_ring_t *r = zenit_ring_create(1024);
    if (r != NULL) {
        fprintf(stderr, "FAIL: expected NULL on handle malloc failure\n");
        zenit_ring_destroy(r);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: ring_create returns NULL on handle malloc failure\n");
    return 0;
}

/* ─── Test: ring_create fails on buffer calloc ─── */
static int test_create_buffer_fail(void) {
    malloc_fail_countdown = 1;
    zenit_ring_t *r = zenit_ring_create(1024);
    if (r != NULL) {
        fprintf(stderr, "FAIL: expected NULL on buffer calloc failure\n");
        zenit_ring_destroy(r);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: ring_create returns NULL on buffer calloc failure\n");
    return 0;
}

/* ─── Test: zenit_ring_reserve fails on malloc ─── */
static int test_reserve_alloc_fail(void) {
    zenit_ring_t *r = zenit_ring_create(16);
    if (r == NULL) {
        fprintf(stderr, "FAIL: create failed\n");
        return 1;
    }

    malloc_fail_countdown = 0;
    zenit_result_t res = zenit_ring_reserve(r, 32);
    malloc_fail_countdown = -1;

    if (res.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ZENIT_ERROR_ALLOC, got %d\n", res.error);
        zenit_ring_destroy(r);
        return 1;
    }
    /* Original ring should still be usable */
    if (zenit_ring_capacity(r) != 16) {
        fprintf(stderr, "FAIL: capacity should still be 16\n");
        zenit_ring_destroy(r);
        return 1;
    }
    zenit_ring_destroy(r);
    printf("PASS: ring_reserve returns ZENIT_ERROR_ALLOC on malloc failure\n");
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_handle_fail();
    ret |= test_create_buffer_fail();
    ret |= test_reserve_alloc_fail();
    return ret;
}
