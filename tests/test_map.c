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
#include <string.h>

#include "test_runner.h"

/* ─── Visitor context for foreach test ─── */
typedef struct {
    int keys[256];
    int values[256];
    size_t count;
} visit_ctx_t;

static void visit_int(const void *key, const void *value, void *ctx) {
    visit_ctx_t *c = (visit_ctx_t *)ctx;
    c->keys[c->count] = *(const int *)key;
    c->values[c->count] = *(const int *)value;
    c->count++;
}

/* ─── Test: create and destroy ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    ASSERT(zenit_map_count(map) == 0, "count should be 0");
    ASSERT(zenit_map_capacity(map) >= 16, "capacity should be at least 16");
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: create with capacity ─── */
static int test_create_with_capacity(void) {
    TEST("create with capacity");
    zenit_map_t *map = zenit_map_create_with_capacity(sizeof(int), sizeof(int), 32);
    ASSERT(map != NULL, "expected non-NULL map");
    ASSERT(zenit_map_capacity(map) >= 32, "capacity should be at least 32");
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: create with zero params returns NULL ─── */
static int test_create_zero_params(void) {
    TEST("create with zero key_size returns NULL");
    ASSERT(zenit_map_create(0, sizeof(int)) == NULL, "expected NULL");
    PASS();
    return 0;
}

/* ─── Test: create with zero value_size returns NULL ─── */
static int test_create_zero_value(void) {
    TEST("create with zero value_size returns NULL");
    ASSERT(zenit_map_create(sizeof(int), 0) == NULL, "expected NULL");
    PASS();
    return 0;
}

/* ─── Test: create_with_capacity zero capacity returns NULL ─── */
static int test_create_zero_capacity(void) {
    TEST("create_with_capacity zero capacity returns NULL");
    ASSERT(zenit_map_create_with_capacity(sizeof(int), sizeof(int), 0) == NULL, "expected NULL");
    PASS();
    return 0;
}

/* ─── Test: NULL destroy is safe ─── */
static int test_null_destroy(void) {
    TEST("NULL destroy is safe");
    zenit_map_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: insert and get ─── */
static int test_insert_get(void) {
    TEST("insert and get");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 42;
    int value = 100;
    zenit_result_t r = zenit_map_insert(map, &key, &value);
    ASSERT(r.error == ZENIT_OK, "insert should succeed");
    ASSERT(zenit_map_count(map) == 1, "count should be 1");

    int out = 0;
    r = zenit_map_get(map, &key, &out);
    ASSERT(r.error == ZENIT_OK, "get should succeed");
    ASSERT(out == 100, "value should be 100");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: insert overwrite ─── */
static int test_insert_overwrite(void) {
    TEST("insert overwrite");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 1;
    int v1 = 10;
    int v2 = 20;
    ASSERT(zenit_map_insert(map, &key, &v1).error == ZENIT_OK, "first insert");
    ASSERT(zenit_map_insert(map, &key, &v2).error == ZENIT_OK, "second insert");
    ASSERT(zenit_map_count(map) == 1, "count should still be 1");

    int out = 0;
    ASSERT(zenit_map_get(map, &key, &out).error == ZENIT_OK, "get");
    ASSERT(out == 20, "value should be 20 (overwritten)");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: get not found ─── */
static int test_get_not_found(void) {
    TEST("get not found");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 99;
    int out = 0;
    zenit_result_t r = zenit_map_get(map, &key, &out);
    ASSERT(r.error == ZENIT_ERROR_NOT_FOUND, "get should return NOT_FOUND");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: remove ─── */
static int test_remove(void) {
    TEST("remove");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 7;
    int value = 77;
    ASSERT(zenit_map_insert(map, &key, &value).error == ZENIT_OK, "insert");
    ASSERT(zenit_map_count(map) == 1, "count 1");

    ASSERT(zenit_map_remove(map, &key).error == ZENIT_OK, "remove");
    ASSERT(zenit_map_count(map) == 0, "count 0 after remove");

    int out = 0;
    ASSERT(zenit_map_get(map, &key, &out).error == ZENIT_ERROR_NOT_FOUND, "get after remove");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: remove not found ─── */
static int test_remove_not_found(void) {
    TEST("remove not found");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 999;
    ASSERT(zenit_map_remove(map, &key).error == ZENIT_ERROR_NOT_FOUND, "remove missing key");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: tombstone chain (insert, remove, insert again) ─── */
static int test_tombstone(void) {
    TEST("tombstone re-insert");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 5;
    int v1 = 55;
    int v2 = 66;
    ASSERT(zenit_map_insert(map, &key, &v1).error == ZENIT_OK, "first insert");
    ASSERT(zenit_map_remove(map, &key).error == ZENIT_OK, "remove");

    /* Re-insert the same key — should reuse the tombstone slot */
    ASSERT(zenit_map_insert(map, &key, &v2).error == ZENIT_OK, "re-insert");
    ASSERT(zenit_map_count(map) == 1, "count 1 after re-insert");

    int out = 0;
    ASSERT(zenit_map_get(map, &key, &out).error == ZENIT_OK, "get after re-insert");
    ASSERT(out == 66, "value should be 66");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: contains ─── */
static int test_contains(void) {
    TEST("contains");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int key = 10;
    int value = 20;
    ASSERT(zenit_map_contains(map, &key) == 0, "should not contain before insert");
    ASSERT(zenit_map_insert(map, &key, &value).error == ZENIT_OK, "insert");
    ASSERT(zenit_map_contains(map, &key) == 1, "should contain after insert");
    ASSERT(zenit_map_remove(map, &key).error == ZENIT_OK, "remove");
    ASSERT(zenit_map_contains(map, &key) == 0, "should not contain after remove");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: contains NULL map ─── */
static int test_contains_null(void) {
    TEST("contains NULL map returns 0");
    ASSERT(zenit_map_contains(NULL, NULL) == 0, "contains NULL should return 0");
    { int k = 1; ASSERT(zenit_map_contains(NULL, &k) == 0, "contains with NULL map"); }
    PASS();
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    TEST("clear");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_map_insert(map, &i, &i).error == ZENIT_OK, "insert");
    }
    ASSERT(zenit_map_count(map) == 10, "count 10");

    zenit_map_clear(map);
    ASSERT(zenit_map_count(map) == 0, "count 0 after clear");

    /* After clear, inserting should still work */
    int k = 1;
    int v = 2;
    ASSERT(zenit_map_insert(map, &k, &v).error == ZENIT_OK, "insert after clear");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: clear NULL is safe ─── */
static int test_clear_null(void) {
    TEST("clear NULL is safe");
    zenit_map_clear(NULL);
    PASS();
    return 0;
}

/* ─── Test: count NULL returns 0 ─── */
static int test_count_null(void) {
    TEST("count NULL returns 0");
    ASSERT(zenit_map_count(NULL) == 0, "count of NULL should be 0");
    PASS();
    return 0;
}

/* ─── Test: capacity NULL returns 0 ─── */
static int test_capacity_null(void) {
    TEST("capacity NULL returns 0");
    ASSERT(zenit_map_capacity(NULL) == 0, "capacity of NULL should be 0");
    PASS();
    return 0;
}

/* ─── Test: insert with NULL map ─── */
static int test_insert_null_map(void) {
    TEST("insert with NULL map");
    { int k = 1;
      int v = 2;
      ASSERT(zenit_map_insert(NULL, &k, &v).error == ZENIT_ERROR_NULL, "NULL map"); }
    PASS();
    return 0;
}

/* ─── Test: insert with NULL key ─── */
static int test_insert_null_key(void) {
    TEST("insert with NULL key");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    { int v = 2;
      ASSERT(zenit_map_insert(map, NULL, &v).error == ZENIT_ERROR_NULL, "NULL key"); }
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: insert with NULL value ─── */
static int test_insert_null_value(void) {
    TEST("insert with NULL value");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    { int k = 1;
      ASSERT(zenit_map_insert(map, &k, NULL).error == ZENIT_ERROR_NULL, "NULL value"); }
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: get with NULL map ─── */
static int test_get_null_map(void) {
    TEST("get with NULL map");
    { int k = 1;
      int v = 0;
      ASSERT(zenit_map_get(NULL, &k, &v).error == ZENIT_ERROR_NULL, "NULL map"); }
    PASS();
    return 0;
}

/* ─── Test: get with NULL key ─── */
static int test_get_null_key(void) {
    TEST("get with NULL key");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    { int v = 0;
      ASSERT(zenit_map_get(map, NULL, &v).error == ZENIT_ERROR_NULL, "NULL key"); }
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: get with NULL out_value ─── */
static int test_get_null_out(void) {
    TEST("get with NULL out_value");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    { int k = 1;
      ASSERT(zenit_map_get(map, &k, NULL).error == ZENIT_ERROR_NULL, "NULL out_value"); }
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: remove with NULL map ─── */
static int test_remove_null_map(void) {
    TEST("remove with NULL map");
    { int k = 1;
      ASSERT(zenit_map_remove(NULL, &k).error == ZENIT_ERROR_NULL, "NULL map"); }
    PASS();
    return 0;
}

/* ─── Test: remove with NULL key ─── */
static int test_remove_null_key(void) {
    TEST("remove with NULL key");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    ASSERT(zenit_map_remove(map, NULL).error == ZENIT_ERROR_NULL, "NULL key");
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: foreach ─── */
static int test_foreach(void) {
    TEST("foreach");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    for (int i = 0; i < 5; i++) {
        int val = i * 10;
        ASSERT(zenit_map_insert(map, &i, &val).error == ZENIT_OK, "insert");
    }

    visit_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    zenit_map_foreach(map, visit_int, &ctx);
    ASSERT(ctx.count == 5, "foreach should visit 5 entries");

    /* Verify we saw all keys and values */
    int seen_keys[5] = {0};
    int seen_values[5] = {0};
    for (size_t i = 0; i < ctx.count; i++) {
        int k = ctx.keys[i];
        int v = ctx.values[i];
        ASSERT(k >= 0 && k < 5, "key out of range");
        ASSERT(v == k * 10, "value mismatch");
        seen_keys[k] = 1;
        seen_values[k] = 1;
    }
    for (int i = 0; i < 5; i++) {
        ASSERT(seen_keys[i], "foreach missed key");
        ASSERT(seen_values[i], "foreach missed value");
    }

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: foreach NULL map ─── */
static int test_foreach_null(void) {
    TEST("foreach NULL map is safe");
    zenit_map_foreach(NULL, visit_int, NULL);
    PASS();
    return 0;
}

/* ─── Test: foreach NULL visit ─── */
static int test_foreach_null_visit(void) {
    TEST("foreach NULL visit is safe");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");
    zenit_map_foreach(map, NULL, NULL);
    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: many inserts (triggers rehash) ─── */
static int test_many_inserts(void) {
    TEST("many inserts (rehash)");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    size_t n = 1000;
    for (size_t i = 0; i < n; i++) {
        int key = (int)i;
        int value = (int)(i * 2);
        ASSERT(zenit_map_insert(map, &key, &value).error == ZENIT_OK, "insert");
    }
    ASSERT(zenit_map_count(map) == n, "count should match inserts");

    /* Verify all values */
    for (size_t i = 0; i < n; i++) {
        int key = (int)i;
        int out = 0;
        ASSERT(zenit_map_get(map, &key, &out).error == ZENIT_OK, "get after rehash");
        ASSERT(out == (int)(i * 2), "value mismatch after rehash");
    }

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: string keys (char* by pointer value) ─── */
static int test_string_keys(void) {
    TEST("string keys (pointer)");
    /* Map of char* -> int */
    zenit_map_t *map = zenit_map_create(sizeof(char *), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    char *k1 = "hello";
    char *k2 = "world";
    int v1 = 1;
    int v2 = 2;
    ASSERT(zenit_map_insert(map, &k1, &v1).error == ZENIT_OK, "insert hello");
    ASSERT(zenit_map_insert(map, &k2, &v2).error == ZENIT_OK, "insert world");
    ASSERT(zenit_map_count(map) == 2, "count 2");

    int out = 0;
    ASSERT(zenit_map_get(map, &k1, &out).error == ZENIT_OK, "get hello");
    ASSERT(out == 1, "value hello");
    ASSERT(zenit_map_get(map, &k2, &out).error == ZENIT_OK, "get world");
    ASSERT(out == 2, "value world");

    zenit_map_destroy(map);
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
    zenit_map_t *map = zenit_map_create(sizeof(point_t), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    point_t p1 = { 1, 2 };
    point_t p2 = { 3, 4 };
    int v1 = 100;
    int v2 = 200;

    ASSERT(zenit_map_insert(map, &p1, &v1).error == ZENIT_OK, "insert p1");
    ASSERT(zenit_map_insert(map, &p2, &v2).error == ZENIT_OK, "insert p2");

    int out = 0;
    ASSERT(zenit_map_get(map, &p1, &out).error == ZENIT_OK, "get p1");
    ASSERT(out == 100, "value p1");
    ASSERT(zenit_map_get(map, &p2, &out).error == ZENIT_OK, "get p2");
    ASSERT(out == 200, "value p2");

    /* Not found */
    point_t p3 = { 5, 6 };
    ASSERT(zenit_map_get(map, &p3, &out).error == ZENIT_ERROR_NOT_FOUND, "get p3 not found");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: insert then remove all then re-insert ─── */
static int test_remove_all_reinsert(void) {
    TEST("remove all then re-insert");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    int keys[10] = {0,1,2,3,4,5,6,7,8,9};
    int vals[10] = {0,10,20,30,40,50,60,70,80,90};

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_map_insert(map, &keys[i], &vals[i]).error == ZENIT_OK, "insert");
    }

    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_map_remove(map, &keys[i]).error == ZENIT_OK, "remove");
    }
    ASSERT(zenit_map_count(map) == 0, "count 0 after all removed");

    /* Re-insert */
    int new_vals[10] = {100,110,120,130,140,150,160,170,180,190};
    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_map_insert(map, &keys[i], &new_vals[i]).error == ZENIT_OK, "re-insert");
    }
    ASSERT(zenit_map_count(map) == 10, "count 10 after re-insert");

    int out = 0;
    for (int i = 0; i < 10; i++) {
        ASSERT(zenit_map_get(map, &keys[i], &out).error == ZENIT_OK, "get re-inserted");
        ASSERT(out == new_vals[i], "re-inserted value correct");
    }

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: count after insert and remove ─── */
static int test_count_ops(void) {
    TEST("count after insert/remove");
    zenit_map_t *map = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(map != NULL, "expected non-NULL map");

    ASSERT(zenit_map_count(map) == 0, "initial count 0");
    int k1 = 1;
    int v1 = 10;
    ASSERT(zenit_map_insert(map, &k1, &v1).error == ZENIT_OK, "insert");
    ASSERT(zenit_map_count(map) == 1, "count 1");

    int k2 = 2;
    int v2 = 20;
    ASSERT(zenit_map_insert(map, &k2, &v2).error == ZENIT_OK, "insert");
    ASSERT(zenit_map_count(map) == 2, "count 2");

    ASSERT(zenit_map_remove(map, &k1).error == ZENIT_OK, "remove");
    ASSERT(zenit_map_count(map) == 1, "count 1 after remove");

    zenit_map_destroy(map);
    PASS();
    return 0;
}

/* ─── Test: map iter ─── */
static int test_map_iter(void) {
    TEST("map_iter");
    zenit_map_t *m = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(m != NULL, "create");
    int k1 = 1;
    int v1 = 100;
    int k2 = 2;
    int v2 = 200;
    zenit_map_insert(m, &k1, &v1);
    zenit_map_insert(m, &k2, &v2);
    zenit_iter_t it = zenit_map_iter(m);
    int found = 0;
    void *key;
    while ((key = zenit_map_iter_next(&it)) != NULL) {
        int kv = *(int*)key;
        ASSERT(kv == 1 || kv == 2, "valid key");
        found++;
    }
    ASSERT(found == 2, "got both keys");
    /* NULL iter test */
    it = zenit_map_iter(NULL);
    ASSERT(zenit_map_iter_next(&it) == NULL, "NULL map iter");
    zenit_map_destroy(m);
    PASS();
    return 0;
}

/* ─── Test: map keys ─── */
static int test_map_keys(void) {
    TEST("map_keys");
    zenit_map_t *m = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(m != NULL, "create");
    int k1 = 1;
    int v1 = 100;
    int k2 = 2;
    int v2 = 200;
    zenit_map_insert(m, &k1, &v1);
    zenit_map_insert(m, &k2, &v2);
    int *keys = NULL;
    size_t count = 0;
    zenit_result_t r = zenit_map_keys(m, (void**)&keys, &count);
    ASSERT(r.error == ZENIT_OK, "keys ok");
    ASSERT(count == 2, "2 keys");
    ASSERT(keys != NULL, "keys allocated");
    /* keys should be 1 and 2 in some order */
    int found1 = 0;
    int found2 = 0;
    for (size_t i = 0; i < count; i++) {
        if (keys[i] == 1) found1 = 1;
        if (keys[i] == 2) found2 = 1;
    }
    ASSERT(found1 && found2, "correct keys");
    free(keys);
    /* NULL params */
    ASSERT(zenit_map_keys(NULL, (void**)&keys, &count).error == ZENIT_ERROR_NULL, "NULL map");
    ASSERT(zenit_map_keys(m, NULL, &count).error == ZENIT_ERROR_NULL, "NULL out_keys");
    ASSERT(zenit_map_keys(m, (void**)&keys, NULL).error == ZENIT_ERROR_NULL, "NULL out_count");
    /* Empty map */
    zenit_map_t *empty = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(empty != NULL, "empty create");
    keys = NULL; count = 99;
    r = zenit_map_keys(empty, (void**)&keys, &count);
    ASSERT(r.error == ZENIT_OK, "empty keys ok");
    ASSERT(count == 0, "0 keys");
    ASSERT(keys == NULL, "NULL keys for empty");
    zenit_map_destroy(empty);
    zenit_map_destroy(m);
    PASS();
    return 0;
}

/* ─── Test: map values ─── */
static int test_create_with_allocator(void) {
    TEST("create_with_allocator");
    zenit_map_t *m = zenit_map_create_with_allocator(sizeof(int), sizeof(int), ZENIT_ALLOCATOR_DEFAULT);
    ASSERT(m != NULL, "create");
    zenit_map_destroy(m);
    PASS();
    return 0;
}

static int test_map_values(void) {
    TEST("map_values");
    zenit_map_t *m = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(m != NULL, "create");
    int k1 = 1;
    int v1 = 100;
    int k2 = 2;
    int v2 = 200;
    zenit_map_insert(m, &k1, &v1);
    zenit_map_insert(m, &k2, &v2);
    int *values = NULL;
    size_t count = 0;
    zenit_result_t r = zenit_map_values(m, (void**)&values, &count);
    ASSERT(r.error == ZENIT_OK, "values ok");
    ASSERT(count == 2, "2 values");
    int found1 = 0;
    int found2 = 0;
    for (size_t i = 0; i < count; i++) {
        if (values[i] == 100) found1 = 1;
        if (values[i] == 200) found2 = 1;
    }
    ASSERT(found1 && found2, "correct values");
    free(values);
    /* NULL params */
    ASSERT(zenit_map_values(NULL, (void**)&values, &count).error == ZENIT_ERROR_NULL, "NULL map");
    ASSERT(zenit_map_values(m, NULL, &count).error == ZENIT_ERROR_NULL, "NULL out_values");
    ASSERT(zenit_map_values(m, (void**)&values, NULL).error == ZENIT_ERROR_NULL, "NULL out_count");
    zenit_map_destroy(m);
    PASS();
    return 0;
}

static int test_map_keys_free_func(void) {
    TEST("map_keys_free");
    zenit_map_t *m = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(m != NULL, "create");
    int k1 = 1;
    int v1 = 100;
    zenit_map_insert(m, &k1, &v1);
    int *keys = NULL;
    size_t count = 0;
    ASSERT(zenit_map_keys(m, (void**)&keys, &count).error == ZENIT_OK, "keys");
    ASSERT(zenit_map_keys_free(m, keys, count).error == ZENIT_OK, "keys_free");
    ASSERT(zenit_map_keys_free(NULL, keys, count).error == ZENIT_ERROR_NULL, "keys_free NULL map");
    ASSERT(zenit_map_keys_free(m, NULL, count).error == ZENIT_ERROR_NULL, "keys_free NULL keys");
    zenit_map_destroy(m);
    PASS();
    return 0;
}

static int test_map_values_free_func(void) {
    TEST("map_values_free");
    zenit_map_t *m = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(m != NULL, "create");
    int k1 = 1;
    int v1 = 100;
    zenit_map_insert(m, &k1, &v1);
    int *values = NULL;
    size_t count = 0;
    ASSERT(zenit_map_values(m, (void**)&values, &count).error == ZENIT_OK, "values");
    ASSERT(zenit_map_values_free(m, values, count).error == ZENIT_OK, "values_free");
    ASSERT(zenit_map_values_free(NULL, values, count).error == ZENIT_ERROR_NULL, "values_free NULL map");
    ASSERT(zenit_map_values_free(m, NULL, count).error == ZENIT_ERROR_NULL, "values_free NULL values");
    zenit_map_destroy(m);
    PASS();
    return 0;
}

static int test_values_empty(void) {
    TEST("map_values empty");
    zenit_map_t *m = zenit_map_create(sizeof(int), sizeof(int));
    ASSERT(m != NULL, "create");
    int *values = NULL;
    size_t count = 99;
    zenit_result_t r = zenit_map_values(m, (void**)&values, &count);
    ASSERT(r.error == ZENIT_OK, "empty values ok");
    ASSERT(count == 0, "0 values");
    ASSERT(values == NULL, "NULL values");
    zenit_map_destroy(m);
    PASS();
    return 0;
}

int main(void) {
    TEST_ENTRY tests[] = {
        { test_create_destroy,      "create_destroy" },
        { test_create_with_capacity, "create_with_capacity" },
        { test_create_zero_params,  "create_zero_params" },
        { test_create_zero_value,   "create_zero_value" },
        { test_create_zero_capacity, "create_zero_capacity" },
        { test_null_destroy,        "null_destroy" },
        { test_insert_get,          "insert_get" },
        { test_insert_overwrite,    "insert_overwrite" },
        { test_get_not_found,       "get_not_found" },
        { test_remove,              "remove" },
        { test_remove_not_found,    "remove_not_found" },
        { test_tombstone,           "tombstone" },
        { test_contains,            "contains" },
        { test_contains_null,       "contains_null" },
        { test_clear,               "clear" },
        { test_clear_null,          "clear_null" },
        { test_count_null,          "count_null" },
        { test_capacity_null,       "capacity_null" },
        { test_insert_null_map,     "insert_null_map" },
        { test_insert_null_key,     "insert_null_key" },
        { test_insert_null_value,   "insert_null_value" },
        { test_get_null_map,        "get_null_map" },
        { test_get_null_key,        "get_null_key" },
        { test_get_null_out,        "get_null_out" },
        { test_remove_null_map,     "remove_null_map" },
        { test_remove_null_key,     "remove_null_key" },
        { test_foreach,             "foreach" },
        { test_foreach_null,        "foreach_null" },
        { test_foreach_null_visit,  "foreach_null_visit" },
        { test_many_inserts,        "many_inserts" },
        { test_string_keys,         "string_keys" },
        { test_struct_keys,         "struct_keys" },
        { test_remove_all_reinsert, "remove_all_reinsert" },
        { test_count_ops,           "count_ops" },
        { test_map_iter,            "map_iter" },
        { test_map_keys,            "map_keys" },
        { test_map_values,          "map_values" },
        { test_values_empty,        "map_values empty" },
        { test_map_keys_free_func,  "map_keys_free" },
        { test_map_values_free_func, "map_values_free" },
        { test_create_with_allocator, "create_with_allocator" },
        { 0, 0 }
    };
    return test_run_all("hash map", tests);
}
