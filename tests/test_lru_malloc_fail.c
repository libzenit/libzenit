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

#include <libzenit/lru.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Test that zenit_lru_create returns NULL when any internal allocation fails.
 *
 * The first allocation (the struct itself) may need to succeed; but any
 * subsequent allocation (keys, values, prev, next, hash_index, hash_table)
 * may be forced to fail by setting malloc_fail_countdown appropriately.
 *
 * We try every position up to 10 (the total number of allocator calls
 * during creation: 1 struct + 6 arrays = 7 calls), and verify either
 * success (countdown > number of calls) or NULL.
 */

int main(void) {
    int failures = 0;

    /* Try failing each allocation slot from 1 to 7 */
    for (int n = 1; n <= 7; n++) {
        malloc_fail_countdown = n;

        zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);

        /* If countdown exceeded the number of alloc calls, creation should succeed */
        if (n == 7) {
            /* All 7 allocations (1 struct + 6 arrays) happened to succeed */
            if (c != NULL) {
                zenit_lru_destroy(c);
            }
        } else {
            /* Some allocation failed — expect NULL */
            if (c != NULL) {
                fprintf(stderr, "FAIL: create should have returned NULL at countdown %d\n", n);
                zenit_lru_destroy(c);
                failures++;
            }
        }
    }

    /* Also test with evict variant */
    for (int n = 1; n <= 7; n++) {
        malloc_fail_countdown = n;

        zenit_lru_t *c = zenit_lru_create_with_evict(
            sizeof(int), sizeof(int), 4, NULL, NULL
        );

        if (n == 7) {
            if (c != NULL) zenit_lru_destroy(c);
        } else {
            if (c != NULL) {
                fprintf(stderr, "FAIL: create_with_evict should have returned NULL at countdown %d\n", n);
                zenit_lru_destroy(c);
                failures++;
            }
        }
    }

    /* Test with allocator variant */
    for (int n = 1; n <= 7; n++) {
        malloc_fail_countdown = n;

        zenit_lru_t *c = zenit_lru_create_with_allocator(
            sizeof(int), sizeof(int), 4, ZENIT_ALLOCATOR_DEFAULT
        );

        if (n == 7) {
            if (c != NULL) zenit_lru_destroy(c);
        } else {
            if (c != NULL) {
                fprintf(stderr, "FAIL: create_with_allocator should have returned NULL at countdown %d\n", n);
                zenit_lru_destroy(c);
                failures++;
            }
        }
    }

    /* Test with evict + allocator variant */
    for (int n = 1; n <= 7; n++) {
        malloc_fail_countdown = n;

        zenit_lru_t *c = zenit_lru_create_with_evict_and_allocator(
            sizeof(int), sizeof(int), 4, NULL, NULL, ZENIT_ALLOCATOR_DEFAULT
        );

        if (n == 7) {
            if (c != NULL) zenit_lru_destroy(c);
        } else {
            if (c != NULL) {
                fprintf(stderr, "FAIL: create_with_evict_and_allocator should have returned NULL at countdown %d\n", n);
                zenit_lru_destroy(c);
                failures++;
            }
        }
    }

    malloc_fail_countdown = -1;

    if (failures > 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }

    printf("PASS: lru_malloc_fail\n");
    return 0;
}
