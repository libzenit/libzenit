<p align="center">
  <img src="media/logo.svg" alt="LibZenit" width="180">
</p>

Portable C library providing building blocks for systems programming: a **typed result type with error codes**, a **ring buffer**, a **finite-state machine engine**, a **fixed-block arena allocator**, a **benchmark framework**, a **dynamic array (vector)**, a **hash map**, a **hash set**, a **doubly linked list**, a **binary heap (priority queue)**, a **double-ended queue (deque)**, and a **version API**.

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
| `zenit_state_process_event(state, event, ctx)` | Feed an event; returns `zenit_result_t` (`ZENIT_OK` / `ZENIT_ERROR_NOT_FOUND`) |
| `zenit_get_last_state(state)` | Read current state (no side-effect) |
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
├── CMakeLists.txt              # Root build (57 lines)
├── cmake/                      # Custom CMake modules
├── include/
│   ├── libzenit.h              # Umbrella header
│   └── libzenit/
│       ├── result.h            # Error codes & result type
│       ├── version.h           # Version API
│       ├── state.h             # State machine API
│       ├── arena.h             # Arena allocator API
│       ├── benchmark.h         # Benchmark framework API
│       ├── ring.h              # Ring buffer API
│       ├── vector.h            # Dynamic array API
│       ├── map.h               # Hash map API
│       ├── set.h               # Hash set API
│       ├── list.h              # Doubly linked list API
│       ├── heap.h              # Binary heap / priority queue API
│       └── deque.h             # Double-ended queue API
├── src/
│   ├── CMakeLists.txt          # Library target: static libzenit
│   ├── result.c
│   ├── version.c
│   ├── state.c
│   ├── arena.c
│   ├── benchmark.c
│   ├── ring.c
│   ├── vector.c
│   ├── map.c
│   ├── set.c
│   ├── list.c
│   ├── heap.c
│   └── deque.c
├── tests/
│   ├── CMakeLists.txt          # 21 test executables
│   ├── test_malloc_fail.h      # Shared malloc/calloc wrappers
│   ├── test_result.c           # 11 error codes + macro helpers
│   ├── test_version.c
│   ├── test_state.c
│   ├── test_state_malloc_fail.c
│   ├── test_arena.c            # 16 sub-tests
│   ├── test_arena_malloc_fail.c # 4 sub-tests
│   ├── test_ring.c             # 13 sub-tests
│   ├── test_ring_malloc_fail.c # 2 sub-tests
│   ├── test_benchmark.c
│   ├── test_vector.c           # 20 sub-tests
│   ├── test_vector_malloc_fail.c # 6 sub-tests
│   ├── test_map.c              # 34 sub-tests
│   ├── test_map_malloc_fail.c  # 4 sub-tests
│   ├── test_set.c              # 28 sub-tests
│   ├── test_set_malloc_fail.c  # 5 sub-tests
│   ├── test_list.c             # 24 sub-tests
│   ├── test_list_malloc_fail.c # 4 sub-tests
│   ├── test_heap.c             # 13 sub-tests
│   ├── test_heap_malloc_fail.c # 3 sub-tests
│   ├── test_deque.c            # 19 sub-tests
│   └── test_deque_malloc_fail.c # 4 sub-tests
├── benchmarks/
│   ├── CMakeLists.txt          # 10 benchmark executables (label: "benchmark")
│   ├── benchmark_version.c
│   ├── benchmark_state.c       # 3 cases (8-state, 1024-state, miss)
│   ├── benchmark_arena.c       # 7 cases (arena vs malloc)
│   ├── benchmark_ring.c        # 3 cases (seq 128B, seq 1K, full-miss)
│   ├── benchmark_vector.c      # 4 cases (seq push, push/pop, insert front, reserve+push)
│   ├── benchmark_map.c         # 5 cases (insert, get hit/miss, rehash, foreach)
│   ├── benchmark_set.c         # 5 cases (insert, contains hit/miss, rehash, foreach)
│   ├── benchmark_list.c        # 4 cases (push_back, push_front, push_pop, foreach)
│   ├── benchmark_heap.c        # 3 cases (push, push_pop, peek)
│   └── benchmark_deque.c       # 3 cases (push_back, push_front, push_pop)
├── scripts/
│   ├── benchmark_report.py     # CI benchmark log → BENCHMARK.md + charts
│   └── checksum.py             # Release SHA-256 generator
├── benchmark_charts/           # Auto-generated PNG charts (6 files)
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

Current version `0.1.0` — **alpha**. All eleven modules are implemented, fully tested, benchmarked, and passing CI across all platforms and sanitizers. The API is stable but may evolve before `1.0.0`.
