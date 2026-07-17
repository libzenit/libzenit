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
#include <string.h>

/* ─── Helpers ─── */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while (0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); tests_failed++; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return 1; } } while (0)

/* ─── Visitor context for foreach test ─── */
typedef struct {
    int keys[256];
    size_t count;
} visit_ctx_t;

static void visit_int(const void *key, void *ctx) {
    visit_ctx_t *c = (visit_ctx_t *)ctx;
    c->keys[c->count] = *(const int *)key;
    c->count++;
}

/* ─── Test: create and destroy ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");
    ASSERT(zenit_set_count(set) == 0, "count should be 0");
    ASSERT(zenit_set_capacity(set) >= 16, "capacity should be at least 16");
    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: create with capacity ─── */
static int test_create_with_capacity(void) {
    TEST("create with capacity");
    zenit_set_t *set = zenit_set_create_with_capacity(sizeof(int), 32);
    ASSERT(set != NULL, "expected non-NULL set");
    ASSERT(zenit_set_capacity(set) >= 32, "capacity should be at least 32");
    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: create with zero key_size returns NULL ─── */
static int test_create_zero_key(void) {
    TEST("create with zero key_size returns NULL");
    ASSERT(zenit_set_create(0) == NULL, "expected NULL");
    PASS();
    return 0;
}

/* ─── Test: create_with_capacity zero capacity returns NULL ─── */
static int test_create_zero_capacity(void) {
    TEST("create_with_capacity zero capacity returns NULL");
    ASSERT(zenit_set_create_with_capacity(sizeof(int), 0) == NULL, "expected NULL");
    PASS();
    return 0;
}

/* ─── Test: NULL destroy is safe ─── */
static int test_null_destroy(void) {
    TEST("NULL destroy is safe");
    zenit_set_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: insert and contains ─── */
static int test_insert_contains(void) {
    TEST("insert and contains");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int key = 42;
    zenit_result_t r = zenit_set_insert(set, &key);
    ASSERT(r.error == ZENIT_OK, "insert should succeed");
    ASSERT(zenit_set_count(set) == 1, "count should be 1");
    ASSERT(zenit_set_contains(set, &key) == 1, "contains should be true");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: insert duplicate (no-op) ─── */
static int test_insert_duplicate(void) {
    TEST("insert duplicate (no-op)");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int key = 1;
    ASSERT(zenit_set_insert(set, &key).error == ZENIT_OK, "first insert");
    ASSERT(zenit_set_count(set) == 1, "count 1");
    ASSERT(zenit_set_insert(set, &key).error == ZENIT_OK, "duplicate insert");
    ASSERT(zenit_set_count(set) == 1, "count still 1 after duplicate");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: contains not found ─── */
static int test_contains_not_found(void) {
    TEST("contains not found");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int key = 99;
    ASSERT(zenit_set_contains(set, &key) == 0, "should not contain key");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: remove ─── */
static int test_remove(void) {
    TEST("remove");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int key = 7;
    ASSERT(zenit_set_insert(set, &key).error == ZENIT_OK, "insert");
    ASSERT(zenit_set_count(set) == 1, "count 1");

    ASSERT(zenit_set_remove(set, &key).error == ZENIT_OK, "remove");
    ASSERT(zenit_set_count(set) == 0, "count 0 after remove");
    ASSERT(zenit_set_contains(set, &key) == 0, "contains false after remove");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: remove not found ─── */
static int test_remove_not_found(void) {
    TEST("remove not found");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int key = 999;
    ASSERT(zenit_set_remove(set, &key).error == ZENIT_ERROR_NOT_FOUND, "remove missing key");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: tombstone re-insert ─── */
static int test_tombstone(void) {
    TEST("tombstone re-insert");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int key = 5;
    ASSERT(zenit_set_insert(set, &key).error == ZENIT_OK, "first insert");
    ASSERT(zenit_set_remove(set, &key).error == ZENIT_OK, "remove");

    /* Re-insert the same key — should reuse the tombstone slot */
    ASSERT(zenit_set_insert(set, &key).error == ZENIT_OK, "re-insert");
    ASSERT(zenit_set_count(set) == 1, "count 1 after re-insert");
    ASSERT(zenit_set_contains(set, &key) == 1, "contains after re-insert");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: contains NULL set ─── */
static int test_contains_null(void) {
    TEST("contains NULL set returns 0");
    ASSERT(zenit_set_contains(NULL, NULL) == 0, "contains NULL should return 0");
    { int k = 1; ASSERT(zenit_set_contains(NULL, &k) == 0, "contains with NULL set"); }
    PASS();
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    TEST("clear");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_insert(set, &i).error == ZENIT_OK, "insert");
    }
    ASSERT(zenit_set_count(set) == 10, "count 10");

    zenit_set_clear(set);
    ASSERT(zenit_set_count(set) == 0, "count 0 after clear");

    /* After clear, inserting should still work */
    int k = 1;
    ASSERT(zenit_set_insert(set, &k).error == ZENIT_OK, "insert after clear");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: clear NULL is safe ─── */
static int test_clear_null(void) {
    TEST("clear NULL is safe");
    zenit_set_clear(NULL);
    PASS();
    return 0;
}

/* ─── Test: count NULL returns 0 ─── */
static int test_count_null(void) {
    TEST("count NULL returns 0");
    ASSERT(zenit_set_count(NULL) == 0, "count of NULL should be 0");
    PASS();
    return 0;
}

/* ─── Test: capacity NULL returns 0 ─── */
static int test_capacity_null(void) {
    TEST("capacity NULL returns 0");
    ASSERT(zenit_set_capacity(NULL) == 0, "capacity of NULL should be 0");
    PASS();
    return 0;
}

/* ─── Test: insert with NULL set ─── */
static int test_insert_null_set(void) {
    TEST("insert with NULL set");
    { int k = 1;
      ASSERT(zenit_set_insert(NULL, &k).error == ZENIT_ERROR_NULL, "NULL set"); }
    PASS();
    return 0;
}

/* ─── Test: insert with NULL key ─── */
static int test_insert_null_key(void) {
    TEST("insert with NULL key");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");
    ASSERT(zenit_set_insert(set, NULL).error == ZENIT_ERROR_NULL, "NULL key");
    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: insert multiple keys and verify ─── */
static int test_insert_multiple(void) {
    TEST("insert multiple keys");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int seen[10] = {0};
    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_insert(set, &i).error == ZENIT_OK, "insert");
    }
    ASSERT(zenit_set_count(set) == 10, "count 10");

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_contains(set, &i) == 1, "contains");
    }

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: remove with NULL set ─── */
static int test_remove_null_set(void) {
    TEST("remove with NULL set");
    { int k = 1;
      ASSERT(zenit_set_remove(NULL, &k).error == ZENIT_ERROR_NULL, "NULL set"); }
    PASS();
    return 0;
}

/* ─── Test: remove with NULL key ─── */
static int test_remove_null_key(void) {
    TEST("remove with NULL key");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");
    ASSERT(zenit_set_remove(set, NULL).error == ZENIT_ERROR_NULL, "NULL key");
    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: foreach ─── */
static int test_foreach(void) {
    TEST("foreach");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    for (int i = 0; i < 5; i++) {
        ASSERT(zenit_set_insert(set, &i).error == ZENIT_OK, "insert");
    }

    visit_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    zenit_set_foreach(set, visit_int, &ctx);
    ASSERT(ctx.count == 5, "foreach should visit 5 entries");

    int seen[5] = {0};
    for (size_t i = 0; i < ctx.count; i++) {
        int k = ctx.keys[i];
        ASSERT(k >= 0 && k < 5, "key out of range");
        seen[k] = 1;
    }
    for (int i = 0; i < 5; i++) {
        ASSERT(seen[i], "foreach missed key");
    }

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: foreach NULL set ─── */
static int test_foreach_null_set(void) {
    TEST("foreach NULL set is safe");
    zenit_set_foreach(NULL, visit_int, NULL);
    PASS();
    return 0;
}

/* ─── Test: foreach NULL visit ─── */
static int test_foreach_null_visit(void) {
    TEST("foreach NULL visit is safe");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");
    zenit_set_foreach(set, NULL, NULL);
    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: many inserts (triggers rehash) ─── */
static int test_many_inserts(void) {
    TEST("many inserts (rehash)");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    size_t n = 1000;
    for (size_t i = 0; i < n; i++) {
        int key = (int)i;
        ASSERT(zenit_set_insert(set, &key).error == ZENIT_OK, "insert");
    }
    ASSERT(zenit_set_count(set) == n, "count should match inserts");

    for (size_t i = 0; i < n; i++) {
        int key = (int)i;
        ASSERT(zenit_set_contains(set, &key) == 1, "contains after rehash");
    }

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: struct keys ─── */
typedef struct {
    int x;
    int y;
} point_t;

static int test_struct_keys(void) {
    TEST("struct keys");
    zenit_set_t *set = zenit_set_create(sizeof(point_t));
    ASSERT(set != NULL, "expected non-NULL set");

    point_t p1 = { 1, 2 };
    point_t p2 = { 3, 4 };

    ASSERT(zenit_set_insert(set, &p1).error == ZENIT_OK, "insert p1");
    ASSERT(zenit_set_insert(set, &p2).error == ZENIT_OK, "insert p2");
    ASSERT(zenit_set_count(set) == 2, "count 2");
    ASSERT(zenit_set_contains(set, &p1) == 1, "contains p1");
    ASSERT(zenit_set_contains(set, &p2) == 1, "contains p2");

    point_t p3 = { 5, 6 };
    ASSERT(zenit_set_contains(set, &p3) == 0, "not contains p3");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: remove all then re-insert ─── */
static int test_remove_all_reinsert(void) {
    TEST("remove all then re-insert");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    int keys[10] = {0,1,2,3,4,5,6,7,8,9};

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_insert(set, &keys[i]).error == ZENIT_OK, "insert");
    }

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_remove(set, &keys[i]).error == ZENIT_OK, "remove");
    }
    ASSERT(zenit_set_count(set) == 0, "count 0 after all removed");

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_insert(set, &keys[i]).error == ZENIT_OK, "re-insert");
    }
    ASSERT(zenit_set_count(set) == 10, "count 10 after re-insert");

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_set_contains(set, &keys[i]) == 1, "contains re-inserted");
    }

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Test: count after insert and remove ─── */
static int test_count_ops(void) {
    TEST("count after insert/remove");
    zenit_set_t *set = zenit_set_create(sizeof(int));
    ASSERT(set != NULL, "expected non-NULL set");

    ASSERT(zenit_set_count(set) == 0, "initial count 0");
    int k1 = 1;
    ASSERT(zenit_set_insert(set, &k1).error == ZENIT_OK, "insert");
    ASSERT(zenit_set_count(set) == 1, "count 1");

    int k2 = 2;
    ASSERT(zenit_set_insert(set, &k2).error == ZENIT_OK, "insert");
    ASSERT(zenit_set_count(set) == 2, "count 2");

    ASSERT(zenit_set_remove(set, &k1).error == ZENIT_OK, "remove");
    ASSERT(zenit_set_count(set) == 1, "count 1 after remove");

    zenit_set_destroy(set);
    PASS();
    return 0;
}

/* ─── Main ─── */
int main(void) {
    printf("hash set tests\n");
    printf("--------------\n");

    int total = 0;

    total += test_create_destroy();
    total += test_create_with_capacity();
    total += test_create_zero_key();
    total += test_create_zero_capacity();
    total += test_null_destroy();
    total += test_insert_contains();
    total += test_insert_duplicate();
    total += test_contains_not_found();
    total += test_remove();
    total += test_remove_not_found();
    total += test_tombstone();
    total += test_contains_null();
    total += test_clear();
    total += test_clear_null();
    total += test_count_null();
    total += test_capacity_null();
    total += test_insert_null_set();
    total += test_insert_null_key();
    total += test_insert_multiple();
    total += test_remove_null_set();
    total += test_remove_null_key();
    total += test_foreach();
    total += test_foreach_null_set();
    total += test_foreach_null_visit();
    total += test_many_inserts();
    total += test_struct_keys();
    total += test_remove_all_reinsert();
    total += test_count_ops();

    printf("\n%d passed, %d failed, %d total\n",
           tests_passed, tests_failed, total);

    return tests_failed > 0 ? 1 : 0;
}
