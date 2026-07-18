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
    };
    ZENIT_RUN_TESTS("graph", tests, names);
}
