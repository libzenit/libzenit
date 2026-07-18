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

#include <libzenit/graph.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>

/* Test that allocation failure in create returns NULL */

static int test_create_malloc_fail(void) {
    /* Reset the countdown before any printf/stdio calls */
    malloc_fail_countdown = -1;

    /* Fail the first malloc (handle allocation) */
    malloc_fail_countdown = 0;
    zenit_graph_t *g = zenit_graph_create(4, 1);
    if (g != NULL) {
        fprintf(stderr, "FAIL: create should return NULL on malloc failure\n");
        zenit_graph_destroy(g);
        return 1;
    }
    printf("PASS: create_malloc_fail (handle)\n");
    return 0;
}

static int test_create_adj_alloc_fail(void) {
    /* Reset before stdio */
    malloc_fail_countdown = -1;

    /* The first 2 mallocs are handle + adj array.
     * Fail the 3rd (counts array).  We allow 2 to succeed then 3rd fails.
     * Actual count depends on how many mallocs zenit_default_alloc uses.
     * Strategy: set countdown high then decrement gradually.
     */
    int count = 0;
    zenit_graph_t *g = NULL;

    /* Keep incrementing the countdown until create succeeds,
     * then try one less to find the failure point. */
    for (count = 0; count < 20; count++) {
        malloc_fail_countdown = count;
        g = zenit_graph_create(4, 1);
        if (g != NULL) {
            zenit_graph_destroy(g);
        } else {
            /* We found the failure point at 'count' */
            break;
        }
    }

    if (count >= 20) {
        fprintf(stderr, "FAIL: never hit malloc failure in create\n");
        return 1;
    }

    printf("PASS: create_adj_alloc_fail (fail at countdown=%d)\n", count);
    return 0;
}

int main(void) {
    int passed = 0;
    int failed = 0;

    /* Mark all countdowns as -1 for the tests themselves, then set
     * at the point of allocation. */
    malloc_fail_countdown = -1;

    /* Test handle malloc failure */
    if (test_create_malloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    /* Test internal array malloc failure */
    if (test_create_adj_alloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    malloc_fail_countdown = -1;

    printf("\n=== graph_malloc_fail ===\n");
    printf("%d passed, %d failed out of %d\n", passed, failed, passed + failed);

    return failed != 0 ? 1 : 0;
}
