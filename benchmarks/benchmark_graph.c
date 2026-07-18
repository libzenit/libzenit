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

#include <libzenit/benchmark.h>
#include <libzenit/graph.h>
#include <stdlib.h>

/* Simple deterministic PRNG (xorshift32) */
static unsigned xorshift32(unsigned *state) {
    unsigned x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

#define N_VERTICES_ADD 100000
#define N_EDGES_ADD 100000
#define N_BFS 10000

/* ─── add_vertex benchmark (100K) ─── */

typedef struct {
    zenit_graph_t *g;
    int n;
} graph_ctx_t;

static void bench_add_vertex_fn(void *ctx) {
    const graph_ctx_t *c = (const graph_ctx_t *)ctx;
    /* Add a single vertex per invocation */
    zenit_graph_add_vertex(c->g);
}

/* ─── add_edge benchmark (100K edges on 10K vertices) ─── */

typedef struct {
    zenit_graph_t *g;
    int *edges_from;
    int *edges_to;
    int count;
    int idx;
} edges_ctx_t;

static void bench_add_edge_fn(void *ctx) {
    edges_ctx_t *c = (edges_ctx_t *)ctx;
    /* Add edges one at a time (invoked once per edge, cycling through pairs) */
    zenit_graph_add_edge(c->g, c->edges_from[c->idx], c->edges_to[c->idx]);
    c->idx = (c->idx + 1) % c->count;
}

/* ─── BFS benchmark (10K vertices) ─── */

/* Visit callback that does nothing — just counts */
static int bench_visit_count(int vertex, void *ctx) {
    (void)vertex;
    size_t *count = (size_t *)ctx;
    (*count)++;
    return 0;
}

static void bench_bfs_fn(void *ctx) {
    const graph_ctx_t *c = (const graph_ctx_t *)ctx;
    size_t visited_count = 0;
    zenit_graph_bfs(c->g, 0, bench_visit_count, &visited_count);
}

/* ─── get_neighbors benchmark (10K vertices) ─── */

static void bench_get_neighbors_fn(void *ctx) {
    const graph_ctx_t *c = (const graph_ctx_t *)ctx;
    int *neighbors = NULL;
    size_t count = 0;
    /* Query a vertex in the middle of the range */
    zenit_graph_get_neighbors(c->g, c->n / 2, &neighbors, &count);
    free(neighbors);
}

int main(void) {
    /* ─── Benchmark 1: add_vertex (100K) ─── */
    {
        zenit_graph_t *g = zenit_graph_create(1024, 0);
        if (g == NULL) return 1;

        /* Pre-populate so we measure only add_vertex overhead */
        graph_ctx_t ctx = { .g = g, .n = N_VERTICES_ADD };

        zenit_bench_result_t r = zenit_bench_run(
            "graph_add_vertex_100K", bench_add_vertex_fn, &ctx, N_VERTICES_ADD
        );
        zenit_bench_print(&r);
        zenit_graph_destroy(g);
    }

    /* ─── Benchmark 2: add_edge (100K on 10K vertices) ─── */
    {
        zenit_graph_t *g = zenit_graph_create(1024, 1);
        if (g == NULL) return 1;

        /* Add vertices first */
        for (int i = 0; i < N_BFS; i++) {
            zenit_graph_add_vertex(g);
        }

        /* Generate random edges */
        int *from = malloc(N_EDGES_ADD * sizeof(int));
        int *to = malloc(N_EDGES_ADD * sizeof(int));
        if (from == NULL || to == NULL) {
            free(from);
            free(to);
            zenit_graph_destroy(g);
            return 1;
        }

        unsigned rng = 12345;
        for (int i = 0; i < N_EDGES_ADD; i++) {
            from[i] = (int)(xorshift32(&rng) % N_BFS);
            to[i] = (int)(xorshift32(&rng) % N_BFS);
        }

        edges_ctx_t ctx = { .g = g, .edges_from = from, .edges_to = to, .count = N_EDGES_ADD, .idx = 0 };

        zenit_bench_result_t r = zenit_bench_run(
            "graph_add_edge_100K", bench_add_edge_fn, &ctx, N_EDGES_ADD
        );
        zenit_bench_print(&r);

        free(from);
        free(to);
        zenit_graph_destroy(g);
    }

    /* ─── Benchmark 3: BFS (10K vertices, random edges) ─── */
    {
        zenit_graph_t *g = zenit_graph_create(1024, 0);
        if (g == NULL) return 1;

        for (int i = 0; i < N_BFS; i++) {
            zenit_graph_add_vertex(g);
        }

        /* Connect each vertex to 3 random neighbors (undirected) */
        unsigned rng = 67890;
        for (int i = 0; i < N_BFS; i++) {
            for (int j = 0; j < 3; j++) {
                int neighbor = (int)(xorshift32(&rng) % N_BFS);
                zenit_graph_add_edge(g, i, neighbor);
            }
        }

        graph_ctx_t ctx = { .g = g, .n = N_BFS };

        zenit_bench_result_t r = zenit_bench_run(
            "graph_bfs_10K", bench_bfs_fn, &ctx, 10000
        );
        zenit_bench_print(&r);
        zenit_graph_destroy(g);
    }

    /* ─── Benchmark 4: get_neighbors (10K vertices) ─── */
    {
        zenit_graph_t *g = zenit_graph_create(1024, 1);
        if (g == NULL) return 1;

        for (int i = 0; i < N_BFS; i++) {
            zenit_graph_add_vertex(g);
        }

        /* Each vertex gets 5 random outgoing edges */
        unsigned rng = 24680;
        for (int i = 0; i < N_BFS; i++) {
            for (int j = 0; j < 5; j++) {
                int neighbor = (int)(xorshift32(&rng) % N_BFS);
                zenit_graph_add_edge(g, i, neighbor);
            }
        }

        graph_ctx_t ctx = { .g = g, .n = N_BFS };

        zenit_bench_result_t r = zenit_bench_run(
            "graph_get_neighbors_10K", bench_get_neighbors_fn, &ctx, 100000
        );
        zenit_bench_print(&r);
        zenit_graph_destroy(g);
    }

    return 0;
}
