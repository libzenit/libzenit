# LibZenit — Agent Guide

## 1. Repository Overview

- **Language:** C (C99/C11 compatible)
- **Build:** CMake >= 3.20
- **License:** AGPL-3.0-only (Ian Torres, 2026)
- **Library target:** `zenit` (static library)
- **Namespace:** `libzenit::` (CMake export), `zenit_` / `libzenit_` (code prefix)
- **CI:** GitHub Actions (build matrix across 4 OS, sanitizers, iOS, releases)
- **Coverage:** Codecov via lcov + `--coverage` flag

> **This is a living document.** Every agent must update it when discovering new conventions, encountering structural changes, or making decisions worth recording (see §12).

## 2. Getting Started

```bash
cmake -B build -D CMAKE_BUILD_TYPE=Debug -D LIBZENIT_BUILD_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

To generate coverage report:
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Debug -D LIBZENIT_BUILD_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## 3. Workflow

Every agent **must** follow this cycle:

### 3.1 Check current branch
```bash
git branch --show-current
git status
```

### 3.2 Create a branch
- If on `master` → create a new branch.
- If on any other branch → create a new child branch from it.
- **Naming convention:** `kebab-case` with category prefix.

```bash
git checkout -b feat/state-persistence
git checkout -b fix/memory-leak-state
git checkout -b test/state-coverage
git checkout -b refactor/state-api
```

### 3.3 Commit discipline
Each commit must be **atomic** (one logical change). After every commit, push immediately.

Format: subject line + blank line + body (detailed description).

```bash
git add <files>
git commit -m "feat: add state persistence API" -m "Implements save/load for state machine.

- Adds zenit_state_save / zenit_state_load functions
- Handles NULL state and allocation failure
- Includes malloc-failure test with --wrap=malloc
- 100% coverage on all new code"
git push -u origin feat/state-persistence
```

**Subject line:** `<prefix>: <short description>` (imperative mood, no period).

| Prefix  | Usage                            |
|---------|----------------------------------|
| `feat`  | New feature                      |
| `fix`   | Bug fix                          |
| `test`  | Adding or fixing tests           |
| `refactor` | Code restructuring without behaviour change |
| `docs`  | Documentation                    |
| `chore` | Build, CI, tooling               |

**Body:** Describe what changed and why. Use bullet points for multiple changes. Always mention test coverage and any relevant edge cases handled.

### 3.4 Push after every commit
```bash
git push -u origin <branch-name>
```

If the branch already exists upstream, simply:
```bash
git push
```

> **⚠️ CI auto-commits.** The benchmark `collect` job may push a report
> commit (`[skip ci]`) to your branch between your commits. If your push
> is rejected with *non-fast-forward*, pull the CI-bot commit first:
> ```bash
> git fetch && git rebase && git push
> ```

### 3.5 End of iteration checklist
Before finishing any task:
- [ ] All tests pass: `ctest --test-dir build --output-on-failure`
- [ ] 100 % line coverage on new code
- [ ] Compiles cleanly with `-Wall -Wextra -Wpedantic` (no warnings)
- [ ] No memory leaks (run with AddressSanitizer: `-DSANITIZE=address`)
- [ ] No thread sanitizer issues (`-DSANITIZE=thread`)
- [ ] Branch pushed to `origin`
- [ ] Sonar issues resolved (if report provided)

## 4. Code Style

All conventions below are extracted from the existing codebase. Every new file must follow them exactly.

### 4.1 Naming

| Element            | Convention              | Example                          |
|--------------------|-------------------------|----------------------------------|
| Functions          | `snake_case`, prefixed  | `libzenit_version(void)`, `zenit_state_allocate(...)` |
| Types (typedef)    | `snake_case_t`, prefixed| `libzenit_version_t`, `zenit_state_transition_t` |
| Structs (opaque)   | forward-declared `typedef struct` | `typedef struct zenit_state_t zenit_state_t;` |
| Enums              | `UPPER_SNAKE_CASE`, anonymous | `enum { ST_IDLE, ST_ACTIVE, ST_ERROR };` |
| Macros             | `UPPER_SNAKE_CASE`     | `LIBZENIT_VERSION_H`             |
| Static globals     | `snake_case`           | `static int callback_invoked = 0;` |

**Internal struct definition** (in .c, not in header):
```c
struct zenit_state_t {
    const zenit_state_transition_t *table;
    size_t count;
    int current;
};
```

### 4.2 Pointer style

**Asterisk attached to the type** (`type* name`), never to the variable:

```c
const char* name;
void *context;
const zenit_state_transition_t *table;
zenit_state_t *state;
int *p;
```

### 4.3 Brace style

**K&R (Egyptian braces)** — opening brace on the same line:

```c
libzenit_version_t libzenit_version(void) {
    // ...
}

if (state == NULL) {
    return NULL;
}

for (size_t i = 0; i < state->count; i++) {
    // ...
}
```

### 4.4 Indentation

**4 spaces, no tabs.** Configure your editor accordingly.

### 4.5 Comments

- **File headers:** Block of `//` single-line comments (16 lines, see §5).

#### 4.5.1 Headers (`*.h`) — Doxygen docblocks

Every public function, typedef, struct, or macro exported in a header **must** carry a Doxygen documentation block in the following format:

```c
/**
 * @brief One line describing what the entity does.
 *
 * Optional paragraph with further detail, preconditions, postconditions,
 * ownership notes, thread safety, etc.
 *
 * @param param1 Description of the parameter.
 * @param param2 Description of the parameter.
 * @return Description of the return value.
 */
```

- Use `@param` (not `\param`).
- Use `@return` (not `\return` or `@returns`).
- For structs/typedefs, document fields inline with `/**< */`.

#### 4.5.2 Implementations (`*.c`) — Line-by-line comments

Every implemented function **must** document its internal logic with `/* ... */` or `//` comments that explain **what each line or block does** and, when relevant, **why** it is done that way.

The goal is not to parrot the C code — it is to explain the intent:

```c
/* Walk the transition table — O(n) */
for (size_t i = 0; i < state->count; i++) {
    /* Grab a pointer to the i-th rule (avoids copying the struct) */
    const zenit_state_transition_t *t = &state->table[i];
```

#### 4.5.3 Documentation verification

Before marking a task as complete, the agent must:

1. Verify that every new public function has a Doxygen docblock in its header.
2. Verify that every new implementation has line-by-line comments.

### 4.6 Include guards

**`#ifndef` / `#define` / `#endif`** pattern, path-based naming:

```c
#ifndef LIBZENIT_VERSION_H
#define LIBZENIT_VERSION_H
// ...
#endif
```

`#pragma once` is **not used**.

### 4.7 Include ordering

1. Corresponding header (in .c files)
2. Standard library headers

```c
#include <libzenit/state.h>
#include <stdlib.h>
```

### 4.8 Error handling

- **Constructors** return a typed pointer, or `NULL` on allocation failure (idiomatic C option pattern).
- **Mutators** return `zenit_result_t` (defined in `<libzenit/result.h>`):
  - `ZENIT_RESULT_OK` on success.
  - `ZENIT_RESULT_ERROR(ZENIT_ERROR_*)` with a specific `zenit_error_t` code on failure.
- Use `zenit_error_string(code)` for human-readable error messages.
- **Always check malloc/calloc return:**

```c
zenit_state_t *state = malloc(sizeof(zenit_state_t));
if (state == NULL) {
    return NULL;
}
```

### 4.9 Designated initializers (C99+)

```c
libzenit_version_t v = {
    .major = 0,
    .minor = 1,
    .patch = 0,
    .name = "alpha"
};

zenit_state_transition_t table[] = {
    { ST_IDLE,   EV_START, ST_ACTIVE, on_transition },
    { ST_ACTIVE, EV_STOP,  ST_IDLE,   on_transition },
};
```

### 4.10 Memory & thread safety

- **Always** check `malloc`/`calloc`/`realloc` for `NULL`.
- **Always** pair allocation with deallocation (`free`).
- Prefer **reentrant design** — pass state via `void *context`, avoid global/static mutable state.
- If a function is not thread-safe, document it clearly.
- Do **not** use: `VLAs`, `strcpy`, `sprintf`, `gets`, `strncpy` without explicit bounds.
- Use `size_t` for sizes and counts.

## 5. License Header Template

**Every** new `.c`, `.h`, `.cmake`, `.py`, `.sh`, `.yml`, or any other source file **must** carry the AGPL header. Copy this verbatim:

```
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
```

For CMake (`#` comments) and Python (`#` comments), adapt the comment delimiter accordingly:

```
#    LibZenit
#    Copyright (C) 2026  Ian Torres
...
```

## 6. Testing Requirements

### 6.1 Framework

No external test framework. Tests are plain C executables with `main()` returning `0` on pass, `1` on fail:

```c
#include <libzenit/state.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // ... test logic ...

    if (result != expected) {
        fprintf(stderr, "FAIL: descriptive message\n");
        return 1;
    }

    printf("PASS: test name\n");
    return 0;
}
```

### 6.2 Registering tests

Register every test in `tests/CMakeLists.txt` via `add_test()`:

```cmake
add_executable(test_my_feature test_my_feature.c)
target_link_libraries(test_my_feature PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_my_feature PRIVATE --coverage -g -O0)
  target_link_options(test_my_feature PRIVATE --coverage)
endif()

add_test(NAME test_my_feature COMMAND test_my_feature)
```

### 6.3 What to test
- **Happy path** — normal operation
- **Edge cases** — NULL pointers, empty tables, invalid events
- **Allocation failure** — use `--wrap=malloc` with the shared helper header `tests/test_malloc_fail.h` (see `test_ring_malloc_fail.c` for a minimal example):

```c
#include "test_malloc_fail.h"

int main(void) {
    malloc_fail_countdown = 0;  // fail next malloc/calloc
    void *p = malloc(1024);
    if (p == NULL) { /* success */ }
}
```

### 6.4 Running tests

```bash
ctest --test-dir build --output-on-failure
```

## 7. Coverage Requirements

- **100 % line coverage** on every new function added.
- Build with `-D LIBZENIT_BUILD_COVERAGE=ON` and verify with `gcov`:

```bash
cmake -B build -D CMAKE_BUILD_TYPE=Debug -D LIBZENIT_BUILD_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure
# Check .gcda /.gcno files generated
lcov --capture --directory build --output-file coverage.info
lcov --list coverage.info
```

Every branch (if/else, loops) and every error-return path must be exercised.

## 8. Quality Gates (Sonar)

When a Sonar report is provided at the end of an iteration:

1. Fix all **Bugs** and **Vulnerabilities** first (highest severity).
2. Fix **Code Smells** related to:
   - Memory leaks / resource leaks
   - Null pointer dereferences
   - Buffer overflows (`strcpy`, `sprintf`, unbounded reads)
   - Unused parameters (use `(void)param;` if intentionally unused)
   - Dead stores
   - Non-reentrant functions (modifying globals/statics)
3. Ensure **Duplications** are removed (extract shared logic into helpers).
4. Re-run tests and verify coverage after fixes.

## 9. Pre-Commit Checklist

Before concluding any work:

```
[ ] ctest --test-dir build --output-on-failure         # all pass
[ ] 100% line coverage on new code                      # gcov / lcov verify
[ ] Compiles with -Wall -Wextra -Wpedantic (no warnings)
[ ] No memory leaks (AddressSanitizer / valgrind)
[ ] No thread safety issues (ThreadSanitizer)
[ ] git push done, branch on origin
[ ] Sonar report processed (if provided)
```

## 10. CI/CD Reference

| Job | Triggers | What it does |
|-----|----------|-------------|
| **build** | PRs, pushes to master, tags `v*` | Matrix: 4 OS × gcc/clang/msvc × Debug/Release. Coverage builds upload to Codecov. |
| **ios** | Same as build | iOS cross-compile (Xcode, arm64) |
| **sanitize** | Same as build | address+undefined+leak, thread, memory, cfi, safe-stack, implicit-truncation |
| **release** | Tags `v*` | Builds release artifacts for linux-amd64/arm64, macos-arm64, windows-amd64, ios-arm64 |

## 11. Project structure

> **Keep this in sync.** Update this tree whenever you add, rename, or remove directories or files — it is the agent's primary navigation reference.

```
libzen/
    ├── CMakeLists.txt          # Root build configuration
    ├── cmake/                  # Custom CMake modules (empty, available for use)
    ├── include/
    │   ├── libzenit.h          # Umbrella header
    │   └── libzenit/
    │       ├── result.h        # Error codes & result type
    │       ├── version.h       # Version API
    │       ├── state.h         # State machine API
    │       ├── arena.h         # Arena allocator API
    │       ├── benchmark.h     # Benchmark framework API
    │       └── ring.h          # Ring buffer API
    ├── src/
    │   ├── CMakeLists.txt
    │   ├── result.c            # Error string conversion
    │   ├── version.c
    │   ├── state.c
    │   ├── arena.c             # Arena allocator impl (free-list, bitmap, boundary tags)
    │   ├── benchmark.c         # Benchmark runner impl (clock_gettime / QueryPerformanceCounter)
    │   └── ring.c              # Ring buffer impl (circular FIFO, wrap-around)
    ├── tests/
    │   ├── CMakeLists.txt
    │   ├── test_malloc_fail.h  # Shared malloc/calloc wrappers for --wrap tests
    │   ├── test_result.c       # Error code & macro validation
    │   ├── test_version.c
    │   ├── test_state.c
    │   ├── test_state_malloc_fail.c
    │   ├── test_arena.c        # Arena happy path, edge cases, coalescing, corruption
    │   ├── test_arena_malloc_fail.c  # Malloc/calloc failure via --wrap
    │   ├── test_ring.c         # Ring buffer happy path, wrap-around, edge cases
    │   ├── test_ring_malloc_fail.c   # Malloc/calloc failure via --wrap
    │   └── test_benchmark.c    # Benchmark API validation & coverage
├── benchmarks/
    │   ├── CMakeLists.txt
    │   ├── benchmark_version.c     # Version call throughput
    │   ├── benchmark_state.c       # State-machine transition throughput
    │   ├── benchmark_arena.c       # Arena allocator throughput (vs malloc baseline)
    │   └── benchmark_ring.c        # Ring buffer throughput (seq 128B, 1K, full-miss)
├── scripts/
│   ├── checksum.py         # Release checksum generator
│   └── benchmark_report.py # Benchmark log parser & report generator (BENCHMARK.md + charts)
├── media/                  # Brand assets (logo)
│   └── logo.svg            # LibZenit logo (geometric sun)
├── codecov.yaml            # Codecov configuration
├── AGENTS.md               # This file
├── LICENSE                 # AGPL-3.0
├── README.md
└── BENCHMARK.md            # Auto-generated CI benchmark report ([skip ci])
```

## 12. Feedback Protocol (AGENTS.md)

AGENTS.md is the project's **collective memory** for AI agents. It must evolve as the project grows.

### 12.1 When to update

Update AGENTS.md whenever you encounter:

- **New directory / file added** — sync the project tree in §11.
- **New code convention discovered** — e.g. a new naming pattern, a style rule not yet documented.
- **Build system changes** — new CMake options, new targets, changed flags.
- **Tooling changes** — new CI jobs, new sanitizers, new scripts.
- **Workflow improvements** — better ways to branch, commit, test, or verify.
- **Common pitfalls** — errors or gotchas you resolved that future agents should avoid.
- **Any decision worth recording** — architecture rationale, API design choices, dependency justifications.

### 12.2 How to update

1. **Read** the current AGENTS.md first (to avoid duplication).
2. **Edit** the relevant section, or add a new subsection if the topic is new.
3. **Commit** with prefix `docs:` and a descriptive message:
   ```
   docs: add thread-safety considerations to code style section
   docs: update project tree after adding src/parser.c
   ```
4. **Push** immediately — see §3.4.

### 12.3 Guiding principles

| Principle | Rationale |
|-----------|-----------|
| **Be concise** | Agents have limited context windows. Prefer tables and examples over prose. |
| **Be precise** | Include exact code snippets, not descriptions. Show, don't tell. |
| **Be actionable** | Every rule must be directly enforceable (e.g. "4 spaces, no tabs", not "use good formatting"). |
| **Don't repeat** | If the information exists elsewhere (README, CI YAML, CMake), reference it rather than copying. |

### 12.4 Review on entry

Every agent **must** read AGENTS.md at the start of a session to ensure awareness of the latest conventions and project state.

---

## 13. Merge closure protocol

When the user says **"Acabo de fusionar tu rama"** (or any variant indicating the merge is complete), the agent must run:

```
git checkout master
git pull
```

This ensures the agent returns to the main branch with the latest merged code.

- Do not fetch or rebase unless explicitly instructed.
- If the current branch is already `master`, still run `git pull` to update.
- Report the result of `git pull` to the user (fast-forward or already up-to-date).

## 14. Benchmarking Discipline

Every agent that adds or modifies a public API **must** include or update a corresponding benchmark in `benchmarks/`.

### 14.1 API

The benchmark framework is part of the public library (`<libzenit/benchmark.h>`). Use it in all benchmarks:

```c
#include <libzenit/benchmark.h>

static void my_bench_fn(void* ctx) {
    /* one unit of work, called repeatedly by the runner */
}
```

```c
zenit_bench_result_t r = zenit_bench_run("my_label", my_bench_fn, ctx, 1000000);
zenit_bench_print(&r);
```

| Element | Purpose |
|---------|---------|
| `zenit_bench_result_t` | Holds name, elapsed time, iterations, ops/sec |
| `zenit_bench_fn_t` | Pointer to the function under measurement |
| `zenit_bench_run()` | Runs warm-up + timed loop, returns result |
| `zenit_bench_print()` | Prints aligned output to stdout |

### 14.2 What to benchmark

| Case | Example |
|------|---------|
| **Happy path** | Sequential transitions in a state machine |
| **Worst case** | Linear scan of a 1024-entry table |
| **Miss / no-op** | Invalid event (table miss) |
| **Comparison** | Arena alloc/free vs malloc/free baseline |

### 14.3 Timing infrastructure

- `clock_gettime(CLOCK_MONOTONIC, ...)` on POSIX (Linux, macOS)
- `QueryPerformanceCounter` on Windows
- One warm-up iteration before timed run

### 14.4 Build & run

```bash
cmake -B build -D LIBZENIT_BUILD_BENCHMARKS=ON
cmake --build build
ctest --test-dir build -L benchmark        # via CTest
cmake --build build --target benchmark     # via custom target
```

Benchmarks are **labeled** `"benchmark"` in CTest so they never run with
plain `ctest`. Use `ctest -L benchmark` to run only benchmarks, or
`ctest -LE benchmark` to exclude them.

Benchmark outputs are **automatically collected** in CI: each matrix job
uploads its log as an artifact, and a `collect` job downloads all three,
generates `BENCHMARK.md` + charts via `scripts/benchmark_report.py`, and
commits them with `[skip ci]` to avoid re-triggering the workflow.

### 14.5 File conventions

| Aspect | Convention |
|--------|-----------|
| Location | `benchmarks/benchmark_<feature>.c` |
| Prefix | `benchmark_` |
| Link target | PRIVATE `zenit` |
| Registration | `add_test(NAME benchmark_<name> COMMAND benchmark_<name>)` + LABELS `"benchmark"` |
| Return | `0` on success, `1` on error |

### 14.6 Verification

Before marking a task complete, verify:
- [ ] Every new public function has a corresponding benchmark.
- [ ] Benchmarks compile with `-Wall -Wextra -Wpedantic` (no warnings).
- [ ] Results are printed via `zenit_bench_print()`.
- [ ] Benchmarks do **not** run with plain `ctest` (label isolation).

## 15. README.md Discipline

`README.md` is the project's **public front door** and must serve as a comprehensive entry point that navigates to every dimension of the project.

### 15.1 When to update

Update `README.md` whenever any change affects how the project is described, built, tested, or understood:

- **New module added** — add a new module subsection with API summary, source path, test path, benchmark path.
- **New public function or type** — update the relevant module table.
- **New test or benchmark** — add the file path and a brief description.
- **Build option added or changed** — update the build options table.
- **CI job added, removed, or changed** — update the CI/CD table.
- **Project tree changes** (new directory or file at the top two levels) — sync the project map.
- **Version bump or status change** — update the Status section.
- **New script added** — update the project map and scripts section.
- **New platform or compiler** — update CI/CD and any relevant sections.
- **New documentation file** (BENCHMARK.md, CHANGELOG, etc.) — add a link in "Further Reading".

### 15.2 What to maintain

The README must always reflect the **current state** of these sections:

| Section | Mandatory content |
|---------|-------------------|
| Badges | CI, coverage, SonarCloud quality gates |
| Quick Start | Build, test, coverage, benchmark commands |
| Modules | One subsection per module: API table, source, test, benchmark links |
| Build Options | CMake options table with defaults and descriptions |
| Project Map | Full directory tree with one-line annotations |
| CI/CD | Job table with platforms and description |
| Quality | Coverage, static analysis, sanitizers, dependencies |
| Further Reading | Links to AGENTS.md, BENCHMARK.md, LICENSE, etc. |
| Status | Current version and release phase |

### 15.3 How to update

1. **Read** the current README.md first (to know what exists).
2. **Edit** the relevant section(s). Keep tables aligned and links absolute to `tree/master`.
3. **Verify** that every module listed has its source, test, and benchmark file paths.
4. **Verify** that every test and benchmark file in the repository is referenced somewhere.
5. **Verify** that the Project Map matches the actual filesystem.

### 15.4 Relationship with AGENTS.md

| Document | Audience | Purpose |
|----------|----------|---------|
| `README.md` | Public (users, contributors) | Project overview, navigation, quick start |
| `AGENTS.md` | AI agents (internal) | Workflow, conventions, agent-specific protocol |

The two documents complement each other — `AGENTS.md` references `README.md` for project map and public overview, while `README.md` references `AGENTS.md` for contributor workflow. Do **not** duplicate content; link between them.

### 15.5 Pre-Commit verification

Before marking a README.md change as complete:
- [ ] Every new or changed public API is documented in its module table.
- [ ] Every new source file, test, or benchmark has a file path reference.
- [ ] The Project Map is in sync with the actual directory tree.
- [ ] Build options table matches `CMakeLists.txt`.
- [ ] CI/CD table matches `.github/workflows/ci.yml`.
- [ ] Links are valid (point to `tree/master` or `blob/master`).
