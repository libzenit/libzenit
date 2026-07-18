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

static int visit_noop(int vertex, void *ctx) {
    (void)vertex;
    (void)ctx;
    return 0;
}

static int test_bfs_visited_malloc_fail(void) {
    malloc_fail_countdown = -1;

    /* Create a graph with enough capacity to make calloc non-trivial */
    zenit_graph_t *g = zenit_graph_create(8, 0);
    if (g == NULL) {
        fprintf(stderr, "FAIL: bfs_visited_malloc_fail — create failed\n");
        return 1;
    }
    for (int i = 0; i < 4; i++) {
        if (zenit_graph_add_vertex(g).error != ZENIT_OK) {
            fprintf(stderr, "FAIL: bfs_visited_malloc_fail — add_vertex\n");
            zenit_graph_destroy(g);
            return 1;
        }
    }

    /* Fail the calloc for visited array (first allocation in BFS) */
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_graph_bfs(g, 0, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: bfs_visited_malloc_fail — expected ALLOC, got %d\n", r.error);
        zenit_graph_destroy(g);
        return 1;
    }

    malloc_fail_countdown = -1;
    zenit_graph_destroy(g);
    printf("PASS: bfs_visited_malloc_fail\n");
    return 0;
}

static int test_bfs_queue_malloc_fail(void) {
    malloc_fail_countdown = -1;

    zenit_graph_t *g = zenit_graph_create(8, 0);
    if (g == NULL) return 1;
    for (int i = 0; i < 4; i++) {
        if (zenit_graph_add_vertex(g).error != ZENIT_OK) {
            zenit_graph_destroy(g);
            return 1;
        }
    }

    /* calloc succeeds (countdown=0 → fail; set to 1 so calloc passes, then malloc fails) */
    /* calloc #0 = visited → set countdown to 1 (succeeds), then malloc #0 = queue → fails (0) */
    malloc_fail_countdown = 1;
    zenit_result_t r = zenit_graph_bfs(g, 0, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: bfs_queue_malloc_fail — expected ALLOC, got %d\n", r.error);
        zenit_graph_destroy(g);
        return 1;
    }

    malloc_fail_countdown = -1;
    zenit_graph_destroy(g);
    printf("PASS: bfs_queue_malloc_fail\n");
    return 0;
}

static int test_dfs_visited_malloc_fail(void) {
    malloc_fail_countdown = -1;

    zenit_graph_t *g = zenit_graph_create(8, 0);
    if (g == NULL) return 1;
    for (int i = 0; i < 4; i++) {
        if (zenit_graph_add_vertex(g).error != ZENIT_OK) {
            zenit_graph_destroy(g);
            return 1;
        }
    }

    /* Fail the calloc for visited array */
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_graph_dfs(g, 0, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: dfs_visited_malloc_fail — expected ALLOC, got %d\n", r.error);
        zenit_graph_destroy(g);
        return 1;
    }

    malloc_fail_countdown = -1;
    zenit_graph_destroy(g);
    printf("PASS: dfs_visited_malloc_fail\n");
    return 0;
}

static int test_dfs_stack_malloc_fail(void) {
    malloc_fail_countdown = -1;

    zenit_graph_t *g = zenit_graph_create(8, 0);
    if (g == NULL) return 1;
    for (int i = 0; i < 4; i++) {
        if (zenit_graph_add_vertex(g).error != ZENIT_OK) {
            zenit_graph_destroy(g);
            return 1;
        }
    }

    /* calloc succeeds (countdown=1), then malloc fails */
    malloc_fail_countdown = 1;
    zenit_result_t r = zenit_graph_dfs(g, 0, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: dfs_stack_malloc_fail — expected ALLOC, got %d\n", r.error);
        zenit_graph_destroy(g);
        return 1;
    }

    malloc_fail_countdown = -1;
    zenit_graph_destroy(g);
    printf("PASS: dfs_stack_malloc_fail\n");
    return 0;
}

static int test_get_neighbors_malloc_fail(void) {
    malloc_fail_countdown = -1;

    zenit_graph_t *g = zenit_graph_create(8, 0);
    if (g == NULL) return 1;
    for (int i = 0; i < 3; i++) {
        if (zenit_graph_add_vertex(g).error != ZENIT_OK) {
            zenit_graph_destroy(g);
            return 1;
        }
    }
    /* Add edges so there are neighbors to copy */
    if (zenit_graph_add_edge(g, 0, 1).error != ZENIT_OK ||
        zenit_graph_add_edge(g, 0, 2).error != ZENIT_OK) {
        zenit_graph_destroy(g);
        return 1;
    }

    int *neighbors = NULL;
    size_t count = 0;

    /* Fail the malloc inside get_neighbors */
    malloc_fail_countdown = 0;
    int ret = zenit_graph_get_neighbors(g, 0, &neighbors, &count);
    if (ret != -1) {
        fprintf(stderr, "FAIL: get_neighbors_malloc_fail — expected -1, got %d\n", ret);
        free(neighbors);
        zenit_graph_destroy(g);
        return 1;
    }

    malloc_fail_countdown = -1;
    zenit_graph_destroy(g);
    printf("PASS: get_neighbors_malloc_fail\n");
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

    /* Test BFS/DFS allocation failures */
    if (test_bfs_visited_malloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    if (test_bfs_queue_malloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    if (test_dfs_visited_malloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    if (test_dfs_stack_malloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    /* Test get_neighbors malloc failure */
    if (test_get_neighbors_malloc_fail() == 0) {
        passed++;
    } else {
        failed++;
    }

    malloc_fail_countdown = -1;

    printf("\n=== graph_malloc_fail ===\n");
    printf("%d passed, %d failed out of %d\n", passed, failed, passed + failed);

    return failed != 0 ? 1 : 0;
}
