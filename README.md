<p align="center">
  <img src="media/logo.svg" alt="LibZenit" width="180">
</p>

Portable C library providing building blocks for systems programming: a **finite-state machine engine**, a **fixed-block arena allocator**, a **benchmark framework**, and a **version API**.

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
| `zenit_state_process_event(state, event, ctx)` | Feed an event; returns `0` on match, `-1` on miss |
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
| `zenit_arena_release(arena, ua)` | Release region back; fails if any buffer is still `IN_USE` |
| `zenit_usable_arena_allocate(ua, size)` | Sub-allocate a buffer; `.data == NULL` on OOM |
| `zenit_usable_buffer_free(buf)` | Free buffer; returns `-1` on double-free or corruption |
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
│       ├── version.h           # Version API
│       ├── state.h             # State machine API
│       ├── arena.h             # Arena allocator API
│       └── benchmark.h         # Benchmark framework API
├── src/
│   ├── CMakeLists.txt          # Library target: static libzenit
│   ├── version.c
│   ├── state.c
│   ├── arena.c
│   └── benchmark.c
├── tests/
│   ├── CMakeLists.txt          # 6 test executables
│   ├── test_version.c
│   ├── test_state.c
│   ├── test_state_malloc_fail.c
│   ├── test_arena.c            # 16 sub-tests
│   ├── test_arena_malloc_fail.c # 4 sub-tests
│   └── test_benchmark.c
├── benchmarks/
│   ├── CMakeLists.txt          # 3 benchmark executables (label: "benchmark")
│   ├── benchmark_version.c
│   ├── benchmark_state.c       # 3 cases (8-state, 1024-state, miss)
│   └── benchmark_arena.c       # 7 cases (arena vs malloc)
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

Current version `0.1.0` — **alpha**. All four modules are implemented, fully tested, benchmarked, and passing CI across all platforms and sanitizers. The API is stable but may evolve before `1.0.0`.
