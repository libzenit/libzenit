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
#include <stdio.h>
#include "test_malloc_fail.h"

int main(void) {
    int failed = 0;

    /* create: handle alloc fails */
    {
        malloc_fail_countdown = 0;
        zenit_pool_t *p = zenit_pool_create(16, 10);
        malloc_fail_countdown = -1;
        if (p != NULL) {
            fprintf(stderr, "FAIL: pool create handle alloc fail should return NULL\n");
            zenit_pool_destroy(p);
            failed++;
        } else {
            printf("PASS: pool create handle alloc fail\n");
        }
    }

    /* create: buffer alloc fails (after handle succeeds) */
    {
        malloc_fail_countdown = 1;
        zenit_pool_t *p = zenit_pool_create(16, 10);
        malloc_fail_countdown = -1;
        if (p != NULL) {
            fprintf(stderr, "FAIL: pool create buffer alloc fail should return NULL\n");
            zenit_pool_destroy(p);
            failed++;
        } else {
            printf("PASS: pool create buffer alloc fail\n");
        }
    }

    /* create: free_list alloc fails (after handle and buffer succeed) */
    {
        malloc_fail_countdown = 2;
        zenit_pool_t *p = zenit_pool_create(16, 10);
        malloc_fail_countdown = -1;
        if (p != NULL) {
            fprintf(stderr, "FAIL: pool create free_list alloc fail should return NULL\n");
            zenit_pool_destroy(p);
            failed++;
        } else {
            printf("PASS: pool create free_list alloc fail\n");
        }
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
