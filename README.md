<p align="center">
  <img src="media/logo.svg" alt="LibZenit" width="180">
</p>

Portable C library providing building blocks for systems programming: a **typed result type with error codes**, a **ring buffer**, a **finite-state machine engine**, a **fixed-block arena allocator**, a **benchmark framework**, a **dynamic array (vector)**, a **hash map**, a **hash set**, a **doubly linked list**, a **binary heap (priority queue)**, a **double-ended queue (deque)**, a **string builder**, a **bit set**, a **Bloom filter**, a **trie (prefix tree)**, an **LRU cache**, a **graph (adjacency list)**, a **directory API**, **path utilities**, a **custom allocator interface**, a **version API**, a **UUID v4 generator**, a **semantic version parser**, a **structured logger**, a **glob pattern matcher**, a **lock-free SPSC ring buffer**, a **generic optional type**, a **CSV record parser**, and a **thread pool**.

<p align="center">
<a href="https://github.com/libzenit/libzenit/actions/workflows/ci.yml"><img src="https://github.com/libzenit/libzenit/actions/workflows/ci.yml/badge.svg?branch=master" alt="CI Status"></a>
<a href="https://codecov.io/gh/libzenit/libzenit"><img src="https://codecov.io/gh/libzenit/libzenit/graph/badge.svg?token=5L264SDK2C" alt="Coverage"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=alert_status" alt="Quality Gate"></a>
</p>

<p align="center">
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=reliability_rating" alt="Reliability"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=security_rating" alt="Security"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=sqale_rating" alt="Maintainability"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=bugs" alt="Bugs"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=vulnerabilities" alt="Vulnerabilities"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=code_smells" alt="Code Smells"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=sqale_index" alt="Technical Debt"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=duplicated_lines_density" alt="Duplicated Lines"></a>
<a href="https://sonarcloud.io/summary/new_code?id=libzenit_libzenit"><img src="https://sonarcloud.io/api/project_badges/measure?project=libzenit_libzenit&metric=ncloc" alt="Lines of Code"></a>
</p>

---

- **Language:** C (C99/C11 compatible)
- **Build:** CMake >= 3.20
- **License:** [AGPL-3.0-only](LICENSE) — Ian Torres, 2026
- **Library target:** `zenit` (static), exported as `libzenit::zenit`
- **Namespace:** `zenit_` / `libzenit_` in code

---

## Quick Start

```bash
# Configure & build
cmake -B build -D CMAKE_BUILD_TYPE=Debug
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# With coverage
cmake -B build -D CMAKE_BUILD_TYPE=Debug -D LIBZENIT_BUILD_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# Benchmarks (requires LIBZENIT_BUILD_BENCHMARKS=ON)
cmake -B build -D CMAKE_BUILD_TYPE=Release -D LIBZENIT_BUILD_BENCHMARKS=ON
cmake --build build
ctest --test-dir build -L benchmark
```

---

## Modules

### 0. Result Type — `include/libzenit/result.h`

Typed error handling replacing raw `int` (0/-1) returns across all public APIs.

| Type / Function | Description |
|---|---|
| `zenit_error_t` | Enum with codes: `ZENIT_OK`, `ZENIT_ERROR_NULL`, `ZENIT_ERROR_ALLOC`, `ZENIT_ERROR_PARAM`, `ZENIT_ERROR_NOT_FOUND`, `ZENIT_ERROR_CORRUPT`, `ZENIT_ERROR_DOUBLE_FREE`, `ZENIT_ERROR_STATE`, `ZENIT_ERROR_SIZE` |
| `zenit_result_t` | Struct wrapping a `zenit_error_t` — returned by all mutator functions |
| `ZENIT_RESULT_OK` / `ZENIT_RESULT_ERROR(e)` | Macros to construct results inline |
| `zenit_error_string(code)` | Returns a static human-readable string for any error code |

- **Source:** [`src/result.c`](src/result.c)
- **Test:** [`tests/test_result.c`](tests/test_result.c) — all 9 codes, fallback, macro helpers

---

### 1. Version API — `include/libzenit/version.h`

Runtime library version introspection.

| Function | Returns |
|---|---|
| `libzenit_version()` | `libzenit_version_t {major, minor, patch, name}` — currently `{0, 1, 0, "alpha"}` |

- **Source:** [`src/version.c`](src/version.c)
- **Test:** [`tests/test_version.c`](tests/test_version.c)
- **Benchmark:** [`benchmarks/benchmark_version.c`](benchmarks/benchmark_version.c) — 100M iterations

---

### 2. State Machine — `include/libzenit/state.h`

Generic deterministic finite-state machine driven by a transition table. Opaque handle, reentrant by design.

| Function | Description |
|---|---|
| `zenit_state_allocate(table, count, initial)` | Create a state machine; returns `NULL` on allocation failure |
| `zenit_state_allocate_with_allocator(table, count, initial, allocator)` | Create with custom allocator |
| `zenit_state_process_event(state, event, ctx)` | Feed an event; returns `zenit_result_t` (`ZENIT_OK` / `ZENIT_ERROR_NOT_FOUND`) |
| `zenit_state_current(state)` | Read current state (no side-effect) |
| `zenit_state_reset(state, new_state)` | Reset to a given state without processing an event |
| `zenit_state_deallocate(state)` | Free all memory; NULL-safe |

**Key types:** `zenit_state_transition_t` (from, event, to, callback), `zenit_state_callback_t`

- **Source:** [`src/state.c`](src/state.c)
- **Tests:** [`tests/test_state.c`](tests/test_state.c) (happy path, edge cases, callback), [`tests/test_state_malloc_fail.c`](tests/test_state_malloc_fail.c) (allocation failure via `--wrap=malloc`)
- **Benchmarks:** [`benchmarks/benchmark_state.c`](benchmarks/benchmark_state.c) — sequential 8-state, sequential 1024-state, miss

---

### 3. Arena Allocator — `include/libzenit/arena.h`

Fixed-block memory arena with sub-allocation, free-list coalescing, and corruption detection.

**Two-tier architecture:**
1. `zenit_arena_t` — owns a fixed pool of equal-sized blocks (bitmap-tracked)
2. `zenit_usable_arena_t` — a contiguous region acquired from the arena, supporting sub-allocations
3. `zenit_usable_buffer_t` — a sub-allocated chunk within a usable arena

| Function | Description |
|---|---|
| `zenit_arena_create(total_size, block_size)` | Create arena; returns `NULL` on invalid params or OOM |
| `zenit_arena_destroy(arena)` | Free all memory; NULL-safe |
| `zenit_arena_acquire(arena, size)` | Acquire contiguous region; returns `NULL` on OOM or fragmentation |
| `zenit_arena_release(arena, ua)` | Release region back; returns `zenit_result_t` (`ZENIT_OK` / `ZENIT_ERROR_NULL` / `ZENIT_ERROR_STATE` / `ZENIT_ERROR_CORRUPT`) |
| `zenit_usable_arena_allocate(ua, size)` | Sub-allocate a buffer; `.data == NULL` on OOM |
| `zenit_usable_buffer_free(buf)` | Free buffer; returns `zenit_result_t` (`ZENIT_OK` / `ZENIT_ERROR_NULL` / `ZENIT_ERROR_DOUBLE_FREE`) |
| `zenit_usable_buffer_data(buf)` / `zenit_usable_buffer_size(buf)` | Inspect buffer |

- **Source:** [`src/arena.c`](src/arena.c)
- **Tests:** [`tests/test_arena.c`](tests/test_arena.c) (16 sub-tests: create/destroy, acquire/release, buffer alloc/free, double-free, coalescing, fragmentation, corruption), [`tests/test_arena_malloc_fail.c`](tests/test_arena_malloc_fail.c) (4 sub-tests covering malloc/calloc failure paths)
- **Benchmarks:** [`benchmarks/benchmark_arena.c`](benchmarks/benchmark_arena.c) — create/destroy, acquire/release, alloc/free at 8B/64B/4KB, vs malloc baseline

---

### 4. Benchmark Framework — `include/libzenit/benchmark.h`

Minimal benchmarking harness used by all project benchmarks. Uses `clock_gettime` (POSIX) or `QueryPerformanceCounter` (Windows).

| Function | Description |
|---|---|
| `zenit_bench_run(name, fn, ctx, iterations)` | Run warm-up + timed loop; returns `zenit_bench_result_t` |
| `zenit_bench_print(result)` | Print aligned result row to stdout |

- **Source:** [`src/benchmark.c`](src/benchmark.c)
- **Test:** [`tests/test_benchmark.c`](tests/test_benchmark.c) — validates elapsed time, iteration count, ops/sec, and that fn is called iterations+1 times

---

### 5. Ring Buffer — `include/libzenit/ring.h`

Fixed-capacity byte-level circular FIFO buffer with wrap-around support. Push fails on full, pop fails on empty — no overwrite.

| Function | Description |
|---|---|
| `zenit_ring_create(capacity)` | Create a ring buffer; returns `NULL` on OOM or zero capacity |
| `zenit_ring_destroy(ring)` | Free all memory; NULL-safe |
| `zenit_ring_push(ring, data, size)` | Push bytes; returns `zenit_result_t` (`ZENIT_OK` / `ZENIT_ERROR_NULL` / `ZENIT_ERROR_PARAM` / `ZENIT_ERROR_FULL`) |
| `zenit_ring_pop(ring, data, size)` | Pop oldest bytes; returns `zenit_result_t` (`ZENIT_OK` / `ZENIT_ERROR_NULL` / `ZENIT_ERROR_PARAM` / `ZENIT_ERROR_EMPTY`) |
| `zenit_ring_peek(ring, data, size)` | Read without consuming; same returns as pop |
| `zenit_ring_count(ring)` | Bytes currently stored (0 if NULL) |
| `zenit_ring_capacity(ring)` | Maximum bytes (0 if NULL) |
| `zenit_ring_clear(ring)` | Reset without freeing; NULL-safe |
| `zenit_ring_reserve(ring, new_capacity)` | Resize capacity, preserving existing data |
| `zenit_ring_shrink_to_fit(ring)` | Shrink buffer to exactly fit current data |

- **Source:** [`src/ring.c`](src/ring.c)
- **Tests:** [`tests/test_ring.c`](tests/test_ring.c) (10 sub-tests: create/destroy, push/pop, full, empty, peek, wrap-around, clear, edge cases, chunks), [`tests/test_ring_malloc_fail.c`](tests/test_ring_malloc_fail.c) (allocation failure via shared `test_malloc_fail.h`)
- **Benchmark:** [`benchmarks/benchmark_ring.c`](benchmarks/benchmark_ring.c) — sequential push/pop 128B, 1KB, full-miss

---

### 6. Dynamic Array (Vector) — `include/libzenit/vector.h`

Generic type-erased dynamic array with 1.5x exponential growth. Elements are stored contiguously and accessed by index.

| Function | Description |
|---|---|
| `zenit_vector_create(elem_size)` | Create empty vector (default capacity 8); returns `NULL` on zero elem_size or OOM |
| `zenit_vector_create_with_capacity(elem_size, capacity)` | Create with pre-allocated buffer; returns `NULL` on invalid params or OOM |
| `zenit_vector_destroy(vector)` | Free all memory; NULL-safe |
| `zenit_vector_push(vector, elem)` | Append element; grows if full; returns `ZENIT_RESULT_OK` / `ZENIT_ERROR_NULL` / `ZENIT_ERROR_ALLOC` |
| `zenit_vector_pop(vector, out_elem)` | Remove and retrieve last element; returns `ZENIT_ERROR_EMPTY` if empty |
| `zenit_vector_insert(vector, index, elem)` | Insert at position, shifting right; returns `ZENIT_ERROR_PARAM` if index > count |
| `zenit_vector_remove(vector, index, out_elem)` | Remove at position, shifting left; returns `ZENIT_ERROR_PARAM` if index >= count |
| `zenit_vector_get(vector, index)` | Get pointer to element; returns `NULL` if out of bounds |
| `zenit_vector_set(vector, index, elem)` | Overwrite element at index; returns `ZENIT_ERROR_PARAM` if OOB |
| `zenit_vector_count(vector)` | Number of elements (0 if NULL) |
| `zenit_vector_capacity(vector)` | Allocated element slots (0 if NULL) |
| `zenit_vector_reserve(vector, capacity)` | Pre-allocate capacity; no-op if already sufficient |
| `zenit_vector_shrink_to_fit(vector)` | Reallocate to exact element count |
| `zenit_vector_clear(vector)` | Reset count to 0 without freeing; NULL-safe |
| `zenit_vector_empty(vector)` | 1 if empty or NULL, 0 otherwise |

- **Source:** [`src/vector.c`](src/vector.c)
- **Tests:** [`tests/test_vector.c`](tests/test_vector.c) (20 sub-tests: create/destroy, push/pop, insert/remove, reserve/shrink, struct, edge cases, many elements), [`tests/test_vector_malloc_fail.c`](tests/test_vector_malloc_fail.c) (6 sub-tests covering malloc/calloc/realloc failure via `--wrap`)
- **Benchmarks:** [`benchmarks/benchmark_vector.c`](benchmarks/benchmark_vector.c) — sequential push (1M), push/pop, insert front (10K), reserve+push (1M)

---

### 7. Hash Map — `include/libzenit/map.h`

Generic type-erased hash map with open-addressing and linear probing. Uses FNV-1a hashing with power-of-2 capacity. Automatically rehashes at 75% load factor.

| Function | Description |
|---|---|
| `zenit_map_create(key_size, value_size)` | Create empty map (default capacity 16); returns `NULL` on zero sizes or OOM |
| `zenit_map_create_with_capacity(key_size, value_size, capacity)` | Create with specific initial capacity (rounded to power of two); returns `NULL` on invalid params or OOM |
| `zenit_map_destroy(map)` | Free all memory; NULL-safe |
| `zenit_map_insert(map, key, value)` | Insert or overwrite; returns `ZENIT_ERROR_NULL` / `ZENIT_ERROR_ALLOC` |
| `zenit_map_get(map, key, out_value)` | Retrieve value; returns `ZENIT_ERROR_NOT_FOUND` if missing |
| `zenit_map_remove(map, key)` | Remove key (tombstone); returns `ZENIT_ERROR_NOT_FOUND` if missing |
| `zenit_map_contains(map, key)` | 1 if present, 0 otherwise |
| `zenit_map_count(map)` | Number of entries (0 if NULL) |
| `zenit_map_capacity(map)` | Slot capacity, always power of two (0 if NULL) |
| `zenit_map_clear(map)` | Remove all entries without shrinking; NULL-safe |
| `zenit_map_foreach(map, visit, ctx)` | Iterate all entries in unspecified order |

- **Source:** [`src/map.c`](src/map.c)
- **Tests:** [`tests/test_map.c`](tests/test_map.c) (34 sub-tests: create/destroy, insert/get, overwrite, remove, tombstone, contains, clear, foreach, rehash, struct keys, string keys, all NULL edge cases), [`tests/test_map_malloc_fail.c`](tests/test_map_malloc_fail.c) (4 sub-tests covering malloc/calloc failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_map.c`](benchmarks/benchmark_map.c) — insert (100K), get hit/miss (100K), insert rehash (100K), foreach (100K×1K)

---

### 8. Hash Set — `include/libzenit/set.h`

Generic type-erased hash set with open-addressing and linear probing. Uses FNV-1a hashing with power-of-2 capacity. Automatically rehashes at 75% load factor. Keys-only counterpart to the hash map.

| Function | Description |
|---|---|
| `zenit_set_create(key_size)` | Create empty set (default capacity 16); returns `NULL` on zero key_size or OOM |
| `zenit_set_create_with_capacity(key_size, capacity)` | Create with specific initial capacity (rounded to power of two); returns `NULL` on invalid params or OOM |
| `zenit_set_destroy(set)` | Free all memory; NULL-safe |
| `zenit_set_insert(set, key)` | Insert a key (no-op if already present); returns `ZENIT_ERROR_NULL` / `ZENIT_ERROR_ALLOC` |
| `zenit_set_remove(set, key)` | Remove key (tombstone); returns `ZENIT_ERROR_NOT_FOUND` if missing |
| `zenit_set_contains(set, key)` | 1 if present, 0 otherwise |
| `zenit_set_count(set)` | Number of entries (0 if NULL) |
| `zenit_set_capacity(set)` | Slot capacity, always power of two (0 if NULL) |
| `zenit_set_clear(set)` | Remove all entries without shrinking; NULL-safe |
| `zenit_set_foreach(set, visit, ctx)` | Iterate all keys in unspecified order |

- **Source:** [`src/set.c`](src/set.c)
- **Tests:** [`tests/test_set.c`](tests/test_set.c) (28 sub-tests: create/destroy, insert/contains, duplicate, remove, tombstone, clear, foreach, rehash, struct keys, all NULL edge cases), [`tests/test_set_malloc_fail.c`](tests/test_set_malloc_fail.c) (5 sub-tests covering malloc/calloc failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_set.c`](benchmarks/benchmark_set.c) — insert (100K), contains hit/miss (100K), insert rehash (100K), foreach (100K×1K)

---

### 9. Doubly Linked List — `include/libzenit/list.h`

Generic type-erased doubly linked list with O(1) push/pop at both ends and O(n) indexed access.

| Function | Description |
|---|---|
| `zenit_list_create(elem_size)` | Create empty list; returns `NULL` on zero elem_size or OOM |
| `zenit_list_destroy(list)` | Free all nodes and handle; NULL-safe |
| `zenit_list_push_front(list, elem)` | Prepend element (O(1)); returns `ZENIT_ERROR_NULL` / `ZENIT_ERROR_ALLOC` |
| `zenit_list_push_back(list, elem)` | Append element (O(1)); same errors |
| `zenit_list_pop_front(list, out)` | Remove and retrieve first (O(1)); returns `ZENIT_ERROR_EMPTY` if empty |
| `zenit_list_pop_back(list, out)` | Remove and retrieve last (O(1)); same |
| `zenit_list_insert(list, index, elem)` | Insert at position; returns `ZENIT_ERROR_PARAM` if index > count |
| `zenit_list_remove(list, index, out)` | Remove at position; returns `ZENIT_ERROR_PARAM` if OOB |
| `zenit_list_get(list, index)` | Pointer to element (O(n)); NULL if OOB |
| `zenit_list_set(list, index, elem)` | Overwrite at index; returns `ZENIT_ERROR_PARAM` if OOB |
| `zenit_list_count(list)` | Number of elements (0 if NULL) |
| `zenit_list_empty(list)` | 1 if empty or NULL |
| `zenit_list_clear(list)` | Remove all elements; NULL-safe |
| `zenit_list_foreach(list, visit, ctx)` | Iterate all elements in order |
| `zenit_list_front(list)` / `zenit_list_back(list)` | Pointer to first/last element (NULL if empty) |

- **Source:** [`src/list.c`](src/list.c)
- **Tests:** [`tests/test_list.c`](tests/test_list.c) (24 sub-tests: create/destroy, push/pop front/back, insert/remove, get/set, clear, foreach, front/back, many elements, struct, all NULL edge cases), [`tests/test_list_malloc_fail.c`](tests/test_list_malloc_fail.c) (4 sub-tests covering malloc/calloc failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_list.c`](benchmarks/benchmark_list.c) — push_back (100K), push_front (100K), push_pop (100K), foreach (100K)

---

### 10. Binary Heap / Priority Queue — `include/libzenit/heap.h`

Generic binary heap with user-provided comparator (max-heap or min-heap). Uses 1.5x exponential growth.

| Function | Description |
|---|---|
| `zenit_heap_create(elem_size, compare)` | Create empty heap (default cap 8); returns `NULL` on zero elem_size, NULL compare, or OOM |
| `zenit_heap_create_with_capacity(elem_size, compare, capacity)` | Create with initial capacity; returns `NULL` on invalid params or OOM |
| `zenit_heap_destroy(heap)` | Free all memory; NULL-safe |
| `zenit_heap_push(heap, elem)` | Insert element (O(log n)); returns `ZENIT_ERROR_ALLOC` on grow failure |
| `zenit_heap_pop(heap, out)` | Remove root (O(log n)); returns `ZENIT_ERROR_EMPTY` if empty |
| `zenit_heap_peek(heap)` | Pointer to root without removal (NULL if empty) |
| `zenit_heap_count(heap)` | Number of elements (0 if NULL) |
| `zenit_heap_capacity(heap)` | Current slot capacity (0 if NULL) |
| `zenit_heap_empty(heap)` | 1 if empty or NULL |
| `zenit_heap_clear(heap)` | Remove all without freeing buffer; NULL-safe |
| `zenit_heap_reserve(heap, capacity)` | Pre-allocate capacity; no-op if sufficient |

- **Source:** [`src/heap.c`](src/heap.c)
- **Tests:** [`tests/test_heap.c`](tests/test_heap.c) (13 sub-tests: create/destroy, push/pop max/min, peek, pop empty, NULL params, reserve, clear, many elements, struct, all query NULL), [`tests/test_heap_malloc_fail.c`](tests/test_heap_malloc_fail.c) (3 sub-tests covering malloc/calloc/realloc failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_heap.c`](benchmarks/benchmark_heap.c) — push (100K), push_pop (100K), peek (100K)

---

### 11. Double-Ended Queue (Deque) — `include/libzenit/deque.h`

Generic type-erased deque with a contiguous circular buffer. Amortized O(1) push/pop at both ends. Grows by 1.5x.

| Function | Description |
|---|---|
| `zenit_deque_create(elem_size)` | Create empty deque (default cap 8); returns `NULL` on zero elem_size or OOM |
| `zenit_deque_create_with_capacity(elem_size, capacity)` | Create with initial capacity; returns `NULL` on invalid params or OOM |
| `zenit_deque_destroy(deque)` | Free all memory; NULL-safe |
| `zenit_deque_push_front(deque, elem)` | Prepend element; returns `ZENIT_ERROR_ALLOC` on grow failure |
| `zenit_deque_push_back(deque, elem)` | Append element; same |
| `zenit_deque_pop_front(deque, out)` | Remove first; returns `ZENIT_ERROR_EMPTY` if empty |
| `zenit_deque_pop_back(deque, out)` | Remove last; same |
| `zenit_deque_get(deque, index)` | Pointer to element at index (O(1)); NULL if OOB |
| `zenit_deque_front(deque)` / `zenit_deque_back(deque)` | Pointer to first/last element (NULL if empty) |
| `zenit_deque_count(deque)` | Number of elements (0 if NULL) |
| `zenit_deque_capacity(deque)` | Current capacity (0 if NULL) |
| `zenit_deque_empty(deque)` | 1 if empty or NULL |
| `zenit_deque_reserve(deque, capacity)` | Pre-allocate; no-op if sufficient |
| `zenit_deque_shrink_to_fit(deque)` | Reallocate to exact element count |
| `zenit_deque_clear(deque)` | Remove all without freeing; NULL-safe |

- **Source:** [`src/deque.c`](src/deque.c)
- **Tests:** [`tests/test_deque.c`](tests/test_deque.c) (19 sub-tests: create/destroy, push/pop all four combinations, empty, NULL params, get, get with wrap, front/back, reserve, shrink, clear, many elements, mixed push/pop, struct, all query NULL), [`tests/test_deque_malloc_fail.c`](tests/test_deque_malloc_fail.c) (4 sub-tests covering malloc/calloc/realloc failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_deque.c`](benchmarks/benchmark_deque.c) — push_back (1M), push_front (1M), push_pop (1M)

---

### 12. String Builder — `include/libzenit/string.h`

Dynamic string with null-terminated C string access. Internally backed by a `zenit_vector_t` — grows by 1.5x on demand. Supports custom allocators.

| Function | Description |
|---|---|
| `zenit_string_create()` | Create empty string; returns `NULL` on OOM |
| `zenit_string_create_from(cstr)` | Create from C string (may be NULL); returns `NULL` on OOM |
| `zenit_string_destroy(s)` | Free all memory; NULL-safe |
| `zenit_string_append(s, data, len)` | Append raw bytes |
| `zenit_string_append_cstr(s, cstr)` | Append null-terminated C string |
| `zenit_string_cstr(s)` | Get null-terminated C string (read-only) |
| `zenit_string_length(s)` | Length excluding null terminator (0 if NULL) |
| `zenit_string_capacity(s)` | Total allocated bytes (0 if NULL) |
| `zenit_string_clear(s)` | Reset to empty; NULL-safe |
| `zenit_string_reserve(s, cap)` | Pre-allocate capacity |
| `zenit_string_shrink_to_fit(s)` | Reallocate to exact length |
| `zenit_string_empty(s)` | 1 if empty or NULL |

- **Source:** [`src/string.c`](src/string.c)
- **Tests:** [`tests/test_string.c`](tests/test_string.c) (19 sub-tests: create/destroy, create_from, append, append_cstr, cstr, length, capacity, clear, reserve, shrink, empty, many appends, large append, NULL edge cases), [`tests/test_string_malloc_fail.c`](tests/test_string_malloc_fail.c) (3 sub-tests covering malloc/calloc/realloc failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_string.c`](benchmarks/benchmark_string.c) — append_cstr 8B (100K), append 64B (100K)

---

### 13. Bit Set — `include/libzenit/bitset.h`

Dynamically-sized bit array with automatic growth. Uses a contiguous byte array with popcount for counting. Supports custom allocators.

| Function | Description |
|---|---|
| `zenit_bitset_create(num_bits)` | Create with initial capacity; returns `NULL` on OOM |
| `zenit_bitset_destroy(bs)` | Free all memory; NULL-safe |
| `zenit_bitset_set(bs, pos)` | Set bit to 1; auto-grows if needed |
| `zenit_bitset_clear(bs, pos)` | Set bit to 0 |
| `zenit_bitset_toggle(bs, pos)` | Flip bit |
| `zenit_bitset_test(bs, pos)` | 1 if set, 0 if clear or out of range |
| `zenit_bitset_set_all(bs)` | Set all bits to 1 |
| `zenit_bitset_clear_all(bs)` | Clear all bits to 0 |
| `zenit_bitset_count(bs)` | Number of 1-bits (popcount) |
| `zenit_bitset_capacity(bs)` | Total bit capacity (0 if NULL) |
| `zenit_bitset_resize(bs, num_bits)` | Grow or shrink the bit array |

- **Source:** [`src/bitset.c`](src/bitset.c)
- **Tests:** [`tests/test_bitset.c`](tests/test_bitset.c) (19 sub-tests: create/destroy, set/test, set/clear, toggle, set_all, clear_all, count, resize, auto-grow, NULL params)
- **Benchmark:** [`benchmarks/benchmark_bitset.c`](benchmarks/benchmark_bitset.c) — set (100K), test (100K), count (100K)

---

### 14. JSON — `include/libzenit/json.h`

Recursive-descent JSON parser and serializer with DOM-style value tree, custom allocator support, and full UTF-8 handling.

| Function | Description |
|---|---|
| `zenit_json_parse(text)` / `_with_length` / `_with_allocator` | Parse JSON string into a document; returns NULL on parse error or OOM |
| `zenit_json_create()` / `_with_allocator` | Create an empty document |
| `zenit_json_destroy(json)` | Free all values and the document; NULL-safe |
| `zenit_json_root(json)` / `zenit_json_set_root(json, val)` | Get/set the root value |
| `zenit_json_value_type(val)` | Query type tag (NULL/BOOL/NUMBER/STRING/ARRAY/OBJECT) |
| `zenit_json_value_is_null(val)` / `_get_bool` / `_get_number` / `_get_string` | Read typed payloads (safe on mismatch — return 0/NULL) |
| `zenit_json_value_null(json)` / `_bool` / `_number` / `_string` / `_array` / `_object` | Construct values within a document |
| `zenit_json_array_count` / `_get` / `_append` / `_remove` / `_insert` | Array manipulation |
| `zenit_json_object_count` / `_key` / `_value_at` / `_get` / `_set` / `_remove` | Object manipulation |
| `zenit_json_serialize(json)` / `zenit_json_serialize_with_allocator(json, allocator)` | Serialise document to JSON string (caller frees) |
| `zenit_json_value_serialize(val)` / `zenit_json_value_serialize_with_allocator(val, allocator)` | Serialise value subtree to JSON string (caller frees) |

**Number formatting:** Uses shortest-round-trip representation — formats with `%.17g` then progressively shortens while ensuring `strtod` reproduces the same double.

- **Source:** [`src/json.c`](src/json.c)
- **Tests:** [`tests/test_json.c`](tests/test_json.c) (39 sub-tests: all types, nesting, escapes, round-trip, errors, NULL safety, construction, array/object ops, mismatched getters, allocator variants), [`tests/test_json_malloc_fail.c`](tests/test_json_malloc_fail.c) (allocation failure via `--wrap=malloc/calloc/realloc`)
- **Benchmark:** [`benchmarks/benchmark_json.c`](benchmarks/benchmark_json.c) — parse (50K), serialize (50K), build (50K)

---

### 15. Base64 — `include/libzenit/base64.h`

Standard Base64 encoding/decoding (RFC 4648) with padding.

| Function | Description |
|---|---|
| Function | Description |
|---|---|---|
| `zenit_base64_encode(data, len)` | Encode raw bytes into a Base64 string (default allocator) |
| `zenit_base64_encode_with_allocator(data, len, allocator)` | Encode with a custom allocator |
| `zenit_base64_decode(encoded, out_len)` | Decode a Base64 string into raw bytes (default allocator) |
| `zenit_base64_decode_with_allocator(encoded, out_len, allocator)` | Decode with a custom allocator |

- **Source:** [`src/base64.c`](src/base64.c)
- **Tests:** [`tests/test_base64.c`](tests/test_base64.c) (9 sub-tests: encode/decode, padding, binary round-trip, NULL params, invalid input), [`tests/test_base64_malloc_fail.c`](tests/test_base64_malloc_fail.c) (allocation failure via `--wrap=malloc`)
- **Benchmark:** [`benchmarks/benchmark_base64.c`](benchmarks/benchmark_base64.c) — encode 256B (100K), decode 256B (100K)

---

### 16. Hex — `include/libzenit/hex.h`

Lowercase hexadecimal encoding/decoding. Accepts uppercase hex on decode. Handles odd-length input by treating it as if a leading `0` were present.

| Function | Description |
|---|---|
| Function | Description |
|---|---|---|
| `zenit_hex_encode(data, len)` | Encode raw bytes into a lowercase hex string (default allocator) |
| `zenit_hex_encode_with_allocator(data, len, allocator)` | Encode with a custom allocator |
| `zenit_hex_decode(hex, out_len)` | Decode a hex string into raw bytes (default allocator) |
| `zenit_hex_decode_with_allocator(hex, out_len, allocator)` | Decode with a custom allocator |

- **Source:** [`src/hex.c`](src/hex.c)
- **Tests:** [`tests/test_hex.c`](tests/test_hex.c) (10 sub-tests: encode/decode, odd-length, uppercase, binary round-trip, invalid input, NULL params), [`tests/test_hex_malloc_fail.c`](tests/test_hex_malloc_fail.c) (allocation failure via `--wrap=malloc`)
- **Benchmark:** [`benchmarks/benchmark_hex.c`](benchmarks/benchmark_hex.c) — encode 256B (100K), decode 256B (100K)

---

### 17. URI Percent-Encoding — `include/libzenit/uri.h`

Percent-encoding for URI components per RFC 3986. Unreserved characters (A-Z, a-z, 0-9, `-`, `_`, `.`, `~`) pass through; everything else is `%XX` encoded. Decodes `+` as space.

| Function | Description |
|---|---|
| Function | Description |
|---|---|---|
| `zenit_uri_encode(input)` | Percent-encode a string (default allocator) |
| `zenit_uri_encode_with_allocator(input, allocator)` | Encode with a custom allocator |
| `zenit_uri_decode(encoded)` | Decode a percent-encoded URI string (default allocator) |
| `zenit_uri_decode_with_allocator(encoded, allocator)` | Decode with a custom allocator |

- **Source:** [`src/uri.c`](src/uri.c)
- **Tests:** [`tests/test_uri.c`](tests/test_uri.c) (11 sub-tests: encode unreserved/reserved/empty/NULL, decode basic/plus/empty/invalid/NULL, round-trip), [`tests/test_uri_malloc_fail.c`](tests/test_uri_malloc_fail.c) (allocation failure via `--wrap=malloc`)
- **Benchmark:** [`benchmarks/benchmark_uri.c`](benchmarks/benchmark_uri.c) — encode 256B (100K), decode 256B (100K)

---

### 18. String Utilities — `include/libzenit/str.h`

Common string manipulation helpers — trim, split, and join. All functions return newly allocated memory (caller frees).

| Function | Description |
|---|---|
| `zenit_str_trim(s)` | Remove leading and trailing whitespace (default allocator) |
| `zenit_str_trim_with_allocator(s, allocator)` | Trim with a custom allocator |
| `zenit_str_split(s, delim, out_count)` | Split into substrings (default allocator) |
| `zenit_str_split_with_allocator(s, delim, out_count, allocator)` | Split with a custom allocator |
| `zenit_str_split_free(result, count, allocator)` | Free a split result with the correct allocator |
| `zenit_str_join(parts, count, delim)` | Join an array of strings (default allocator) |
| `zenit_str_join_with_allocator(parts, count, delim, allocator)` | Join with a custom allocator |

- **Source:** [`src/str.c`](src/str.c)
- **Tests:** [`tests/test_str.c`](tests/test_str.c) (18 sub-tests: trim basic/left/right/none/all/empty/NULL, split basic/consecutive/empty/no-delim/NULL, join basic/empty-delim/zero-count/one-part/NULL), [`tests/test_str_malloc_fail.c`](tests/test_str_malloc_fail.c) (allocation failure via `--wrap=malloc/calloc/realloc`)
- **Benchmark:** [`benchmarks/benchmark_str.c`](benchmarks/benchmark_str.c) — trim/split/join throughput

---

### 19. Sort & Binary Search — `include/libzenit/sort.h`

In-place quicksort (median-of-three pivot) and binary search over sorted arrays. Uses a caller-provided comparator, matching the same signature as the heap module.

| Function | Description |
|---|---|
| `zenit_sort_quick(base, count, elem_size, compare)` | In-place quicksort (not stable) |
| `zenit_binary_search(key, base, count, elem_size, compare)` | Binary search on sorted array; returns pointer or NULL |

- **Source:** [`src/sort.c`](src/sort.c)
- **Tests:** [`tests/test_sort.c`](tests/test_sort.c) (13 sub-tests: sort already-sorted/reverse/random/single/empty/doubles/duplicates/large-elements, binary search found/not-found/first/last/empty/NULL)
- **Benchmark:** [`benchmarks/benchmark_sort.c`](benchmarks/benchmark_sort.c) — sort random 10K, sort sorted 10K, binary search hit (1M), binary search miss (1M)

---

### 20. Stack — `include/libzenit/stack.h`

LIFO stack wrapper over the dynamic array (vector). Push and pop at the top are O(1) amortized.

| Function | Description |
|---|---|
| `zenit_stack_create(elem_size)` | Create empty stack (default capacity 8); returns NULL on invalid param or OOM |
| `zenit_stack_create_with_allocator(elem_size, allocator)` | Create with a custom allocator |
| `zenit_stack_destroy(stack)` | Free all memory; NULL-safe |
| `zenit_stack_push(stack, elem)` | Push onto top; grows if full; returns `ZENIT_ERROR_ALLOC` on failure |
| `zenit_stack_pop(stack, out)` | Pop from top; returns `ZENIT_ERROR_EMPTY` if empty |
| `zenit_stack_peek(stack)` | Pointer to top element (NULL if empty) |
| `zenit_stack_count(stack)` | Number of elements (0 if NULL) |
| `zenit_stack_empty(stack)` | 1 if empty or NULL |
| `zenit_stack_clear(stack)` | Remove all without freeing buffer; NULL-safe |

- **Source:** [`src/stack.c`](src/stack.c)
- **Tests:** [`tests/test_stack.c`](tests/test_stack.c) (11 sub-tests: create/destroy, push/pop, peek, clear, NULL params, many elements, struct, allocator), [`tests/test_stack_malloc_fail.c`](tests/test_stack_malloc_fail.c) (allocation failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_stack.c`](benchmarks/benchmark_stack.c) — push (1M), push_pop (1M), peek (1M)

---

### 21. Queue — `include/libzenit/queue.h`

FIFO queue wrapper over the double-ended queue (deque). Enqueue and dequeue are O(1) amortized.

| Function | Description |
|---|---|
| `zenit_queue_create(elem_size)` | Create empty queue (default capacity 8); returns NULL on invalid param or OOM |
| `zenit_queue_create_with_allocator(elem_size, allocator)` | Create with a custom allocator |
| `zenit_queue_destroy(queue)` | Free all memory; NULL-safe |
| `zenit_queue_enqueue(queue, elem)` | Append to back; grows if full; returns `ZENIT_ERROR_ALLOC` on failure |
| `zenit_queue_dequeue(queue, out)` | Remove from front; returns `ZENIT_ERROR_EMPTY` if empty |
| `zenit_queue_peek(queue)` | Pointer to front element (NULL if empty) |
| `zenit_queue_count(queue)` | Number of elements (0 if NULL) |
| `zenit_queue_empty(queue)` | 1 if empty or NULL |
| `zenit_queue_clear(queue)` | Remove all without freeing buffer; NULL-safe |

- **Source:** [`src/queue.c`](src/queue.c)
- **Tests:** [`tests/test_queue.c`](tests/test_queue.c) (12 sub-tests: create/destroy, enqueue/dequeue, peek, clear, NULL params, many elements, mixed ordering, struct, allocator), [`tests/test_queue_malloc_fail.c`](tests/test_queue_malloc_fail.c) (allocation failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_queue.c`](benchmarks/benchmark_queue.c) — enqueue (1M), enqueue_dequeue (1M), peek (1M)

---

### 22. Timer / Stopwatch — `include/libzenit/timer.h`

High-resolution timer value type using `clock_gettime` (POSIX) or `QueryPerformanceCounter` (Windows). No heap allocation — all functions operate on stack-allocated `zenit_time_t` values.

| Function | Description |
|---|---|
| `zenit_time_now()` | Capture current monotonic time point |
| `zenit_time_elapsed_s(start, end)` | Elapsed wall-clock time in seconds |
| `zenit_time_elapsed_ms(start, end)` | Elapsed time in milliseconds |
| `zenit_time_elapsed_us(start, end)` | Elapsed time in microseconds |
| `zenit_time_elapsed_ns(start, end)` | Elapsed time in nanoseconds |
| `zenit_time_add(a, b)` | Add two time points with nanosecond carry |
| `zenit_time_sub(a, b)` | Subtract with nanosecond borrow |
| `zenit_time_cmp(a, b)` | Compare: negative if a < b, zero if equal, positive if a > b |

- **Source:** [`src/timer.c`](src/timer.c)
- **Test:** [`tests/test_timer.c`](tests/test_timer.c) — now, elapsed, add, sub, cmp edge cases (no heap allocation — no malloc-fail test)
- **Benchmark:** [`benchmarks/benchmark_timer.c`](benchmarks/benchmark_timer.c) — now (10M), elapsed_ns (10M)

---

### 23. Object Pool — `include/libzenit/pool.h`

Fixed-capacity object pool with O(1) acquire/release. Pre-allocates a contiguous block of objects at creation; no heap allocation occurs after construction.

| Function | Description |
|---|---|
| `zenit_pool_create(object_size, capacity)` | Create a pool; returns NULL on invalid params or OOM |
| `zenit_pool_create_with_allocator(object_size, capacity, allocator)` | Create with a custom allocator |
| `zenit_pool_destroy(pool)` | Free all memory; NULL-safe |
| `zenit_pool_acquire(pool)` | Get a pre-allocated object; returns NULL if exhausted |
| `zenit_pool_release(pool, obj)` | Return object; rejects double-free and foreign pointers |
| `zenit_pool_count(pool)` | Number of acquired objects (0 if NULL) |
| `zenit_pool_capacity(pool)` | Maximum objects the pool can hold (0 if NULL) |
| `zenit_pool_available(pool)` | Number of objects available for acquisition (0 if NULL) |
| `zenit_pool_clear(pool)` | Release all objects; NULL-safe |

- **Source:** [`src/pool.c`](src/pool.c)
- **Tests:** [`tests/test_pool.c`](tests/test_pool.c) (13 sub-tests: create/destroy, acquire/release, empty, double-free, foreign pointer, misaligned, clear, struct, allocator, consistency), [`tests/test_pool_malloc_fail.c`](tests/test_pool_malloc_fail.c) (allocation failure via `--wrap`)
- **Benchmark:** [`benchmarks/benchmark_pool.c`](benchmarks/benchmark_pool.c) — acquire (1M), acquire_release (1M), small pool (1M)

---

### 24. File I/O — `include/libzenit/io.h`

Portable file operations with a unified API across POSIX and Windows. Supports chunked copy for large files.

| Function | Description |
|---|---|
| `zenit_file_read(path, out_data, out_len)` | Read entire file into allocated buffer (default allocator) |
| `zenit_file_read_with_allocator(path, out_data, out_len, allocator)` | Read with a custom allocator |
| `zenit_file_write(path, data, len)` | Write data, creating or truncating the file |
| `zenit_file_append(path, data, len)` | Append data, creating the file if needed |
| `zenit_file_exists(path)` | 1 if the file exists, 0 otherwise |
| `zenit_file_delete(path)` | Delete a file |
| `zenit_file_size(path, out_size)` | Get file size in bytes |
| `zenit_file_copy(src, dst)` | Copy file (chunked, no full load into memory) |
| `zenit_file_read_line(path, out_line)` | Read first line into allocated buffer (default allocator) |
| `zenit_file_read_line_with_allocator(path, out_line, allocator)` | Read first line with a custom allocator |

- **Source:** [`src/io.c`](src/io.c)
- **Tests:** [`tests/test_io.c`](tests/test_io.c) (9 sub-tests: write/read, allocator variant, append, exists, delete, size, copy, NULL params, nonexistent), [`tests/test_io_malloc_fail.c`](tests/test_io_malloc_fail.c) (allocation failure via `--wrap=malloc`)
- **Benchmark:** [`benchmarks/benchmark_io.c`](benchmarks/benchmark_io.c) — write 1KB (100K), read 1KB (100K)

---

### 25. Bloom Filter — `include/libzenit/bloom.h`

Probabilistic set membership test with a tunable false positive rate. False negatives are impossible. Uses FNV-1a double hashing.

| Function | Description |
|---|---|
| `zenit_bloom_create(capacity, fpr)` | Create with expected capacity and false positive rate (optimal params computed automatically) |
| `zenit_bloom_create_with_allocator(capacity, fpr, allocator)` | Create with a custom allocator |
| `zenit_bloom_create_explicit(num_bits, num_hashes)` | Create with explicit parameters |
| `zenit_bloom_create_explicit_with_allocator(num_bits, num_hashes, allocator)` | Create explicit with a custom allocator |
| `zenit_bloom_destroy(bf)` | Free all memory; NULL-safe |
| `zenit_bloom_insert(bf, data, len)` | Insert an element (sets hash bits) |
| `zenit_bloom_contains(bf, data, len)` | 1 if possibly present, 0 if definitely absent |
| `zenit_bloom_clear(bf)` | Reset all bits; NULL-safe |
| `zenit_bloom_false_positive_rate(bf)` | Return the configured false positive rate |
| `zenit_bloom_count(bf)` | Approximate number of inserted items |
| `zenit_bloom_num_bits(bf)` | Total number of bits in the bit array |
| `zenit_bloom_num_hashes(bf)` | Number of hash functions used |

- **Source:** [`src/bloom.c`](src/bloom.c)
- **Tests:** [`tests/test_bloom.c`](tests/test_bloom.c) (15 sub-tests: create/destroy, insert/contains, clear, FPR, explicit params, NULL safety, many insertions, edge cases), [`tests/test_bloom_malloc_fail.c`](tests/test_bloom_malloc_fail.c) (allocation failure via `--wrap=malloc,calloc`)
- **Benchmark:** [`benchmarks/benchmark_bloom.c`](benchmarks/benchmark_bloom.c) — insert 100K, contains hit 100K, contains miss 100K

---

### 26. Trie / Prefix Tree — `include/libzenit/trie.h`

26-ary trie (a–z, case-insensitive) with integer payload per key. Supports insertion, search, prefix query, and deletion.

| Function | Description |
|---|---|
| `zenit_trie_create()` / `_with_allocator` | Create an empty trie; returns NULL on OOM |
| `zenit_trie_destroy(trie)` | Free all nodes; NULL-safe |
| `zenit_trie_insert(trie, key, value)` | Insert a key with integer payload |
| `zenit_trie_search(trie, key, out_value)` | 1 if found, 0 otherwise |
| `zenit_trie_starts_with(trie, prefix)` | 1 if any key begins with prefix |
| `zenit_trie_delete(trie, key)` | Remove a key; returns NOT_FOUND if absent |
| `zenit_trie_count(trie)` | Number of keys stored (0 if NULL) |
| `zenit_trie_clear(trie)` | Remove all keys; NULL-safe |

- **Source:** [`src/trie.c`](src/trie.c)
- **Tests:** [`tests/test_trie.c`](tests/test_trie.c) (12 sub-tests: create/destroy, insert/search, starts_with, delete, clear, count, prefix sharing, case folding, NULL safety, allocator variant), [`tests/test_trie_malloc_fail.c`](tests/test_trie_malloc_fail.c) (allocation failure via `--wrap=malloc,calloc`)
- **Benchmark:** [`benchmarks/benchmark_trie.c`](benchmarks/benchmark_trie.c) — insert 100K, search hit/miss 100K, starts_with 100K

---

### 27. LRU Cache — `include/libzenit/lru.h`

Fixed-capacity LRU cache with O(1) put / get. Uses a hash map with open-addressing and a doubly-linked list for the eviction order. Eviction callback support.

| Function | Description |
|---|---|
| `zenit_lru_create(key_size, value_size, capacity)` | Create empty cache; returns NULL on OOM |
| `zenit_lru_create_with_allocator(...)` | Create with a custom allocator |
| `zenit_lru_create_with_evict(key_size, value_size, capacity, evict_fn, ctx)` | Create with eviction callback |
| `zenit_lru_create_with_evict_and_allocator(...)` | Create with evict callback + custom allocator |
| `zenit_lru_destroy(cache)` | Free all memory; NULL-safe |
| `zenit_lru_put(cache, key, value)` | Insert or update; evicts LRU entry if full |
| `zenit_lru_get(cache, key, out_value)` | Retrieve and promote; returns 1 if found |
| `zenit_lru_peek(cache, key, out_value)` | Retrieve without promoting; returns 1 if found |
| `zenit_lru_contains(cache, key)` | 1 if present, 0 otherwise |
| `zenit_lru_remove(cache, key)` | Remove a single key |
| `zenit_lru_clear(cache)` | Remove all entries; NULL-safe |
| `zenit_lru_count(cache)` | Number of entries (0 if NULL) |
| `zenit_lru_capacity(cache)` | Maximum entries (0 if NULL) |

- **Source:** [`src/lru.c`](src/lru.c)
- **Tests:** [`tests/test_lru.c`](tests/test_lru.c) (19 sub-tests: create/destroy, put/get, eviction order, peek, update, remove, contains, clear, NULL safety, many ops, allocator, evict callback), [`tests/test_lru_malloc_fail.c`](tests/test_lru_malloc_fail.c) (allocation failure via `--wrap=malloc,calloc`)
- **Benchmark:** [`benchmarks/benchmark_lru.c`](benchmarks/benchmark_lru.c) — put 100K, get hit 100K, put evict 100K

---

### 28. Graph — `include/libzenit/graph.h`

Adjacency-list graph supporting directed and undirected modes. Dynamic vertex allocation with tombstone compaction. BFS and DFS traversals with a visit callback.

| Function | Description |
|---|---|
| `zenit_graph_create(capacity, directed)` / `_with_allocator` | Create empty graph; returns NULL on OOM |
| `zenit_graph_destroy(g)` | Free all memory; NULL-safe |
| `zenit_graph_add_vertex(g)` | Add a new vertex |
| `zenit_graph_remove_vertex(g, vertex)` | Remove vertex + incident edges |
| `zenit_graph_add_edge(g, from, to)` | Add edge (undirected adds reverse) |
| `zenit_graph_remove_edge(g, from, to)` | Remove edge |
| `zenit_graph_has_edge(g, from, to)` | 1 if edge exists |
| `zenit_graph_get_neighbors(g, vertex, out, out_count)` | Get malloc'd copy of neighbors (caller frees) |
| `zenit_graph_vertex_count(g)` | Number of active vertices (0 if NULL) |
| `zenit_graph_edge_count(g)` | Number of logical edges (0 if NULL) |
| `zenit_graph_is_directed(g)` | 1 if directed, 0 if undirected |
| `zenit_graph_clear(g)` | Remove all vertices + edges; NULL-safe |
| `zenit_graph_bfs(g, start, visit, ctx)` | Breadth-first traversal |
| `zenit_graph_dfs(g, start, visit, ctx)` | Depth-first traversal |

- **Source:** [`src/graph.c`](src/graph.c)
- **Tests:** [`tests/test_graph.c`](tests/test_graph.c) (34 sub-tests: directed/undirected, add/remove vertices, add/remove edges, has_edge, get_neighbors, BFS, DFS, clear, NULL safety, many vertices), [`tests/test_graph_malloc_fail.c`](tests/test_graph_malloc_fail.c) (allocation failure via `--wrap=malloc,calloc`)
- **Benchmark:** [`benchmarks/benchmark_graph.c`](benchmarks/benchmark_graph.c) — add_vertex 100K, add_edge 100K, BFS, get_neighbors

---

### 29. Directory API — `include/libzenit/dir.h`

Platform-independent directory operations: create (mkdir -p), remove, exists, iteration, and bulk listing. Uses `#ifdef _WIN32` / POSIX `dirent.h` internally.

| Function | Description |
|---|---|
| `zenit_dir_create(path)` | Create directory (mkdir -p); returns `ZENIT_RESULT_OK` on success |
| `zenit_dir_remove(path)` | Remove empty directory |
| `zenit_dir_exists(path)` | 1 if directory exists, 0 otherwise |
| `zenit_dir_iter(path)` | Create directory iterator; returns NULL on error |
| `zenit_dir_next(iter, out_entry)` | 1 if entry read, 0 at end |
| `zenit_dir_iter_destroy(iter)` | Free iterator; NULL-safe |
| `zenit_dir_list(path, out_names, out_count)` | List all entries (caller frees strings + array) |

- **Source:** [`src/dir.c`](src/dir.c)
- **Tests:** [`tests/test_dir.c`](tests/test_dir.c) (9 sub-tests: create/exists, nested create, list empty/with entries, iterate, remove, nonexistent, NULL params), [`tests/test_dir_malloc_fail.c`](tests/test_dir_malloc_fail.c) (allocation failure via `--wrap=malloc,calloc`)
- **Benchmark:** [`benchmarks/benchmark_dir.c`](benchmarks/benchmark_dir.c) — dir_list throughput

---

### 30. Path Utilities — `include/libzenit/path.h`

String-based path manipulation functions. All functions return newly allocated strings (caller frees). All have `_with_allocator` variants.

| Function | Description |
|---|---|
| `zenit_path_join(a, b)` | Join two path components |
| `zenit_path_dirname(path)` | Return parent directory path |
| `zenit_path_basename(path)` | Return final path component |
| `zenit_path_extension(path)` | Return extension including the dot |
| `zenit_path_normalize(path)` | Resolve `.` / `..`, collapse `//`, remove trailing `/` |

- **Source:** [`src/path.c`](src/path.c)
- **Tests:** [`tests/test_path.c`](tests/test_path.c) (21 sub-tests: join, dirname, basename, extension, normalize, hidden files, multi-dot, NULL params, allocator variants), [`tests/test_path_malloc_fail.c`](tests/test_path_malloc_fail.c) (allocation failure via `--wrap=malloc,calloc`)
- **Benchmark:** [`benchmarks/benchmark_path.c`](benchmarks/benchmark_path.c) — join/dirname/basename/normalize 100K each

---

### 31. Sort Improvements — `include/libzenit/sort.h`

The sort module now includes a **stable merge sort** (`zenit_sort_stable`) and **bound-based binary searches** (`zenit_lower_bound`, `zenit_upper_bound`) in addition to the existing quicksort and binary search.

| Function | Description |
|---|---|
| `zenit_sort_quick(base, count, elem_size, compare)` | In-place quicksort (not stable) |
| `zenit_sort_stable(base, count, elem_size, compare)` | Stable merge sort (O(n) temp buffer) |
| `zenit_binary_search(key, base, count, elem_size, compare)` | Any matching element (unspecified which) |
| `zenit_lower_bound(key, base, count, elem_size, compare)` | First element >= key |
| `zenit_upper_bound(key, base, count, elem_size, compare)` | First element > key |

- **Source:** [`src/sort.c`](src/sort.c)
- **Tests:** [`tests/test_sort.c`](tests/test_sort.c) (27 sub-tests: quicksort, binary search, lower_bound, upper_bound, stable_sort, stability, edge cases, NULL safety)
- **Benchmark:** [`benchmarks/benchmark_sort.c`](benchmarks/benchmark_sort.c) — sort_random, sort_sorted, stable_sort_random, stable_sort_sorted, binary_search hit/miss

---

### 31. Sort Improvements — `include/libzenit/sort.h`

The sort module now includes a **stable merge sort** (`zenit_sort_stable`) and **bound-based binary searches** (`zenit_lower_bound`, `zenit_upper_bound`) in addition to the existing quicksort and binary search.

| Function | Description |
|---|---|
| `zenit_sort_quick(base, count, elem_size, compare)` | In-place quicksort (not stable) |
| `zenit_sort_stable(base, count, elem_size, compare)` | Stable merge sort (O(n) temp buffer) |
| `zenit_binary_search(key, base, count, elem_size, compare)` | Any matching element (unspecified which) |
| `zenit_lower_bound(key, base, count, elem_size, compare)` | First element >= key |
| `zenit_upper_bound(key, base, count, elem_size, compare)` | First element > key |

- **Source:** [`src/sort.c`](src/sort.c)
- **Tests:** [`tests/test_sort.c`](tests/test_sort.c) (27 sub-tests: quicksort, binary search, lower_bound, upper_bound, stable_sort, stability, edge cases, NULL safety)
- **Benchmark:** [`benchmarks/benchmark_sort.c`](benchmarks/benchmark_sort.c) — sort_random, sort_sorted, stable_sort_random, stable_sort_sorted, binary_search hit/miss

---

### 32. UUID v4 — `include/libzenit/uuid.h`

RFC 4122 UUID v4 generation with cryptographically secure random sources (`getrandom`/`/dev/urandom` on Linux, `arc4random_buf` on macOS, `BCryptGenRandom` on Windows).

| Function | Description |
|---|---|
| `zenit_uuid_generate(out)` | Generate a random UUID v4 |
| `zenit_uuid_format(uuid, out)` | Format as 36-char string |
| `zenit_uuid_parse(str, out)` | Parse 36-char UUID string |
| `zenit_uuid_equal(a, b)` | Compare two UUIDs for equality |

- **Source:** [`src/uuid.c`](src/uuid.c)
- **Tests:** [`tests/test_uuid.c`](tests/test_uuid.c) (9 sub-tests: generate, format, round-trip, known string, uppercase, equal, not-equal, NULL params, invalid)
- **Benchmark:** [`benchmarks/benchmark_uuid.c`](benchmarks/benchmark_uuid.c) — generate, format, parse

---

### 33. Semantic Versioning — `include/libzenit/semver.h`

SemVer 2.0.0 parsing, formatting, and comparison with full precedence rules.

| Function | Description |
|---|---|
| `zenit_semver_parse(str, out)` | Parse "MAJOR.MINOR.PATCH[-pre][+build]" |
| `zenit_semver_format(v, out)` | Format back to string |
| `zenit_semver_format_with_allocator(v, out, allocator)` | Format with custom allocator |
| `zenit_semver_compare(a, b)` | Compare with precedence rules |

- **Source:** [`src/semver.c`](src/semver.c)
- **Tests:** [`tests/test_semver.c`](tests/test_semver.c) (13 sub-tests: parse, prerelease, build, rc+build, format, round-trip, compare, NULL, invalid, allocator)
- **Benchmark:** [`benchmarks/benchmark_semver.c`](benchmarks/benchmark_semver.c) — parse, format, compare

---

### 34. Structured Logger — `include/libzenit/logger.h`

Six-level logger with custom sinks, level filtering, timestamps, and convenience per-level functions.

| Function | Description |
|---|---|
| `zenit_logger_create(sink, ctx)` | Create logger (default allocator) |
| `zenit_logger_create_with_allocator(...)` | Create with custom allocator |
| `zenit_logger_destroy(logger)` | Free all memory; NULL-safe |
| `zenit_logger_set_level(logger, level)` / `zenit_logger_get_level(logger)` | Set/get minimum log level |
| `zenit_logger_log(logger, level, fmt, ...)` | Log with explicit level |
| `zenit_logger_trace/debug/info/warn/error/fatal(logger, ...)` | Convenience per-level functions |

Levels: `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL` (FATAL calls `abort()`).

- **Source:** [`src/logger.c`](src/logger.c)
- **Tests:** [`tests/test_logger.c`](tests/test_logger.c) (8 sub-tests: create/destroy, custom sink, level filtering, get/set, convenience functions, NULL safety, allocator variant)
- **Benchmark:** [`benchmarks/benchmark_logger.c`](benchmarks/benchmark_logger.c) — info, filtered, filtered-fast

---

### 35. Glob Pattern Matching — `include/libzenit/glob.h`

Shell-style glob pattern matching with `*`, `?`, character classes `[abc]`, ranges `[a-z]`, and negated classes `[!abc]`.

| Function | Description |
|---|---|
| `zenit_glob_match(pattern, str)` | Match string against glob pattern |

- **Source:** [`src/glob.c`](src/glob.c)
- **Tests:** [`tests/test_glob.c`](tests/test_glob.c) (10 sub-tests: exact, star, question, star empty, multi-star, char class, range, negated, NULL, complex)
- **Benchmark:** [`benchmarks/benchmark_glob.c`](benchmarks/benchmark_glob.c) — exact, star, star-miss, question, class

---

### 36. Lock-Free SPSC Ring Buffer — `include/libzenit/spsc.h`

Wait-free single-producer single-consumer ring buffer for thread-safe communication. Uses volatile and memory barriers — no mutexes.

| Function | Description |
|---|---|
| `zenit_spsc_create(elem_size, capacity)` | Create fixed-capacity buffer |
| `zenit_spsc_destroy(spsc)` | Free all memory; NULL-safe |
| `zenit_spsc_push(spsc, elem)` | Push element (producer) |
| `zenit_spsc_pop(spsc, out)` | Pop element (consumer) |
| `zenit_spsc_count(spsc)` | Number of elements stored |
| `zenit_spsc_capacity(spsc)` | Maximum capacity |
| `zenit_spsc_full(spsc)` / `zenit_spsc_empty(spsc)` | State queries |

- **Source:** [`src/spsc.c`](src/spsc.c)
- **Tests:** [`tests/test_spsc.c`](tests/test_spsc.c) (9 sub-tests: create/destroy, push/pop, full, empty, count, wrap-around, multiple, NULL, invalid params)
- **Benchmark:** [`benchmarks/benchmark_spsc.c`](benchmarks/benchmark_spsc.c) — push-pop, pop-push

---

### 37. Optional Type — `include/libzenit/optional.h`

Header-only generic optional value type. No heap allocation — value is stored inline.

| Macro | Description |
|---|---|
| `ZENIT_OPTIONAL(type)` | Declare an optional of `type` |
| `ZENIT_OPTIONAL_NONE` | Initializer for empty optional |
| `ZENIT_OPTIONAL_SOME(v)` | Initializer for valued optional |
| `zenit_optional_has(opt)` | 1 if value present |
| `zenit_optional_get(opt)` | Get value (undefined if empty) |
| `zenit_optional_set(opt, v)` | Set value |
| `zenit_optional_clear(opt)` | Reset to empty |
| `zenit_optional_get_or(opt, default)` | Get value or default |
| `zenit_optional_copy(opt, dst)` | Copy value to destination |
| `zenit_optional_map(opt, fn, out)` | Map function over value |

- **Source:** Header-only (`include/libzenit/optional.h`)
- **Tests:** [`tests/test_optional.c`](tests/test_optional.c) (10 sub-tests: none, some, set, clear, get_or, copy, map, map-none, double, pointer, overwrite)

---

### 38. CSV Parser/Serialiser — `include/libzenit/csv.h`

RFC 4180 CSV record parser and serialiser with quote escaping and configurable delimiter.

| Function | Description |
|---|---|
| `zenit_csv_parse_record(line, delim, out)` | Parse a CSV record (default allocator) |
| `zenit_csv_parse_record_with_allocator(...)` | Parse with custom allocator |
| `zenit_csv_record_destroy(record)` | Free record; NULL-safe |
| `zenit_csv_serialise_record(record, delim, out)` | Serialise to string (default allocator) |
| `zenit_csv_serialise_record_with_allocator(...)` | Serialise with custom allocator |

- **Source:** [`src/csv.c`](src/csv.c)
- **Tests:** [`tests/test_csv.c`](tests/test_csv.c) (9 sub-tests: simple, quoted, empty fields, serialise, quoted serialise, round-trip, NULL, single field, tab delimiter)
- **Benchmark:** [`benchmarks/benchmark_csv.c`](benchmarks/benchmark_csv.c) — parse 9-field, serialise 9-field

---

### 39. Thread Pool — `include/libzenit/thread_pool.h`

Cross-platform thread pool (pthreads / Win32 threads) with a fixed-size task queue and barrier synchronisation.

| Function | Description |
|---|---|
| `zenit_thread_pool_create(thread_count)` | Create pool; starts workers immediately |
| `zenit_thread_pool_destroy(pool)` | Drain queue, shut down workers; NULL-safe |
| `zenit_thread_pool_enqueue(pool, fn, ctx)` | Queue a task for execution |
| `zenit_thread_pool_thread_count(pool)` | Number of worker threads |
| `zenit_thread_pool_pending(pool)` | Queued + in-progress tasks |
| `zenit_thread_pool_wait(pool)` | Block until queue is empty |

- **Source:** [`src/thread_pool.c`](src/thread_pool.c)
- **Tests:** [`tests/test_thread_pool.c`](tests/test_thread_pool.c) (6 sub-tests: create/destroy, enqueue, pending, NULL, zero threads, single thread)
- **Benchmark:** [`benchmarks/benchmark_thread_pool.c`](benchmarks/benchmark_thread_pool.c) — enqueue (10K tasks)

---

## Build Options

| Option | Default | Description |
|---|---|---|
| `LIBZENIT_BUILD_TESTS` | `ON` | Build test executables |
| `LIBZENIT_BUILD_BENCHMARKS` | `OFF` | Build benchmarks (release mode recommended) |
| `LIBZENIT_BUILD_COVERAGE` | `OFF` | Instrument with `--coverage` for line coverage |

Compiler flags: `-Wall -Wextra -Wpedantic` (GCC/Clang/AppleClang), `CMAKE_POSITION_INDEPENDENT_CODE ON`.

Custom sanitizer builds use the `-DSANITIZE=<type>` convention (see CI).

---

## Project Map

```
libzen/
├── CMakeLists.txt              # Root build configuration
├── cmake/                      # Custom CMake modules (empty)
├── include/
│   ├── libzenit.h              # Umbrella header
│   └── libzenit/
│       ├── allocator.h         # Custom allocator interface
│       ├── result.h            # Error codes & result type
│       ├── version.h           # Version API
│       ├── state.h             # State machine API
│       ├── arena.h             # Arena allocator API
│       ├── benchmark.h         # Benchmark framework API
│       ├── bitset.h            # Bit set API
│       ├── json.h              # JSON parser / serializer API
│       ├── base64.h            # Base64 encoding/decoding
│       ├── hex.h               # Hex encoding/decoding
│       ├── uri.h               # URI percent-encoding
│       ├── str.h               # String utilities (trim/split/join)
│       ├── sort.h              # Quicksort + binary search
│       ├── ring.h              # Ring buffer API
│       ├── string.h            # String builder API
│       ├── vector.h            # Dynamic array API
│       ├── map.h               # Hash map API
│       ├── set.h               # Hash set API
│       ├── list.h              # Doubly linked list API
│       ├── heap.h              # Binary heap / priority queue API
│       ├── deque.h             # Double-ended queue API
│       ├── stack.h             # LIFO stack wrapper over vector
│       ├── queue.h             # FIFO queue wrapper over deque
│       │       ├── timer.h             # High-resolution stopwatch (value type)
│       ├── pool.h              # Fixed-capacity object pool
│       ├── io.h                # File I/O (read/write/append/copy/delete)
│       ├── bloom.h             # Bloom filter API
│       ├── trie.h              # Trie (prefix tree) API
│       ├── lru.h               # LRU cache API
│       ├── graph.h             # Graph (adjacency list) API
│       ├── dir.h               # Directory API
│       ├── dir.h               # Directory API
│       ├── path.h              # Path utilities (join/dirname/basename/normalize)
│       ├── uuid.h              # RFC 4122 UUID v4 generation
│       ├── semver.h            # Semantic version 2.0.0 parsing/comparison
│       ├── logger.h            # Structured logger with levels & sinks
│       ├── glob.h              # Shell-style glob pattern matching
│       ├── spsc.h              # Lock-free SPSC ring buffer
│       ├── optional.h          # Header-only generic optional type
│       ├── csv.h               # CSV record parser/serialiser
│       └── thread_pool.h       # Thread pool with task queue
├── src/
│   ├── CMakeLists.txt          # Library target: static libzenit
│   ├── result.c / version.c / state.c / arena.c / benchmark.c
│   ├── ring.c / _hash_common.h / vector.c / map.c / set.c
│   ├── list.c / heap.c / deque.c / string.c / bitset.c / json.c
│   ├── base64.c / hex.c / uri.c / str.c / sort.c
│   ├── stack.c / queue.c / timer.c / pool.c / io.c
│   ├── path.c / lru.c / graph.c / trie.c / bloom.c / dir.c
│   ├── uuid.c / semver.c / logger.c / glob.c / spsc.c
│   └── csv.c / thread_pool.c
├── tests/
│   ├── CMakeLists.txt          # 76 test executables (DRY helpers)
│   ├── test_malloc_fail.h      # Shared malloc/calloc wrappers
│   ├── test_runner.h           # Shared test runner
│   ├── test_result.c ... test_graph.c  # One per module
│   ├── test_*_malloc_fail.c    # Allocation-failure tests (--wrap)
├── benchmarks/
│   ├── CMakeLists.txt          # 38 benchmark executables
│   ├── benchmark_version.c ... benchmark_thread_pool.c  # All modules
├── scripts/
│   ├── benchmark_report.py     # CI benchmark log → BENCHMARK.md + charts
│   └── checksum.py             # Release SHA-256 generator
├── benchmark_charts/           # Auto-generated PNG charts
├── BENCHMARK.md                # Auto-generated benchmark report
├── AGENTS.md                   # AI agent guide (contributors)
└── codecov.yaml                # Codecov configuration
```

---

## CI/CD

| Job | Platforms | What it does |
|---|---|---|
| **build** | ubuntu (x86+arm), macOS, Windows × gcc/clang/msvc × Debug/Release | Compile, test (`-LE benchmark`), coverage upload |
| **ios** | macOS (Xcode, arm64) | iOS cross-compile |
| **sanitize** | ubuntu × 8 sanitizer configs | address+UB+leak, thread, memory, CFI, safe-stack, implicit-truncation |
| **benchmark** | ubuntu (x86+arm), macOS | Release-mode benchmarks, log artifact |
| **collect** | ubuntu | Aggregates benchmark logs, generates BENCHMARK.md + charts, commits with `[skip ci]` |
| **release** | linux (x86+arm), macOS, Windows, iOS | Builds + packages release artifacts, uploads to GitHub Release |

---

## Quality

- **Coverage:** 100% line coverage on all modules (verified via `lcov` + `gcov`). Malloc-failure paths tested via linker `--wrap`.
- **Static analysis:** SonarCloud quality gate — reliability, security, maintainability, duplications, technical debt.
- **Sanitizers:** AddressSanitizer, UndefinedBehaviorSanitizer, LeakSanitizer, ThreadSanitizer, MemorySanitizer, CFI, safe-stack, implicit-truncation — all passing in CI.
- **No external dependencies:** Zero — the library is self-contained C99.

---

## Further Reading

| Document | What it covers |
|---|---|
| [`AGENTS.md`](AGENTS.md) | Full contributor guide: code style, workflow, testing, coverage, benchmarks, CI reference |
| [`BENCHMARK.md`](BENCHMARK.md) | Performance results across platforms (auto-generated) |
| [`LICENSE`](LICENSE) | AGPL-3.0-only license text |
| [`codecov.yaml`](codecov.yaml) | Codecov ignore / threshold configuration |

---

## Status

Current version `0.1.0` — **alpha**. All 39 modules are implemented, fully tested (76 test executables), benchmarked (38 benchmarks), and passing CI across all platforms and sanitizers. All containers support custom allocators. Encoding/utility functions (base64, hex, uri, str, path) also support custom allocators via `_with_allocator` variants. The API is stable but may evolve before `1.0.0`.
