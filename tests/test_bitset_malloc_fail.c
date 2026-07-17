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

#include <libzenit/bitset.h>
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

int main(void) {
    /* create fails on first malloc (handle allocation fails) */
    malloc_fail_countdown = 0;
    zenit_bitset_t *bs = zenit_bitset_create(64);
    malloc_fail_countdown = -1;
    if (bs != NULL) {
        fprintf(stderr, "FAIL: expected NULL on create alloc fail\n");
        zenit_bitset_destroy(bs);
        return 1;
    }

    /* create fails on second malloc (handle succeeds, bits fails) */
    malloc_fail_countdown = 1;
    bs = zenit_bitset_create(64);
    malloc_fail_countdown = -1;
    if (bs != NULL) {
        fprintf(stderr, "FAIL: expected NULL when bits alloc fails\n");
        zenit_bitset_destroy(bs);
        return 1;
    }

    /* set triggers grow which fails */
    bs = zenit_bitset_create(0);
    if (bs == NULL) {
        fprintf(stderr, "FAIL: create should succeed\n");
        return 1;
    }
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_bitset_set(bs, 100);
    malloc_fail_countdown = -1;
    if (r.error == ZENIT_OK) {
        fprintf(stderr, "FAIL: set should fail on alloc\n");
        zenit_bitset_destroy(bs);
        return 1;
    }
    zenit_bitset_destroy(bs);

    /* clear triggers grow which fails */
    bs = zenit_bitset_create(0);
    if (bs == NULL) {
        fprintf(stderr, "FAIL: create should succeed\n");
        return 1;
    }
    malloc_fail_countdown = 0;
    r = zenit_bitset_clear(bs, 100);
    malloc_fail_countdown = -1;
    if (r.error == ZENIT_OK) {
        fprintf(stderr, "FAIL: clear should fail on alloc\n");
        zenit_bitset_destroy(bs);
        return 1;
    }
    zenit_bitset_destroy(bs);

    /* toggle triggers grow which fails */
    bs = zenit_bitset_create(0);
    if (bs == NULL) {
        fprintf(stderr, "FAIL: create should succeed\n");
        return 1;
    }
    malloc_fail_countdown = 0;
    r = zenit_bitset_toggle(bs, 100);
    malloc_fail_countdown = -1;
    if (r.error == ZENIT_OK) {
        fprintf(stderr, "FAIL: toggle should fail on alloc\n");
        zenit_bitset_destroy(bs);
        return 1;
    }
    zenit_bitset_destroy(bs);

    printf("PASS: bitset malloc fail tests\n");
    return 0;
}
