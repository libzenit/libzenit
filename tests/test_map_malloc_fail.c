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

#include <libzenit/map.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_malloc_fail.h"

static int passed = 0;
static int failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while (0)
#define PASS() do { printf("PASS\n"); passed++; } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); failed++; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while (0)

/* ─── Test: create fails on first allocation (map handle) ─── */
static void test_create_fail_handle(void) {
    TEST("create fails on handle malloc");
    malloc_fail_countdown = 0;
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map == NULL, "expected NULL on malloc failure");
    malloc_fail_countdown = -1;
    PASS();
}

/* ─── Test: create fails on slots calloc ─── */
static void test_create_fail_slots(void) {
    TEST("create fails on slots calloc");
    malloc_fail_countdown = 1;
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map == NULL, "expected NULL when slots allocation fails");
    malloc_fail_countdown = -1;
    PASS();
}

/* ─── Test: create fails on states calloc ─── */
static void test_create_fail_states(void) {
    TEST("create fails on states calloc");
    malloc_fail_countdown = 2;
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map == NULL, "expected NULL when states allocation fails");
    malloc_fail_countdown = -1;
    PASS();
}

/* ─── Test: insert fails during rehash (slots calloc) ─── */
static void test_insert_fail_rehash_slots(void) {
    TEST("insert fails during rehash (slots calloc)");

    zenit_map_t *map = zenit_map_create_with_capacity(sizeof(int), sizeof(int), 1);
    ASSERT(map != NULL, "expected non-NULL map");

    int k1 = 10, v1 = 100;
    ASSERT(zenit_map_insert(map, &k1, &v1).error == ZENIT_OK, "first insert");

    malloc_fail_countdown = 0;
    int k2 = 20, v2 = 200;
    zenit_result_t r = zenit_map_insert(map, &k2, &v2);
    ASSERT(r.error == ZENIT_ERROR_ALLOC, "insert should fail with ALLOC on rehash");

    int out = 0;
    ASSERT(zenit_map_get(map, &k1, &out).error == ZENIT_OK, "first key still accessible");
    ASSERT(out == 100, "first value intact");

    malloc_fail_countdown = -1;
    zenit_map_destroy(map);
    PASS();
}

/* ─── Main ─── */
int main(void) {
    printf("hash map malloc-fail tests\n");
    printf("--------------------------\n");

    /* Disable printf buffering so output order is deterministic */
    setvbuf(stdout, NULL, _IONBF, 0);

    test_create_fail_handle();
    test_create_fail_slots();
    test_create_fail_states();
    test_insert_fail_rehash_slots();

    printf("\n%d passed, %d failed, %d total\n",
           passed, failed, passed + failed);

    return failed > 0 ? 1 : 0;
}
