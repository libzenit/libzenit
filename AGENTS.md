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

- **Constructors** return `NULL` on allocation failure.
- **Mutators** return `int`: `0` on success, `-1` on failure.
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
- **Allocation failure** — use `--wrap=malloc` (see existing `test_state_malloc_fail.c`):

```c
static int malloc_fail_once = 1;

void *__real_malloc(size_t size);
void *__wrap_malloc(size_t size) {
    if (malloc_fail_once) {
        malloc_fail_once = 0;
        return NULL;
    }
    return __real_malloc(size);
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
│       ├── version.h       # Version API
│       └── state.h         # State machine API
├── src/
│   ├── CMakeLists.txt
│   ├── version.c
│   └── state.c
├── tests/
│   ├── CMakeLists.txt
│   ├── test_version.c
│   ├── test_state.c
│   └── test_state_malloc_fail.c
├── scripts/
│   └── checksum.py         # Release checksum generator
├── codecov.yaml            # Codecov configuration
├── AGENTS.md               # This file
├── LICENSE                 # AGPL-3.0
└── README.md
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
