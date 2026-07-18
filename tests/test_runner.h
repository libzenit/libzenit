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

#ifndef LIBZENIT_TEST_RUNNER_H
#define LIBZENIT_TEST_RUNNER_H

#include <stdio.h>
#include <stdlib.h>

/*
 * Shared test runner infrastructure.
 *
 * Usage:
 *   #include "test_runner.h"
 *   ...
 *   int main(void) {
 *       TEST_ENTRY tests[] = {
 *           { test_feature, "feature" },
 *           { 0, 0 }
 *       };
 *       return test_run_all("my_suite", tests);
 *   }
 *
 * Each test function must return 0 on success, 1 on failure and use
 * the TEST/PASS/FAIL/ASSERT macros for output.
 */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while (0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); tests_failed++; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return 1; } } while (0)

typedef struct {
    int (*fn)(void);
    const char *name;
} TEST_ENTRY;

static inline int test_run_all(const char *suite, const TEST_ENTRY *tests) {
    printf("=== %s ===\n", suite);
    for (int i = 0; tests[i].fn != NULL; i++) {
        tests[i].fn();
    }

    printf("\n%d passed, %d failed out of %d\n",
           tests_passed, tests_failed, tests_passed + tests_failed);

    if (tests_failed != 0) return 1;

    size_t total = tests_passed + tests_failed;
    size_t expected = 0;
    while (tests[expected].fn != NULL) expected++;
    if (total != expected) {
        fprintf(stderr, "Mismatch: %zu tests defined, %zu executed\n", expected, total);
        return 1;
    }

    return 0;
}

/*
 * Array-driven test runner for sequential test suites.
 *
 * Usage:
 *   int main(void) {
 *       const int (*tests[])(void) = { &test_a, &test_b, NULL };
 *       const char *names[] = { "test_a", "test_b" };
 *       return ZENIT_RUN_TESTS("suite", tests, names);
 *   }
 */
#define ZENIT_RUN_TESTS(suite, tests, names) do { \
    size_t _n = sizeof(names) / sizeof(names[0]); \
    int _failed = 0; \
    printf("=== %s ===\n", suite); \
    for (size_t _i = 0; _i < _n; _i++) { \
        printf("  %s ... ", names[_i]); \
        int _r = tests[_i](); \
        if (_r) { _failed++; } \
        else { printf("PASS\n"); } \
    } \
    printf("\n%d tests failed\n", _failed); \
    return _failed != 0 ? 1 : 0; \
} while(0)

#endif
