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
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>

/* ─── Custom allocator for failure injection ─── */

typedef struct {
    int alloc_calls;
    int fail_alloc_at;   /* -1 = never fail */
    int realloc_calls;
    int fail_realloc_at; /* -1 = never fail */
} test_alloc_ctx_t;

static void* test_alloc(size_t size, void *ctx) {
    test_alloc_ctx_t *tc = (test_alloc_ctx_t *)ctx;
    if (tc->fail_alloc_at >= 0 && tc->alloc_calls == tc->fail_alloc_at) {
        return NULL;
    }
    tc->alloc_calls++;
    return malloc(size);
}

static void* test_realloc(void *ptr, size_t size, void *ctx) {
    test_alloc_ctx_t *tc = (test_alloc_ctx_t *)ctx;
    if (tc->fail_realloc_at >= 0 && tc->realloc_calls == tc->fail_realloc_at) {
        return NULL;
    }
    tc->realloc_calls++;
    return realloc(ptr, size);
}

static void test_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

/* ─── create / destroy ─── */

static int test_create_directed(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create directed failed");
    ASSERT(zenit_graph_is_directed(g), "is_directed should be 1");
    ASSERT(zenit_graph_vertex_count(g) == 0, "vertex count should be 0");
    ASSERT(zenit_graph_edge_count(g) == 0, "edge count should be 0");
    zenit_graph_destroy(g);
    return 0;
}

static int test_create_undirected(void) {
    zenit_graph_t *g = zenit_graph_create(4, 0);
    ASSERT(g != NULL, "create undirected failed");
    ASSERT(!zenit_graph_is_directed(g), "is_directed should be 0");
    zenit_graph_destroy(g);
    return 0;
}

static int test_create_zero_capacity(void) {
    /* Zero initial capacity should use default (8) and succeed */
    zenit_graph_t *g = zenit_graph_create(0, 1);
    ASSERT(g != NULL, "create zero capacity failed");
    zenit_graph_destroy(g);
    return 0;
}

static int test_destroy_null(void) {
    /* Should not crash */
    zenit_graph_destroy(NULL);
    return 0;
}

/* ─── add_vertex ─── */

static int test_add_vertex_basic(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    for (int i = 0; i < 5; i++) {
        zenit_result_t r = zenit_graph_add_vertex(g);
        ASSERT(r.error == ZENIT_OK, "add_vertex failed");
    }

    ASSERT(zenit_graph_vertex_count(g) == 5, "vertex count should be 5");
    zenit_graph_destroy(g);
    return 0;
}

static int test_add_vertex_null(void) {
    zenit_result_t r = zenit_graph_add_vertex(NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "add_vertex NULL should return NULL error");
    return 0;
}

/* ─── add_edge / has_edge ─── */

static int test_add_edge_directed(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v2");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge 0->1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add edge 0->2");

    ASSERT(zenit_graph_has_edge(g, 0, 1), "has_edge 0->1");
    ASSERT(zenit_graph_has_edge(g, 0, 2), "has_edge 0->2");
    /* Directed: reverse should NOT exist */
    ASSERT(!zenit_graph_has_edge(g, 1, 0), "has_edge 1->0 should be false (directed)");
    ASSERT(!zenit_graph_has_edge(g, 2, 0), "has_edge 2->0 should be false (directed)");

    ASSERT(zenit_graph_edge_count(g) == 2, "edge count should be 2");
    zenit_graph_destroy(g);
    return 0;
}

static int test_add_edge_undirected(void) {
    zenit_graph_t *g = zenit_graph_create(4, 0);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge 0-1");

    ASSERT(zenit_graph_has_edge(g, 0, 1), "has_edge 0->1 (undirected)");
    ASSERT(zenit_graph_has_edge(g, 1, 0), "has_edge 1->0 (undirected)");

    /* One logical edge for undirected */
    ASSERT(zenit_graph_edge_count(g) == 1, "edge count should be 1");
    zenit_graph_destroy(g);
    return 0;
}

static int test_add_edge_duplicate(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "first add");
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "duplicate should be OK");
    ASSERT(zenit_graph_edge_count(g) == 1, "edge count still 1 after duplicate");
    zenit_graph_destroy(g);
    return 0;
}

static int test_add_edge_self_loop(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");

    ASSERT(zenit_graph_add_edge(g, 0, 0).error == ZENIT_OK, "self-loop");
    ASSERT(zenit_graph_has_edge(g, 0, 0), "has_edge self-loop");
    ASSERT(zenit_graph_edge_count(g) == 1, "edge count 1 for self-loop");
    zenit_graph_destroy(g);
    return 0;
}

static int test_add_edge_null(void) {
    zenit_result_t r = zenit_graph_add_edge(NULL, 0, 1);
    ASSERT(r.error == ZENIT_ERROR_NULL, "add_edge NULL graph");
    return 0;
}

static int test_add_edge_invalid_vertex(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");

    /* Vertex 5 does not exist */
    zenit_result_t r = zenit_graph_add_edge(g, 0, 5);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "add_edge invalid to vertex");

    r = zenit_graph_add_edge(g, 5, 0);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "add_edge invalid from vertex");

    zenit_graph_destroy(g);
    return 0;
}

/* ─── remove_edge ─── */

static int test_remove_edge_directed(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge");
    ASSERT(zenit_graph_has_edge(g, 0, 1), "has_edge before remove");

    ASSERT(zenit_graph_remove_edge(g, 0, 1).error == ZENIT_OK, "remove edge");
    ASSERT(!zenit_graph_has_edge(g, 0, 1), "has_edge false after remove");
    ASSERT(zenit_graph_edge_count(g) == 0, "edge count 0 after remove");
    zenit_graph_destroy(g);
    return 0;
}

static int test_remove_edge_undirected(void) {
    zenit_graph_t *g = zenit_graph_create(4, 0);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge");
    ASSERT(zenit_graph_remove_edge(g, 0, 1).error == ZENIT_OK, "remove edge");
    ASSERT(!zenit_graph_has_edge(g, 0, 1), "has_edge 0->1 false");
    ASSERT(!zenit_graph_has_edge(g, 1, 0), "has_edge 1->0 false");
    ASSERT(zenit_graph_edge_count(g) == 0, "edge count 0");
    zenit_graph_destroy(g);
    return 0;
}

static int test_remove_edge_nonexistent(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");

    /* Removing a non-existent edge is a no-op (returns OK) */
    ASSERT(zenit_graph_remove_edge(g, 0, 1).error == ZENIT_OK, "remove nonexistent edge");
    zenit_graph_destroy(g);
    return 0;
}

static int test_remove_edge_null(void) {
    zenit_result_t r = zenit_graph_remove_edge(NULL, 0, 1);
    ASSERT(r.error == ZENIT_ERROR_NULL, "remove_edge NULL graph");
    return 0;
}

/* ─── remove_vertex ─── */

static int test_remove_vertex_basic(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v2");
    ASSERT(zenit_graph_vertex_count(g) == 3, "vertex count 3");

    ASSERT(zenit_graph_remove_vertex(g, 1).error == ZENIT_OK, "remove v1");
    ASSERT(zenit_graph_vertex_count(g) == 2, "vertex count 2 after remove");

    /* Removing the same vertex again should fail */
    zenit_result_t r = zenit_graph_remove_vertex(g, 1);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "double remove fails");

    zenit_graph_destroy(g);
    return 0;
}

static int test_remove_vertex_removes_edges(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v2");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0->1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add 0->2");
    ASSERT(zenit_graph_edge_count(g) == 2, "edge count 2");

    /* Remove vertex 0 — should remove both edges */
    ASSERT(zenit_graph_remove_vertex(g, 0).error == ZENIT_OK, "remove v0");
    ASSERT(zenit_graph_edge_count(g) == 0, "edge count 0 after v0 removed");
    ASSERT(!zenit_graph_has_edge(g, 0, 1), "edge 0->1 gone");
    ASSERT(!zenit_graph_has_edge(g, 0, 2), "edge 0->2 gone");

    zenit_graph_destroy(g);
    return 0;
}

static int test_remove_vertex_null(void) {
    zenit_result_t r = zenit_graph_remove_vertex(NULL, 0);
    ASSERT(r.error == ZENIT_ERROR_NULL, "remove_vertex NULL graph");
    return 0;
}

static int test_remove_vertex_invalid(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    zenit_result_t r = zenit_graph_remove_vertex(g, 99);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "remove invalid vertex");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── get_neighbors ─── */

static int test_get_neighbors_basic(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v2");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v3");

    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0->1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add 0->2");
    ASSERT(zenit_graph_add_edge(g, 0, 3).error == ZENIT_OK, "add 0->3");

    int *neighbors = NULL;
    size_t count = 0;
    int ret = zenit_graph_get_neighbors(g, 0, &neighbors, &count);

    ASSERT(ret == 3, "get_neighbors count return 3");
    ASSERT(count == 3, "out_count 3");

    int found1 = 0;
    int found2 = 0;
    int found3 = 0;
    for (size_t i = 0; i < count; i++) {
        if (neighbors[i] == 1) found1 = 1;
        if (neighbors[i] == 2) found2 = 1;
        if (neighbors[i] == 3) found3 = 1;
    }
    ASSERT(found1 && found2 && found3, "all neighbors found");

    free(neighbors);
    zenit_graph_destroy(g);
    return 0;
}

static int test_get_neighbors_empty(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");

    int *neighbors = NULL;
    size_t count = 99;
    int ret = zenit_graph_get_neighbors(g, 0, &neighbors, &count);
    ASSERT(ret == 0, "empty neighbor count 0");
    ASSERT(count == 0, "out_count 0");
    ASSERT(neighbors == NULL, "neighbors NULL for empty");

    zenit_graph_destroy(g);
    return 0;
}

static int test_get_neighbors_invalid(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    int *neighbors = NULL;
    size_t count = 0;
    int ret = zenit_graph_get_neighbors(g, 99, &neighbors, &count);
    ASSERT(ret == -1, "invalid vertex returns -1");

    zenit_graph_destroy(g);
    return 0;
}

static int test_get_neighbors_null(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    int *neighbors = NULL;
    size_t count = 0;
    int ret = zenit_graph_get_neighbors(NULL, 0, &neighbors, &count);
    ASSERT(ret == -1, "NULL graph returns -1");

    ret = zenit_graph_get_neighbors(g, 0, NULL, &count);
    ASSERT(ret == -1, "NULL out_neighbors returns -1");

    ret = zenit_graph_get_neighbors(g, 0, &neighbors, NULL);
    ASSERT(ret == -1, "NULL out_count returns -1");

    zenit_graph_destroy(g);
    return 0;
}

/* ─── BFS ─── */

typedef struct {
    int *visited;
    size_t count;
    size_t capacity;
    int stop_at;
} visit_ctx_t;

static int visit_recorder(int vertex, void *ctx) {
    visit_ctx_t *vc = (visit_ctx_t *)ctx;
    if (vc->count < vc->capacity) {
        vc->visited[vc->count++] = vertex;
    }
    /* Return non-zero to stop if stop_at matches */
    return (vertex == vc->stop_at) ? 1 : 0;
}

static int visit_noop(int vertex, void *ctx) {
    (void)vertex;
    (void)ctx;
    return 0;
}

static int test_bfs_basic(void) {
    zenit_graph_t *g = zenit_graph_create(8, 0);
    ASSERT(g != NULL, "create failed");

    /* Build a simple undirected graph:
     *     0
     *    / \
     *   1   2
     *   |   |
     *   3   4
     */
    for (int i = 0; i < 5; i++) {
        ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex");
    }
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0-1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add 0-2");
    ASSERT(zenit_graph_add_edge(g, 1, 3).error == ZENIT_OK, "add 1-3");
    ASSERT(zenit_graph_add_edge(g, 2, 4).error == ZENIT_OK, "add 2-4");

    int buffer[16];
    visit_ctx_t vc = { .visited = buffer, .count = 0, .capacity = 16, .stop_at = -1 };

    zenit_result_t r = zenit_graph_bfs(g, 0, visit_recorder, &vc);
    ASSERT(r.error == ZENIT_OK, "BFS succeeded");
    ASSERT(vc.count == 5, "BFS visited all 5 vertices");

    /* BFS order should be: 0, then 1, 2 (in insertion order), then 3, 4 */
    ASSERT(buffer[0] == 0, "BFS first is 0");
    ASSERT(buffer[1] == 1, "BFS second is 1");
    ASSERT(buffer[2] == 2, "BFS third is 2");
    ASSERT(buffer[3] == 3, "BFS fourth is 3");
    ASSERT(buffer[4] == 4, "BFS fifth is 4");

    zenit_graph_destroy(g);
    return 0;
}

static int test_bfs_stop_early(void) {
    zenit_graph_t *g = zenit_graph_create(4, 0);
    ASSERT(g != NULL, "create failed");

    for (int i = 0; i < 5; i++) {
        ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex");
    }
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0-1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add 0-2");
    ASSERT(zenit_graph_add_edge(g, 1, 3).error == ZENIT_OK, "add 1-3");
    ASSERT(zenit_graph_add_edge(g, 2, 4).error == ZENIT_OK, "add 2-4");

    int buffer[16];
    visit_ctx_t vc = { .visited = buffer, .count = 0, .capacity = 16, .stop_at = 2 };

    zenit_graph_bfs(g, 0, visit_recorder, &vc);
    /* Should stop when visiting 2 — visited at most 0,1,2 */
    ASSERT(vc.count <= 3, "BFS stopped early at v2");

    zenit_graph_destroy(g);
    return 0;
}

static int test_bfs_null(void) {
    zenit_result_t r = zenit_graph_bfs(NULL, 0, visit_recorder, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "BFS NULL graph");

    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");
    r = zenit_graph_bfs(g, 0, NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "BFS NULL visit");
    zenit_graph_destroy(g);
    return 0;
}

static int test_bfs_invalid_start(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    zenit_result_t r = zenit_graph_bfs(g, 99, visit_recorder, NULL);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "BFS invalid start");

    zenit_graph_destroy(g);
    return 0;
}

/* ─── DFS ─── */

static int test_dfs_visits_all(void) {
    zenit_graph_t *g = zenit_graph_create(8, 0);
    ASSERT(g != NULL, "create failed");

    /* Same graph as BFS test */
    for (int i = 0; i < 5; i++) {
        ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex");
    }
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0-1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add 0-2");
    ASSERT(zenit_graph_add_edge(g, 1, 3).error == ZENIT_OK, "add 1-3");
    ASSERT(zenit_graph_add_edge(g, 2, 4).error == ZENIT_OK, "add 2-4");

    int buffer[16];
    visit_ctx_t vc = { .visited = buffer, .count = 0, .capacity = 16, .stop_at = -1 };

    zenit_result_t r = zenit_graph_dfs(g, 0, visit_recorder, &vc);
    ASSERT(r.error == ZENIT_OK, "DFS succeeded");
    ASSERT(vc.count == 5, "DFS visited all 5 vertices");

    zenit_graph_destroy(g);
    return 0;
}

static int test_dfs_stop_early(void) {
    zenit_graph_t *g = zenit_graph_create(4, 0);
    ASSERT(g != NULL, "create failed");

    for (int i = 0; i < 5; i++) {
        ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex");
    }
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0-1");
    ASSERT(zenit_graph_add_edge(g, 0, 2).error == ZENIT_OK, "add 0-2");

    int buffer[16];
    visit_ctx_t vc = { .visited = buffer, .count = 0, .capacity = 16, .stop_at = 1 };

    zenit_graph_dfs(g, 0, visit_recorder, &vc);
    /* Should stop at vertex 1, so visited count is at most 2 */
    ASSERT(vc.count <= 2, "DFS stopped early");

    zenit_graph_destroy(g);
    return 0;
}

static int test_dfs_null(void) {
    zenit_result_t r = zenit_graph_dfs(NULL, 0, visit_recorder, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "DFS NULL graph");

    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");
    r = zenit_graph_dfs(g, 0, NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "DFS NULL visit");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── clear ─── */

static int test_clear_graph(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create failed");

    for (int i = 0; i < 5; i++) {
        ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex");
    }
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge");
    ASSERT(zenit_graph_add_edge(g, 2, 3).error == ZENIT_OK, "add edge");

    zenit_graph_clear(g);
    ASSERT(zenit_graph_vertex_count(g) == 0, "vertex count 0 after clear");
    ASSERT(zenit_graph_edge_count(g) == 0, "edge count 0 after clear");

    /* Graph should still be usable after clear */
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex after clear");
    ASSERT(zenit_graph_vertex_count(g) == 1, "vertex count 1 after re-add");

    zenit_graph_destroy(g);
    return 0;
}

static int test_clear_null(void) {
    /* Should not crash */
    zenit_graph_clear(NULL);
    return 0;
}

/* ─── large graph stress test ─── */

static int test_many_vertices_edges(void) {
    zenit_graph_t *g = zenit_graph_create(16, 0);
    ASSERT(g != NULL, "create failed");

    /* Add 100 vertices */
    int n = 100;
    for (int i = 0; i < n; i++) {
        ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add vertex in stress");
    }
    ASSERT(zenit_graph_vertex_count(g) == (size_t)n, "100 vertices");

    /* Add a chain: 0-1, 1-2, 2-3, ... */
    for (int i = 0; i < n - 1; i++) {
        ASSERT(zenit_graph_add_edge(g, i, i + 1).error == ZENIT_OK, "add chain edge");
    }
    ASSERT(zenit_graph_edge_count(g) == (size_t)(n - 1), "99 edges in chain");

    /* Verify has_edge along the chain */
    for (int i = 0; i < n - 1; i++) {
        ASSERT(zenit_graph_has_edge(g, i, i + 1), "has_edge along chain");
    }

    /* Remove vertex 50 */
    ASSERT(zenit_graph_remove_vertex(g, 50).error == ZENIT_OK, "remove middle vertex");

    /* Edge (49,50) and (50,51) should be gone */
    ASSERT(!zenit_graph_has_edge(g, 49, 50), "edge 49-50 gone");
    ASSERT(!zenit_graph_has_edge(g, 50, 51), "edge 50-51 gone");

    zenit_graph_destroy(g);
    return 0;
}

/* ─── Allocation-failure tests (custom allocator) ─── */

static int test_create_with_allocator_fail_counts(void) {
    /* Alloc #0 = handle, #1 = adj, #2 = counts ← fail here */
    test_alloc_ctx_t ctx = { 0, 2, 0, -1 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    const zenit_graph_t *g = zenit_graph_create_with_allocator(4, 1, a);
    ASSERT(g == NULL, "create should return NULL when counts alloc fails");
    return 0;
}

static int test_grow_vertex_arrays_adj_realloc_fail(void) {
    test_alloc_ctx_t ctx = { 0, -1, 0, 0 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    zenit_graph_t *g = zenit_graph_create_with_allocator(2, 1, a);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    /* 3rd vertex triggers grow_vertex_arrays(2→3):
     * realloc #0 = adj ← fail here */
    zenit_result_t r = zenit_graph_add_vertex(g);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "grow adj realloc fail");
    zenit_graph_destroy(g);
    return 0;
}

static int test_grow_vertex_arrays_counts_realloc_fail(void) {
    test_alloc_ctx_t ctx = { 0, -1, 0, 1 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    zenit_graph_t *g = zenit_graph_create_with_allocator(2, 1, a);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    /* realloc #0 = adj (succeeds), realloc #1 = counts ← fail here */
    zenit_result_t r = zenit_graph_add_vertex(g);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "grow counts realloc fail");
    zenit_graph_destroy(g);
    return 0;
}

static int test_grow_vertex_arrays_capacities_realloc_fail(void) {
    test_alloc_ctx_t ctx = { 0, -1, 0, 2 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    zenit_graph_t *g = zenit_graph_create_with_allocator(2, 1, a);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    /* realloc #0 = adj (OK), #1 = counts (OK), #2 = capacities ← fail here */
    zenit_result_t r = zenit_graph_add_vertex(g);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "grow capacities realloc fail");
    zenit_graph_destroy(g);
    return 0;
}

static int test_grow_vertex_arrays_removed_realloc_fail(void) {
    test_alloc_ctx_t ctx = { 0, -1, 0, 3 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    zenit_graph_t *g = zenit_graph_create_with_allocator(2, 1, a);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    /* realloc #0..2 = adj, counts, capacities (OK), #3 = removed ← fail here */
    zenit_result_t r = zenit_graph_add_vertex(g);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "grow removed realloc fail");
    zenit_graph_destroy(g);
    return 0;
}

static int test_grow_adj_list_realloc_fail(void) {
    test_alloc_ctx_t ctx = { 0, -1, 0, 0 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    zenit_graph_t *g = zenit_graph_create_with_allocator(8, 1, a);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    /* First edge triggers grow_adj_list(0, 0→8): realloc #0 fails */
    zenit_result_t r = zenit_graph_add_edge(g, 0, 1);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "grow_adj_list realloc fail");
    zenit_graph_destroy(g);
    return 0;
}

static int test_undirected_add_edge_reverse_growth_fail(void) {
    test_alloc_ctx_t ctx = { 0, -1, 0, 1 };
    zenit_allocator_t a = { test_alloc, test_realloc, test_free, &ctx };
    zenit_graph_t *g = zenit_graph_create_with_allocator(8, 0, a);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    /* First edge: forward grow_adj_list succeeds (realloc #0),
     * reverse grow_adj_list fails (realloc #1) — rollback expected */
    zenit_result_t r = zenit_graph_add_edge(g, 0, 1);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "reverse growth fail");
    ASSERT(zenit_graph_edge_count(g) == 0, "edge count rolled back");
    ASSERT(!zenit_graph_has_edge(g, 0, 1), "forward edge rolled back");
    ASSERT(!zenit_graph_has_edge(g, 1, 0), "reverse edge not present");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── remove_edge invalid-param coverage ─── */

static int test_remove_edge_invalid_vertices(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_remove_edge(g, 99, 0).error == ZENIT_ERROR_PARAM, "remove_edge invalid from");
    ASSERT(zenit_graph_remove_edge(g, 0, 99).error == ZENIT_ERROR_PARAM, "remove_edge invalid to");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── has_edge coverage ─── */

static int test_has_edge_null(void) {
    ASSERT(zenit_graph_has_edge(NULL, 0, 1) == 0, "has_edge NULL graph");
    return 0;
}

static int test_has_edge_invalid_vertices(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_has_edge(g, -1, 0) == 0, "has_edge negative from");
    ASSERT(zenit_graph_has_edge(g, 99, 0) == 0, "has_edge out-of-range from");
    ASSERT(zenit_graph_has_edge(g, 0, -1) == 0, "has_edge negative to");
    ASSERT(zenit_graph_has_edge(g, 0, 99) == 0, "has_edge out-of-range to");
    zenit_graph_destroy(g);
    return 0;
}

static int test_has_edge_removed_from(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge");
    ASSERT(zenit_graph_remove_vertex(g, 0).error == ZENIT_OK, "remove v0");
    ASSERT(zenit_graph_has_edge(g, 0, 1) == 0, "has_edge removed from");
    zenit_graph_destroy(g);
    return 0;
}

static int test_has_edge_removed_to(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add edge");
    ASSERT(zenit_graph_remove_vertex(g, 1).error == ZENIT_OK, "remove v1");
    ASSERT(zenit_graph_has_edge(g, 0, 1) == 0, "has_edge removed to");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── get_neighbors with removed vertex ─── */

static int test_get_neighbors_removed_vertex(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_remove_vertex(g, 0).error == ZENIT_OK, "remove v0");
    int *neighbors = NULL;
    size_t count = 0;
    int ret = zenit_graph_get_neighbors(g, 0, &neighbors, &count);
    ASSERT(ret == -1, "removed vertex returns -1");
    ASSERT(neighbors == NULL, "out_neighbors NULL");
    ASSERT(count == 0, "out_count 0");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── NULL query coverage ─── */

static int test_vertex_count_null(void) {
    ASSERT(zenit_graph_vertex_count(NULL) == 0, "vertex_count(NULL)");
    return 0;
}

static int test_edge_count_null(void) {
    ASSERT(zenit_graph_edge_count(NULL) == 0, "edge_count(NULL)");
    return 0;
}

static int test_is_directed_null(void) {
    ASSERT(zenit_graph_is_directed(NULL) == 0, "is_directed(NULL)");
    return 0;
}

/* ─── remove_vertex on directed graph ─── */

static int test_remove_vertex_directed_edge_count(void) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    ASSERT(g != NULL, "create");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v0");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v1");
    ASSERT(zenit_graph_add_vertex(g).error == ZENIT_OK, "add v2");
    ASSERT(zenit_graph_add_edge(g, 0, 1).error == ZENIT_OK, "add 0→1");
    ASSERT(zenit_graph_add_edge(g, 1, 2).error == ZENIT_OK, "add 1→2");
    ASSERT(zenit_graph_add_edge(g, 2, 0).error == ZENIT_OK, "add 2→0");
    ASSERT(zenit_graph_edge_count(g) == 3, "3 edges before remove");
    /* Remove vertex 0: outgoing 0→1 (1 edge), incoming 2→0 (1 edge) are removed */
    ASSERT(zenit_graph_remove_vertex(g, 0).error == ZENIT_OK, "remove v0");
    /* Only edge 1→2 remains */
    ASSERT(zenit_graph_edge_count(g) == 1, "1 edge after remove v0");
    ASSERT(zenit_graph_has_edge(g, 1, 2), "edge 1→2 still exists");
    zenit_graph_destroy(g);
    return 0;
}

/* ─── BFS/DFS start-vertex coverage ─── */

typedef zenit_result_t (*traverse_fn_t)(const zenit_graph_t*, int, int(*)(int, void*), void*);

static int test_traverse_invalid_starts(traverse_fn_t fn, const char* label) {
    zenit_graph_t *g = zenit_graph_create(4, 1);
    if (g == NULL) { fprintf(stderr, "FAIL: %s create\n", label); return 1; }

    zenit_result_t r = fn(g, -1, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: %s negative start\n", label); zenit_graph_destroy(g); return 1; }

    zenit_graph_add_vertex(g);
    zenit_graph_remove_vertex(g, 0);
    r = fn(g, 0, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: %s removed start\n", label); zenit_graph_destroy(g); return 1; }

    r = fn(g, 99, visit_noop, NULL);
    if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: %s out-of-range start\n", label); zenit_graph_destroy(g); return 1; }

    zenit_graph_destroy(g);
    return 0;
}

static int test_bfs_invalid_starts(void) {
    return test_traverse_invalid_starts(zenit_graph_bfs, "BFS");
}

static int test_dfs_invalid_starts(void) {
    return test_traverse_invalid_starts(zenit_graph_dfs, "DFS");
}

/* ─── test suite runner ─── */

int main(void) {
    int (*tests[])(void) = {
        &test_create_directed,
        &test_create_undirected,
        &test_create_zero_capacity,
        &test_destroy_null,
        &test_add_vertex_basic,
        &test_add_vertex_null,
        &test_add_edge_directed,
        &test_add_edge_undirected,
        &test_add_edge_duplicate,
        &test_add_edge_self_loop,
        &test_add_edge_null,
        &test_add_edge_invalid_vertex,
        &test_remove_edge_directed,
        &test_remove_edge_undirected,
        &test_remove_edge_nonexistent,
        &test_remove_edge_null,
        &test_remove_vertex_basic,
        &test_remove_vertex_removes_edges,
        &test_remove_vertex_null,
        &test_remove_vertex_invalid,
        &test_get_neighbors_basic,
        &test_get_neighbors_empty,
        &test_get_neighbors_invalid,
        &test_get_neighbors_null,
        &test_bfs_basic,
        &test_bfs_stop_early,
        &test_bfs_null,
        &test_bfs_invalid_start,
        &test_dfs_visits_all,
        &test_dfs_stop_early,
        &test_dfs_null,
        &test_clear_graph,
        &test_clear_null,
        &test_many_vertices_edges,
        &test_create_with_allocator_fail_counts,
        &test_grow_vertex_arrays_adj_realloc_fail,
        &test_grow_vertex_arrays_counts_realloc_fail,
        &test_grow_vertex_arrays_capacities_realloc_fail,
        &test_grow_vertex_arrays_removed_realloc_fail,
        &test_grow_adj_list_realloc_fail,
        &test_undirected_add_edge_reverse_growth_fail,
        &test_remove_edge_invalid_vertices,
        &test_has_edge_null,
        &test_has_edge_invalid_vertices,
        &test_has_edge_removed_from,
        &test_has_edge_removed_to,
        &test_get_neighbors_removed_vertex,
        &test_vertex_count_null,
        &test_edge_count_null,
        &test_is_directed_null,
        &test_remove_vertex_directed_edge_count,
        &test_bfs_invalid_starts,
        &test_dfs_invalid_starts,
    };
    const char *names[] = {
        "create_directed",
        "create_undirected",
        "create_zero_capacity",
        "destroy_null",
        "add_vertex_basic",
        "add_vertex_null",
        "add_edge_directed",
        "add_edge_undirected",
        "add_edge_duplicate",
        "add_edge_self_loop",
        "add_edge_null",
        "add_edge_invalid_vertex",
        "remove_edge_directed",
        "remove_edge_undirected",
        "remove_edge_nonexistent",
        "remove_edge_null",
        "remove_vertex_basic",
        "remove_vertex_removes_edges",
        "remove_vertex_null",
        "remove_vertex_invalid",
        "get_neighbors_basic",
        "get_neighbors_empty",
        "get_neighbors_invalid",
        "get_neighbors_null",
        "bfs_basic",
        "bfs_stop_early",
        "bfs_null",
        "bfs_invalid_start",
        "dfs_visits_all",
        "dfs_stop_early",
        "dfs_null",
        "clear_graph",
        "clear_null",
        "many_vertices_edges",
        "create_with_allocator_fail_counts",
        "grow_vertex_arrays_adj_realloc_fail",
        "grow_vertex_arrays_counts_realloc_fail",
        "grow_vertex_arrays_capacities_realloc_fail",
        "grow_vertex_arrays_removed_realloc_fail",
        "grow_adj_list_realloc_fail",
        "undirected_add_edge_reverse_growth_fail",
        "remove_edge_invalid_vertices",
        "has_edge_null",
        "has_edge_invalid_vertices",
        "has_edge_removed_from",
        "has_edge_removed_to",
        "get_neighbors_removed_vertex",
        "vertex_count_null",
        "edge_count_null",
        "is_directed_null",
        "remove_vertex_directed_edge_count",
        "bfs_invalid_starts",
        "dfs_invalid_starts",
    };
    ZENIT_RUN_TESTS("graph", tests, names);
}
