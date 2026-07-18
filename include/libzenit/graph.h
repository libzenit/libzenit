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

#ifndef LIBZENIT_GRAPH_H
#define LIBZENIT_GRAPH_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque handle for a graph (adjacency list representation).
 *
 * Supports both directed and undirected graphs. Vertices are identified
 * by non-negative integer IDs assigned on creation. Removed vertices leave
 * tombstones that are not reused — vertex IDs are stable for the lifetime
 * of the graph.
 *
 * Memory for adjacency lists is managed internally using the custom
 * allocator interface. Thread safety is NOT provided — the caller must
 * synchronise access.
 */
typedef struct zenit_graph_t zenit_graph_t;

/**
 * @brief Callback for graph traversals (BFS/DFS).
 *
 * Called once for each reachable vertex. Return non-zero to stop the
 * traversal early (the traversal function returns ZENIT_OK).
 *
 * @param vertex The current vertex ID.
 * @param ctx    Opaque context pointer supplied by the caller.
 * @return 0 to continue, non-zero to stop.
 */
typedef int (*zenit_graph_visit_fn_t)(int vertex, void *ctx);

/**
 * @brief Create an empty graph with the default allocator.
 *
 * @p initial_capacity is the initial vertex slot allocation. If 0, a
 * default capacity of 8 is used.
 *
 * @param initial_capacity Initial number of vertex slots to allocate.
 * @param directed         Non-zero for a directed graph, zero for undirected.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_graph_t* zenit_graph_create(size_t initial_capacity, int directed);

/**
 * @brief Create an empty graph with a custom allocator.
 *
 * @param initial_capacity Initial number of vertex slots to allocate.
 * @param directed         Non-zero for a directed graph, zero for undirected.
 * @param allocator        Custom allocator to use for all internal memory.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_graph_t* zenit_graph_create_with_allocator(size_t initial_capacity, int directed, zenit_allocator_t allocator);

/**
 * @brief Destroy a graph and free all owned memory.
 *
 * Frees every adjacency list, internal arrays, and the handle itself.
 * Passing NULL is safe and is a no-op.
 *
 * @param g Graph handle, or NULL.
 */
void zenit_graph_destroy(zenit_graph_t *g);

/**
 * @brief Add a new vertex to the graph.
 *
 * Assigns the next available vertex ID (sequential, no tombstone reuse).
 * Grows internal arrays by 1.5x if at capacity.
 *
 * @param g Graph handle.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p g is NULL
 *         - ZENIT_ERROR_ALLOC if reallocation fails.
 */
zenit_result_t zenit_graph_add_vertex(zenit_graph_t *g);

/**
 * @brief Remove a vertex and all incident edges.
 *
 * Marks the vertex as a tombstone, removes every edge where the vertex
 * appears as a neighbor in other adjacency lists, and frees its adjacency
 * list memory.
 *
 * @param g      Graph handle.
 * @param vertex Vertex ID to remove.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p g is NULL
 *         - ZENIT_ERROR_PARAM if @p vertex is out of range or already removed.
 */
zenit_result_t zenit_graph_remove_vertex(zenit_graph_t *g, int vertex);

/**
 * @brief Add an edge between two vertices.
 *
 * For undirected graphs, the reverse edge is added as well. If the edge
 * already exists, this is a no-op (returns ZENIT_RESULT_OK).
 *
 * @param g    Graph handle.
 * @param from Source vertex ID.
 * @param to   Target vertex ID.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p g is NULL
 *         - ZENIT_ERROR_PARAM if @p from or @p to is out of range or removed
 *         - ZENIT_ERROR_ALLOC if adjacency list reallocation fails.
 */
zenit_result_t zenit_graph_add_edge(zenit_graph_t *g, int from, int to);

/**
 * @brief Remove an edge between two vertices.
 *
 * For undirected graphs, the reverse edge is removed as well. If the edge
 * does not exist, this is a no-op.
 *
 * @param g    Graph handle.
 * @param from Source vertex ID.
 * @param to   Target vertex ID.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p g is NULL
 *         - ZENIT_ERROR_PARAM if @p from or @p to is out of range or removed.
 */
zenit_result_t zenit_graph_remove_edge(zenit_graph_t *g, int from, int to);

/**
 * @brief Check whether an edge exists between two vertices.
 *
 * @param g    Graph handle.
 * @param from Source vertex ID.
 * @param to   Target vertex ID.
 * @return 1 if the edge exists, 0 if not or if any parameter is invalid.
 */
int zenit_graph_has_edge(const zenit_graph_t *g, int from, int to);

/**
 * @brief Get a malloc'd copy of the adjacency list for a vertex.
 *
 * The caller is responsible for calling free() on the returned array.
 * The number of neighbors is written to @p *out_count, and also returned
 * as the function value (cast to int).
 *
 * @param g            Graph handle.
 * @param vertex       Vertex ID to query.
 * @param out_neighbors On success, set to a malloc'd array of neighbor IDs.
 * @param out_count    On success, set to the number of neighbors.
 * @return The number of neighbors on success, or -1 on error:
 *         - -1 if @p g is NULL, @p vertex is invalid, or allocation fails.
 */
int zenit_graph_get_neighbors(const zenit_graph_t *g, int vertex, int **out_neighbors, size_t *out_count);

/**
 * @brief Return the number of active vertices.
 *
 * @param g Graph handle.
 * @return Vertex count (0 if @p g is NULL).
 */
size_t zenit_graph_vertex_count(const zenit_graph_t *g);

/**
 * @brief Return the number of edges.
 *
 * For undirected graphs, each logical edge counts once.
 *
 * @param g Graph handle.
 * @return Edge count (0 if @p g is NULL).
 */
size_t zenit_graph_edge_count(const zenit_graph_t *g);

/**
 * @brief Check whether the graph is directed.
 *
 * @param g Graph handle.
 * @return 1 if directed, 0 if undirected or @p g is NULL.
 */
int zenit_graph_is_directed(const zenit_graph_t *g);

/**
 * @brief Remove all vertices and edges without destroying the graph.
 *
 * After this call vertex_count and edge_count are 0. Internal buffers may
 * be shrunk to fit. Passing NULL is safe.
 *
 * @param g Graph handle, or NULL.
 */
void zenit_graph_clear(zenit_graph_t *g);

/**
 * @brief Breadth-first traversal starting from @p start.
 *
 * Visits reachable vertices in BFS order, calling @p visit for each.
 * If @p visit returns non-zero, traversal stops immediately and
 * ZENIT_OK is returned.
 *
 * @param g     Graph handle.
 * @param start Starting vertex ID.
 * @param visit Callback invoked for each visited vertex.
 * @param ctx   Opaque context passed to @p visit.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p g or @p visit is NULL
 *         - ZENIT_ERROR_PARAM if @p start is out of range or removed
 *         - ZENIT_ERROR_ALLOC if temporary structures cannot be allocated.
 */
zenit_result_t zenit_graph_bfs(const zenit_graph_t *g, int start, zenit_graph_visit_fn_t visit, void *ctx);

/**
 * @brief Depth-first traversal starting from @p start.
 *
 * Visits reachable vertices in DFS order (explicit stack), calling @p visit
 * for each. If @p visit returns non-zero, traversal stops immediately and
 * ZENIT_OK is returned.
 *
 * @param g     Graph handle.
 * @param start Starting vertex ID.
 * @param visit Callback invoked for each visited vertex.
 * @param ctx   Opaque context passed to @p visit.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p g or @p visit is NULL
 *         - ZENIT_ERROR_PARAM if @p start is out of range or removed
 *         - ZENIT_ERROR_ALLOC if temporary structures cannot be allocated.
 */
zenit_result_t zenit_graph_dfs(const zenit_graph_t *g, int start, zenit_graph_visit_fn_t visit, void *ctx);

#endif
