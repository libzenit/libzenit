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

#include <libzenit/set.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_malloc_fail.h"

static int passed = 0;
static int failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while (0)
#define PASS() do { printf("PASS\n"); passed++; } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); failed++; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while (0)

/* ─── Test: create fails on handle malloc ─── */
static void test_create_fail_handle(void) {
    TEST("create fails on handle malloc");
    malloc_fail_countdown = 0;
    const zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set == NULL, "expected NULL on malloc failure");
    malloc_fail_countdown = -1;
    PASS();
}

/* ─── Test: create fails on slots calloc ─── */
static void test_create_fail_slots(void) {
    TEST("create fails on slots calloc");
    malloc_fail_countdown = 1;
    const zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set == NULL, "expected NULL when slots allocation fails");
    malloc_fail_countdown = -1;
    PASS();
}

/* ─── Test: create fails on states calloc ─── */
static void test_create_fail_states(void) {
    TEST("create fails on states calloc");
    malloc_fail_countdown = 2;
    const zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set == NULL, "expected NULL when states allocation fails");
    malloc_fail_countdown = -1;
    PASS();
}

/* ─── Test: insert fails during rehash (slots calloc) ─── */
static void test_insert_fail_rehash_slots(void) {
    TEST("insert fails during rehash (slots calloc)");

    zenit_set_t *set = zenit_set_create_with_capacity(sizeof(int), 1);
    ASSERT(set != NULL, "expected non-NULL set");

    int k1 = 10;
    ASSERT(zenit_set_insert(set, &k1).error == ZENIT_OK, "first insert");

    malloc_fail_countdown = 0;
    int k2 = 20;
    zenit_result_t r = zenit_set_insert(set, &k2);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "insert should fail with ALLOC on rehash");

    ASSERT(zenit_set_contains(set, &k1) == 1, "first key still accessible");

    malloc_fail_countdown = -1;
    zenit_set_destroy(set);
    PASS();
}

/* ─── Test: insert fails during rehash (states calloc fails after slots succeed) ─── */
static void test_insert_fail_rehash_states(void) {
    TEST("insert fails during rehash (states calloc)");

    zenit_set_t *set = zenit_set_create_with_capacity(sizeof(int), 1);
    ASSERT(set != NULL, "expected non-NULL set");

    int k1 = 10;
    ASSERT(zenit_set_insert(set, &k1).error == ZENIT_OK, "first insert");

    malloc_fail_countdown = 1;
    int k2 = 20;
    zenit_result_t r = zenit_set_insert(set, &k2);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "insert should fail with ALLOC on rehash");

    ASSERT(zenit_set_contains(set, &k1) == 1, "first key still accessible");

    malloc_fail_countdown = -1;
    zenit_set_destroy(set);
    PASS();
}

/* ─── Test: set_to_array fails on allocation ─── */
static void test_to_array_fail_alloc(void) {
    TEST("set_to_array alloc fail");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "create");
    int k = 42;
    ASSERT(zenit_set_insert(set, &k).error == ZENIT_OK, "insert");
    int *keys = NULL;
    size_t count = 0;
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_set_to_array(set, (void**)&keys, &count);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "expected ALLOC");
    malloc_fail_countdown = -1;
    zenit_set_destroy(set);
    PASS();
}

/* ─── Main ─── */
int main(void) {
    printf("hash set malloc-fail tests\n");
    printf("--------------------------\n");

    setvbuf(stdout, NULL, _IONBF, 0);

    test_create_fail_handle();
    test_create_fail_slots();
    test_create_fail_states();
    test_insert_fail_rehash_slots();
    test_insert_fail_rehash_states();
    test_to_array_fail_alloc();

    printf("\n%d passed, %d failed, %d total\n",
           passed, failed, passed + failed);

    return failed > 0 ? 1 : 0;
}
