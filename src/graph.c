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
#include <stdlib.h>
#include <string.h>

/* ─── Internal structure ─── */

/**
 * @brief Internal graph state.
 *
 * Adjacency lists are stored as a jagged array: each vertex has a
 * dynamically-grown array of neighbor IDs.  Removed vertices are marked
 * with a tombstone but their IDs are never reused, keeping all existing
 * vertex references stable.
 */
struct zenit_graph_t {
    int **adj;                /**< Jagged array of adjacency lists. */
    size_t *counts;           /**< Number of neighbors per vertex. */
    size_t *capacities;       /**< Allocated capacity per adjacency list. */
    size_t vertex_count;      /**< Number of active (non-tombstone) vertices. */
    size_t vertex_capacity;   /**< Total allocated vertex slots. */
    size_t edge_count;        /**< Total number of logical edges. */
    int directed;             /**< Non-zero if the graph is directed. */
    int *removed;             /**< Tombstone bitset: 0=active, 1=removed. */
    zenit_allocator_t allocator; /**< Allocator used for all internal memory. */
};

/* ─── Default initial capacity when zero is passed ─── */
#define ZENIT_GRAPH_DEFAULT_CAP 8

/* ─── Growth factor: 1.5x ─── */
static size_t grow_capacity(size_t old) {
    if (old == 0) return ZENIT_GRAPH_DEFAULT_CAP;
    return old + old / 2;
}

/* ─── Grow vertex arrays to accommodate more slots ─── */

/**
 * @brief Grow the vertex-level arrays (adj, counts, capacities, removed).
 *
 * Uses the graph's allocator (via zenit_allocator_realloc) to resize each
 * array.  New slots are zeroed and marked as removed (tombstone).
 *
 * @param g Graph handle.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_ALLOC on failure.
 */
static zenit_result_t grow_vertex_arrays(zenit_graph_t *g) {
    /* Calculate new capacity using 1.5x growth factor */
    size_t old_cap = g->vertex_capacity;
    size_t new_cap = grow_capacity(old_cap);
    zenit_allocator_t a = g->allocator;

    /* Resize the adjacency-list pointer array */
    int **new_adj = zenit_allocator_realloc(a, g->adj, old_cap * sizeof(int*), new_cap * sizeof(int*));
    if (new_adj == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    g->adj = new_adj;

    /* Resize the counts array */
    size_t *new_counts = zenit_allocator_realloc(a, g->counts, old_cap * sizeof(size_t), new_cap * sizeof(size_t));
    if (new_counts == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    g->counts = new_counts;

    /* Resize the capacities array */
    size_t *new_capacities = zenit_allocator_realloc(a, g->capacities, old_cap * sizeof(size_t), new_cap * sizeof(size_t));
    if (new_capacities == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    g->capacities = new_capacities;

    /* Resize the removed (tombstone) array */
    int *new_removed = zenit_allocator_realloc(a, g->removed, old_cap * sizeof(int), new_cap * sizeof(int));
    if (new_removed == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    g->removed = new_removed;

    /* Zero-initialise the new slots in adj, counts, capacities */
    size_t n = new_cap - old_cap;
    memset(g->adj + old_cap, 0, n * sizeof(int*));
    memset(g->counts + old_cap, 0, n * sizeof(size_t));
    memset(g->capacities + old_cap, 0, n * sizeof(size_t));

    /* Mark new slots as tombstones (available for new vertices) */
    for (size_t i = old_cap; i < new_cap; i++) {
        g->removed[i] = 1;
    }

    g->vertex_capacity = new_cap;
    return ZENIT_RESULT_OK;
}

/* ─── Grow a single vertex's adjacency list ─── */

/**
 * @brief Grow the adjacency list of a specific vertex.
 *
 * Uses 1.5x growth factor.  Returns ZENIT_ERROR_ALLOC on failure.
 *
 * @param g      Graph handle.
 * @param vertex Vertex whose adjacency list should grow.
 * @return zenit_result_t indicating success or allocation failure.
 */
static zenit_result_t grow_adj_list(zenit_graph_t *g, int vertex) {
    size_t old_cap = g->capacities[vertex];
    size_t new_cap = grow_capacity(old_cap);
    zenit_allocator_t a = g->allocator;

    int *new_list = zenit_allocator_realloc(a, g->adj[vertex], old_cap * sizeof(int), new_cap * sizeof(int));
    if (new_list == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    g->adj[vertex] = new_list;
    g->capacities[vertex] = new_cap;
    return ZENIT_RESULT_OK;
}

/* ─── Create / Destroy ─── */

/**
 * @brief Create an empty graph with a custom allocator.
 *
 * Allocates the handle and the four vertex arrays (adj, counts, capacities,
 * removed).  Every slot is initially marked as a tombstone (removed=1) so
 * that add_vertex knows which slots are free.
 */
zenit_graph_t *zenit_graph_create_with_allocator(size_t initial_capacity, int directed, zenit_allocator_t allocator) {
    /* Clamp initial_capacity to a reasonable minimum */
    size_t cap = initial_capacity;
    if (cap == 0) {
        cap = ZENIT_GRAPH_DEFAULT_CAP;
    }

    /* Allocate the handle itself via the custom allocator */
    zenit_graph_t *g = allocator.alloc_fn(sizeof(zenit_graph_t), allocator.ctx);
    if (g == NULL) {
        return NULL;
    }

    /* Allocate the four vertex-level arrays (zeroed — calloc equivalent) */
    g->adj = zenit_allocator_alloc_zero(allocator, cap, sizeof(int*));
    g->counts = zenit_allocator_alloc_zero(allocator, cap, sizeof(size_t));
    g->capacities = zenit_allocator_alloc_zero(allocator, cap, sizeof(size_t));
    g->removed = zenit_allocator_alloc_zero(allocator, cap, sizeof(int));

    /* If any allocation failed, free everything and return NULL */
    if (g->adj == NULL || g->counts == NULL || g->capacities == NULL || g->removed == NULL) {
        allocator.free_fn(g->adj, allocator.ctx);
        allocator.free_fn(g->counts, allocator.ctx);
        allocator.free_fn(g->capacities, allocator.ctx);
        allocator.free_fn(g->removed, allocator.ctx);
        allocator.free_fn(g, allocator.ctx);
        return NULL;
    }

    /* Initialise struct fields */
    g->vertex_capacity = cap;
    g->vertex_count = 0;
    g->edge_count = 0;
    g->directed = directed ? 1 : 0;
    g->allocator = allocator;

    /* All slots start as tombstones (available for new vertices) */
    memset(g->removed, 1, cap * sizeof(int));

    return g;
}

/**
 * @brief Create an empty graph with the default malloc-based allocator.
 */
zenit_graph_t *zenit_graph_create(size_t initial_capacity, int directed) {
    return zenit_graph_create_with_allocator(initial_capacity, directed, ZENIT_ALLOCATOR_DEFAULT);
}

/**
 * @brief Destroy a graph and free all owned memory.
 *
 * Walks every vertex slot, frees the adjacency list if present, then frees
 * the four arrays and finally the handle.  NULL-safe.
 */
void zenit_graph_destroy(zenit_graph_t *g) {
    if (g == NULL) {
        return;
    }

    /* Save the allocator before freeing — we need it to free the handle */
    zenit_allocator_t a = g->allocator;

    /* Free every adjacency list */
    for (size_t i = 0; i < g->vertex_capacity; i++) {
        if (g->adj[i] != NULL) {
            a.free_fn(g->adj[i], a.ctx);
        }
    }

    /* Free the four vertex-level arrays */
    a.free_fn(g->adj, a.ctx);
    a.free_fn(g->counts, a.ctx);
    a.free_fn(g->capacities, a.ctx);
    a.free_fn(g->removed, a.ctx);

    /* Free the handle itself */
    a.free_fn(g, a.ctx);
}

/* ─── Vertex operations ─── */

/**
 * @brief Add a new vertex to the graph.
 *
 * Scans for the first tombstone slot.  If none exists and the arrays are
 * full, grows them by 1.5x.  The vertex ID is the array index (never
 * returned by this function — the caller infers it from the order of
 * additions).
 */
zenit_result_t zenit_graph_add_vertex(zenit_graph_t *g) {
    if (g == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Find the first tombstone (removed) slot */
    int slot = -1;
    for (size_t i = 0; i < g->vertex_capacity; i++) {
        if (g->removed[i]) {
            slot = (int)i;
            break;
        }
    }

    /* No tombstone found — grow the vertex arrays */
    if (slot == -1) {
        zenit_result_t r = grow_vertex_arrays(g);
        if (r.error != ZENIT_OK) {
            return r;
        }
        /* After growth, the first new slot is a tombstone — use it */
        slot = (int)g->vertex_count;
    }

    /* Activate the slot */
    g->removed[slot] = 0;
    g->vertex_count++;
    return ZENIT_RESULT_OK;
}

/**
 * @brief Remove a vertex and all edges incident to it.
 *
 * Marks the vertex as removed (tombstone), scans every other vertex's
 * adjacency list to remove any reference to this vertex, updates the
 * edge count, and frees the vertex's own adjacency list.
 */
zenit_result_t zenit_graph_remove_vertex(zenit_graph_t *g, int vertex) {
    if (g == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Validate vertex range */
    if (vertex < 0 || (size_t)vertex >= g->vertex_capacity) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Vertex must be active */
    if (g->removed[vertex]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Decrement edge_count for the outgoing edges of the removed vertex */
    g->edge_count -= g->counts[vertex];

    /* Walk every other active vertex and remove edges pointing to 'vertex' */
    for (size_t i = 0; i < g->vertex_capacity; i++) {
        if (g->removed[i]) {
            continue; /* skip tombstones */
        }
        /* Scan the adjacency list of vertex i for 'vertex' */
        size_t j = 0;
        while (j < g->counts[i]) {
            if (g->adj[i][j] == vertex) {
                /* Shift remaining elements left by one */
                memmove(g->adj[i] + j, g->adj[i] + j + 1, (g->counts[i] - j - 1) * sizeof(int));
                g->counts[i]--;
                /* For directed: each incoming edge is a separate logical edge.
                 * For undirected: each logical edge was already counted once via
                 * counts[vertex], so skip the second decrement. */
                if (g->directed) {
                    g->edge_count--;
                }
            } else {
                j++;
            }
        }
    }

    /* Free the adjacency list of the removed vertex */
    if (g->adj[vertex] != NULL) {
        g->allocator.free_fn(g->adj[vertex], g->allocator.ctx);
        g->adj[vertex] = NULL;
    }
    g->counts[vertex] = 0;
    g->capacities[vertex] = 0;
    g->removed[vertex] = 1;
    g->vertex_count--;

    return ZENIT_RESULT_OK;
}

/* ─── Edge operations ─── */

/**
 * @brief Add an edge between two vertices.
 *
 * For undirected graphs, adds the reverse edge as well (unless from == to).
 * If the edge already exists, returns OK without modifying anything.
 */
zenit_result_t zenit_graph_add_edge(zenit_graph_t *g, int from, int to) {
    if (g == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Validate both vertex endpoints */
    if (from < 0 || (size_t)from >= g->vertex_capacity || g->removed[from]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    if (to < 0 || (size_t)to >= g->vertex_capacity || g->removed[to]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Skip if the edge already exists */
    if (zenit_graph_has_edge(g, from, to)) {
        return ZENIT_RESULT_OK;
    }

    /* Grow the 'from' adjacency list if needed */
    if (g->counts[from] == g->capacities[from]) {
        zenit_result_t r = grow_adj_list(g, from);
        if (r.error != ZENIT_OK) {
            return r;
        }
    }

    /* Append 'to' to the adjacency list of 'from' */
    g->adj[from][g->counts[from]++] = to;
    g->edge_count++;

    /* For undirected graphs, add the reverse edge (but not self-loop twice) */
    if (!g->directed && from != to) {
        /* Grow the 'to' adjacency list if needed — no double edge_count increment */
        if (g->counts[to] == g->capacities[to]) {
            zenit_result_t r = grow_adj_list(g, to);
            if (r.error != ZENIT_OK) {
                /* Undo the forward edge on failure */
                g->counts[from]--;
                g->edge_count--;
                return r;
            }
        }
        /* Append 'from' to the adjacency list of 'to' */
        g->adj[to][g->counts[to]++] = from;
    }

    return ZENIT_RESULT_OK;
}

/**
 * @brief Remove an edge between two vertices.
 *
 * Searches the adjacency list of 'from' for 'to' and shifts elements to
 * close the gap.  For undirected graphs, also removes the reverse edge.
 */
zenit_result_t zenit_graph_remove_edge(zenit_graph_t *g, int from, int to) {
    if (g == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Validate both vertex endpoints */
    if (from < 0 || (size_t)from >= g->vertex_capacity || g->removed[from]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    if (to < 0 || (size_t)to >= g->vertex_capacity || g->removed[to]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    int found = 0;

    /* Scan the adjacency list of 'from' for 'to' */
    for (size_t i = 0; i < g->counts[from]; i++) {
        if (g->adj[from][i] == to) {
            /* Shift remaining elements left by one */
            memmove(g->adj[from] + i, g->adj[from] + i + 1, (g->counts[from] - i - 1) * sizeof(int));
            g->counts[from]--;
            g->edge_count--;
            found = 1;
            break;
        }
    }

    /* If the edge was found and the graph is undirected, remove reverse */
    if (found && !g->directed && from != to) {
        /* Scan and remove 'from' from the adjacency list of 'to' — no double edge_count decrement */
        for (size_t i = 0; i < g->counts[to]; i++) {
            if (g->adj[to][i] == from) {
                memmove(g->adj[to] + i, g->adj[to] + i + 1, (g->counts[to] - i - 1) * sizeof(int));
                g->counts[to]--;
                break;
            }
        }
    }

    return ZENIT_RESULT_OK;
}

/* ─── Queries ─── */

/**
 * @brief Check whether an edge exists between two vertices.
 *
 * Linear scan of the 'from' adjacency list.  Returns 0 for invalid
 * parameters (no error reporting).
 */
int zenit_graph_has_edge(const zenit_graph_t *g, int from, int to) {
    if (g == NULL) {
        return 0;
    }
    if (from < 0 || (size_t)from >= g->vertex_capacity || g->removed[from]) {
        return 0;
    }
    if (to < 0 || (size_t)to >= g->vertex_capacity || g->removed[to]) {
        return 0;
    }

    /* Linear scan — O(deg(from)) */
    for (size_t i = 0; i < g->counts[from]; i++) {
        if (g->adj[from][i] == to) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Return a malloc'd copy of the adjacency list for a vertex.
 *
 * The caller must free the returned array.  Returns -1 on error
 * (NULL parameter, invalid vertex, allocation failure).
 */
int zenit_graph_get_neighbors(const zenit_graph_t *g, int vertex, int **out_neighbors, size_t *out_count) {
    if (g == NULL || out_neighbors == NULL || out_count == NULL) {
        return -1;
    }

    /* Defaults for error returns */
    *out_neighbors = NULL;
    *out_count = 0;

    /* Validate vertex */
    if (vertex < 0 || (size_t)vertex >= g->vertex_capacity || g->removed[vertex]) {
        return -1;
    }

    size_t n = g->counts[vertex];

    /* Return an empty list for isolated vertices */
    if (n == 0) {
        return 0;
    }

    /* Allocate with malloc — caller will use free() */
    int *copy = malloc(n * sizeof(int));
    if (copy == NULL) {
        return -1;
    }

    /* Copy the adjacency list into the freshly-allocated buffer */
    memcpy(copy, g->adj[vertex], n * sizeof(int));
    *out_neighbors = copy;
    *out_count = n;
    return (int)n;
}

/**
 * @brief Return the number of active (non-tombstone) vertices.
 */
size_t zenit_graph_vertex_count(const zenit_graph_t *g) {
    if (g == NULL) {
        return 0;
    }
    return g->vertex_count;
}

/**
 * @brief Return the number of logical edges.
 */
size_t zenit_graph_edge_count(const zenit_graph_t *g) {
    if (g == NULL) {
        return 0;
    }
    return g->edge_count;
}

/**
 * @brief Check whether the graph is directed.
 */
int zenit_graph_is_directed(const zenit_graph_t *g) {
    if (g == NULL) {
        return 0;
    }
    return g->directed;
}

/**
 * @brief Remove all vertices and edges without destroying the graph.
 *
 * Frees every adjacency list, resets all counts to zero, and marks every
 * slot as a tombstone.  Vertex capacity is unchanged.
 */
void zenit_graph_clear(zenit_graph_t *g) {
    if (g == NULL) {
        return;
    }

    zenit_allocator_t a = g->allocator;

    /* Free each adjacency list and reset its meta-data */
    for (size_t i = 0; i < g->vertex_capacity; i++) {
        if (g->adj[i] != NULL) {
            a.free_fn(g->adj[i], a.ctx);
            g->adj[i] = NULL;
        }
        g->counts[i] = 0;
        g->capacities[i] = 0;
        g->removed[i] = 1; /* mark as tombstone (available) */
    }

    g->vertex_count = 0;
    g->edge_count = 0;
}

/* ─── Traversal helpers ─── */

/**
 * @brief Breadth-first traversal using an array-based circular queue.
 *
 * Visits vertices level by level.  The visit function may stop traversal
 * early by returning non-zero.
 */
zenit_result_t zenit_graph_bfs(const zenit_graph_t *g, int start, zenit_graph_visit_fn_t visit, void *ctx) {
    if (g == NULL || visit == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Validate start vertex */
    if (start < 0 || (size_t)start >= g->vertex_capacity || g->removed[start]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Allocate visited array — one element per vertex slot */
    int *visited = calloc(g->vertex_capacity, sizeof(int));
    if (visited == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    /* Allocate an array-based circular queue (worst case: all vertices) */
    int *queue = malloc(g->vertex_capacity * sizeof(int));
    if (queue == NULL) {
        free(visited);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    size_t head = 0;
    size_t tail = 0;

    /* Mark start as visited and enqueue it */
    visited[start] = 1;
    queue[tail++] = start;

    /* BFS loop: dequeue front, visit, enqueue unvisited neighbors */
    while (head < tail) {
        int v = queue[head++];

        /* Call the visit callback — stop if it returns non-zero */
        if (visit(v, ctx)) {
            break;
        }

        /* Enqueue all unvisited neighbors */
        for (size_t i = 0; i < g->counts[v]; i++) {
            int u = g->adj[v][i];
            if (!visited[u]) {
                visited[u] = 1;
                queue[tail++] = u;
            }
        }
    }

    free(queue);
    free(visited);
    return ZENIT_RESULT_OK;
}

/**
 * @brief Depth-first traversal using an explicit stack.
 *
 * Uses a LIFO array (stack) instead of recursion to avoid stack overflow
 * on deep graphs.  The visit function may stop traversal early.
 */
zenit_result_t zenit_graph_dfs(const zenit_graph_t *g, int start, zenit_graph_visit_fn_t visit, void *ctx) {
    if (g == NULL || visit == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Validate start vertex */
    if (start < 0 || (size_t)start >= g->vertex_capacity || g->removed[start]) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Allocate visited array */
    int *visited = calloc(g->vertex_capacity, sizeof(int));
    if (visited == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    /* Allocate explicit stack (worst case: all vertices) */
    int *stack = malloc(g->vertex_capacity * sizeof(int));
    if (stack == NULL) {
        free(visited);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    size_t sp = 0;

    /* Mark start as visited and push it */
    visited[start] = 1;
    stack[sp++] = start;

    /* DFS loop: pop from stack, visit, push unvisited neighbors */
    while (sp > 0) {
        int v = stack[--sp];

        /* Call the visit callback — stop if it returns non-zero */
        if (visit(v, ctx)) {
            break;
        }

        /* Push all unvisited neighbors (reverse order so first neighbor pops first) */
        for (size_t i = g->counts[v]; i > 0; i--) {
            int u = g->adj[v][i - 1];
            if (!visited[u]) {
                visited[u] = 1;
                stack[sp++] = u;
            }
        }
    }

    free(stack);
    free(visited);
    return ZENIT_RESULT_OK;
}
