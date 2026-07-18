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

#include <libzenit/bloom.h>
#include <stdio.h>
#include "test_malloc_fail.h"

/* ─── Test: bloom_create fails on handle malloc ─── */
static int test_create_handle_fail(void) {
    malloc_fail_countdown = 0;
    zenit_bloom_t *bf = zenit_bloom_create(10000, 0.01);
    if (bf != NULL) {
        fprintf(stderr, "FAIL: expected NULL on handle malloc failure\n");
        zenit_bloom_destroy(bf);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: bloom_create returns NULL on handle malloc failure\n");
    return 0;
}

/* ─── Test: bloom_create fails on bits array calloc ─── */
static int test_create_bits_fail(void) {
    /* countdown = 1: first malloc (handle) succeeds, second (bits) fails */
    malloc_fail_countdown = 1;
    zenit_bloom_t *bf = zenit_bloom_create(10000, 0.01);
    if (bf != NULL) {
        fprintf(stderr, "FAIL: expected NULL on bits array malloc failure\n");
        zenit_bloom_destroy(bf);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: bloom_create returns NULL on bits array malloc failure\n");
    return 0;
}

/* ─── Test: bloom_create_explicit fails on handle malloc ─── */
static int test_explicit_handle_fail(void) {
    malloc_fail_countdown = 0;
    zenit_bloom_t *bf = zenit_bloom_create_explicit(1024, 7);
    if (bf != NULL) {
        fprintf(stderr, "FAIL: expected NULL on explicit handle malloc failure\n");
        zenit_bloom_destroy(bf);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: bloom_create_explicit returns NULL on handle malloc failure\n");
    return 0;
}

/* ─── Test: bloom_create_explicit fails on bits array calloc ─── */
static int test_explicit_bits_fail(void) {
    malloc_fail_countdown = 1;
    zenit_bloom_t *bf = zenit_bloom_create_explicit(1024, 7);
    if (bf != NULL) {
        fprintf(stderr, "FAIL: expected NULL on explicit bits array malloc failure\n");
        zenit_bloom_destroy(bf);
        malloc_fail_countdown = -1;
        return 1;
    }
    malloc_fail_countdown = -1;
    printf("PASS: bloom_create_explicit returns NULL on bits array malloc failure\n");
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_handle_fail();
    ret |= test_create_bits_fail();
    ret |= test_explicit_handle_fail();
    ret |= test_explicit_bits_fail();
    return ret;
}
