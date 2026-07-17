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

#include <libzenit/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_malloc_fail.h"

void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_realloc(ptr, size);
}

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "FAIL: %s\n", msg); failures++; return; } \
} while(0)

static void test_create_fail(void) {
    printf("  create on malloc fail ... ");
    fflush(stdout);
    malloc_fail_countdown = 0;
    zenit_json_t *doc = zenit_json_create();
    malloc_fail_countdown = -1;
    if (doc != NULL) { fprintf(stderr, "FAIL: expected NULL\n"); failures++; zenit_json_destroy(doc); return; }
    printf("OK\n");
}

static void test_parse_fail(void) {
    printf("  parse on malloc fail ... ");
    fflush(stdout);
    malloc_fail_countdown = 0;
    zenit_json_t *doc = zenit_json_parse("null");
    malloc_fail_countdown = -1;
    if (doc != NULL) { fprintf(stderr, "FAIL: expected NULL\n"); failures++; zenit_json_destroy(doc); return; }
    printf("OK\n");
}

/* Create a doc first, then fail on value construction */
static void test_value_construct_fail(void) {
    printf("  value constructor on malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    /* Try value creation with countdown starting at specific values.
     * countdown=1: doc create already consumed one success, next alloc = value struct = fail */
    /* But wait, with countdown=1 already consumed for doc creation, the next
     * value_alloc will decrement to 0 and fail. Let me count:
     * After doc creation with countdown=-1, we set countdown=1.
     * zenit_json_value_number -> value_alloc -> alloc_fn -> __wrap_malloc -> countdown=1>0, dec to 0, real_malloc succeeds
     * That's not what we want - we want value_alloc to fail.
     * So countdown=0 should be set before calling the value constructor.
     * But countdown=0 was already used above... ah, countdown was reset to -1 after doc creation.
     * So: set countdown=0, then call value_alloc -> __wrap_malloc -> returns NULL. */
    malloc_fail_countdown = 0;
    zenit_json_value_t *val = zenit_json_value_number(doc, 42.0);
    malloc_fail_countdown = -1;
    if (val != NULL) { fprintf(stderr, "FAIL: value should be NULL\n"); failures++; zenit_json_destroy(doc); return; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test string creation with malloc fail */
static void test_string_value_fail(void) {
    printf("  string value with malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    /* First alloc: value struct (succeeds, countdown goes 1→0)
     * Second alloc: string copy (countdown=0, fails) */
    malloc_fail_countdown = 1;
    zenit_json_value_t *val = zenit_json_value_string(doc, "hello");
    malloc_fail_countdown = -1;
    if (val != NULL) { fprintf(stderr, "FAIL: value should be NULL\n"); failures++; zenit_json_destroy(doc); return; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test string value where value struct fails (1st alloc), not the copy */
static void test_string_value_struct_fail(void) {
    printf("  string value struct on malloc fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    /* Set countdown=0 so the very next malloc (value struct) fails */
    malloc_fail_countdown = 0;
    zenit_json_value_t *val = zenit_json_value_string(doc, "hello");
    malloc_fail_countdown = -1;
    if (val != NULL) { fprintf(stderr, "FAIL: value should be NULL\n"); failures++; zenit_json_destroy(doc); return; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test array append with malloc fail (array growth) */
static void test_array_append_grow_fail(void) {
    printf("  array append grow fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    zenit_json_value_t *arr = zenit_json_value_array(doc);
    if (arr == NULL) { fprintf(stderr, "FAIL: create arr\n"); failures++; zenit_json_destroy(doc); return; }

    /* Create the item first so it doesn't interfere with countdown */
    zenit_json_value_t *item = zenit_json_value_number(doc, 1.0);
    if (item == NULL) { fprintf(stderr, "FAIL: create item\n"); failures++; zenit_json_destroy(doc); return; }

    /* Append one item (initial capacity=0, growth to 8 → needs allocation).
     * The array_grow call inside append triggers the first alloc. */
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_array_append(arr, item);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error); failures++; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test array insert with malloc fail */
static void test_array_insert_grow_fail(void) {
    printf("  array insert grow fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    /* Create array, append an item to trigger first growth (0→8) */
    zenit_json_value_t *arr = zenit_json_value_array(doc);
    if (arr == NULL) { fprintf(stderr, "FAIL: create arr\n"); failures++; zenit_json_destroy(doc); return; }

    /* Append first item succeeds (growth from 0 to 8) */
    if (zenit_json_array_append(arr, zenit_json_value_number(doc, 0.0)).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: first append\n"); failures++; zenit_json_destroy(doc); return;
    }

    /* Now insert at 0 — bumps count to 2, still within 8, no growth needed.
     * Need to reach capacity=8 and then insert to trigger growth failure.
     * Let's append 7 more items to fill the array (count 1→8). */
    for (int i = 1; i < 8; i++) {
        zenit_json_value_t *n = zenit_json_value_number(doc, (double)i);
        if (n == NULL) { fprintf(stderr, "FAIL: create num %d\n", i); failures++; zenit_json_destroy(doc); return; }
        if (zenit_json_array_append(arr, n).error != ZENIT_OK) {
            fprintf(stderr, "FAIL: append %d\n", i); failures++; zenit_json_destroy(doc); return;
        }
    }

    /* Create insert item beforehand */
    zenit_json_value_t *insert_item = zenit_json_value_number(doc, -1.0);

    /* Now count=8, capacity=8. Next insert needs growth. */
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_array_insert(arr, 0, insert_item);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { fprintf(stderr, "FAIL: expected ALLOC\n"); failures++; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test object set with malloc fail (key string allocation) */
static void test_object_set_key_fail(void) {
    printf("  object set key fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { fprintf(stderr, "FAIL: create obj\n"); failures++; zenit_json_destroy(doc); return; }

    /* Create value beforehand */
    zenit_json_value_t *v = zenit_json_value_number(doc, 1.0);
    if (v == NULL) { fprintf(stderr, "FAIL: create val\n"); failures++; zenit_json_destroy(doc); return; }

    /* Set a key — first alloc is object_grow (keys array), which should fail */
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_object_set(obj, "a", v);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { fprintf(stderr, "FAIL: expected ALLOC, got %d\n", r.error); failures++; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test object set where key dup succeeds but object_grow first alloc fails */
static void test_object_set_grow_fail(void) {
    printf("  object set grow fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { fprintf(stderr, "FAIL: create obj\n"); failures++; zenit_json_destroy(doc); return; }

    /* First, add enough items so we're at capacity (0→8 after first add).
     * To trigger a grow failure, we need count=capacity, then another set.
     * First set: capacity 0→8, adds key. Now count=1, cap=8.
     * Need 7 more sets to reach cap=8, then 1 more to trigger growth.
     * Let's just fill the array to 8 items then try to fail on growth. */

    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "k%d", i);
        zenit_json_value_t *nv = zenit_json_value_number(doc, (double)i);
        if (nv == NULL) { fprintf(stderr, "FAIL: create val %d\n", i); failures++; zenit_json_destroy(doc); return; }
        if (zenit_json_object_set(obj, key, nv).error != ZENIT_OK) {
            fprintf(stderr, "FAIL: prep set %d\n", i); failures++; zenit_json_destroy(doc); return;
        }
    }

    /* Create the 9th value beforehand */
    zenit_json_value_t *v9 = zenit_json_value_number(doc, 8.0);

    /* Now count=8, capacity=8. Next set triggers object_grow. */
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_object_set(obj, "k8", v9);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { fprintf(stderr, "FAIL: expected ALLOC on grow\n"); failures++; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test object_grow where keys alloc succeeds but values alloc fails (lines 232-235) */
static void test_object_grow_rollback(void) {
    printf("  object grow rollback ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { fprintf(stderr, "FAIL: create obj\n"); failures++; zenit_json_destroy(doc); return; }

    /* Fill to capacity=8 */
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "k%d", i);
        zenit_json_value_t *nv = zenit_json_value_number(doc, (double)i);
        if (nv == NULL) { fprintf(stderr, "FAIL: create val %d\n", i); failures++; zenit_json_destroy(doc); return; }
        if (zenit_json_object_set(obj, key, nv).error != ZENIT_OK) {
            fprintf(stderr, "FAIL: prep set %d\n", i); failures++; zenit_json_destroy(doc); return;
        }
    }

    /* Create the 9th value beforehand */
    zenit_json_value_t *v9 = zenit_json_value_number(doc, 8.0);

    /* countdown=1: keys alloc succeeds (1→0), values alloc fails (0→NULL) */
    malloc_fail_countdown = 1;
    zenit_result_t r = zenit_json_object_set(obj, "k8", v9);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { fprintf(stderr, "FAIL: expected ALLOC on rollback, got %d\n", r.error); failures++; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Test object_set where key_copy fails (line 980) after grow succeeds */
static void test_object_set_key_copy_fail(void) {
    printf("  object set key copy fail ... ");
    fflush(stdout);

    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { fprintf(stderr, "FAIL: create doc\n"); failures++; return; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { fprintf(stderr, "FAIL: create obj\n"); failures++; zenit_json_destroy(doc); return; }

    zenit_json_value_t *v = zenit_json_value_number(doc, 1.0);
    if (v == NULL) { fprintf(stderr, "FAIL: create val\n"); failures++; zenit_json_destroy(doc); return; }

    /* For the FIRST set on an empty object:
     * object_grow: capacity=0 → 8, needs 2 allocs (keys + values) or 1 realloc
     * Actually capacity=0 → both keys and values are NULL → 2 alloc_fn calls.
     * countdown=2: keys alloc succeeds (2→1), values alloc succeeds (1→0),
     *              then str_dup(key) fails (0→NULL)
     * That should work! */
    malloc_fail_countdown = 2;
    zenit_result_t r = zenit_json_object_set(obj, "mykey", v);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { fprintf(stderr, "FAIL: expected ALLOC on key copy, got %d\n", r.error); failures++; }

    zenit_json_destroy(doc);
    printf("OK\n");
}

/* Try serialize with many countdown values to hit every OOM path */
static void test_serialize_fail(void) {
    printf("  serialize on malloc fail ... ");
    fflush(stdout);

    int oom_hits = 0;
    for (int cd = 0; cd < 60; cd++) {
        malloc_fail_countdown = -1;
        zenit_json_t *doc = zenit_json_parse("[1,2,3,4,5,6,7,8,9]");
        if (doc == NULL) { continue; }

        malloc_fail_countdown = cd;
        char *out = zenit_json_serialize(doc);
        malloc_fail_countdown = -1;

        if (out == NULL) {
            oom_hits++;
        }
        free(out);
        zenit_json_destroy(doc);
    }

    if (oom_hits == 0) {
        fprintf(stderr, "FAIL: no OOM hit\n");
        failures++;
    }
    printf("OK (%d OOM hits)\n", oom_hits);
}

static void test_serialize_value_fail(void) {
    printf("  value serialize on malloc fail ... ");
    fflush(stdout);

    int oom_hits = 0;
    for (int cd = 0; cd < 60; cd++) {
        malloc_fail_countdown = -1;
        zenit_json_t *doc = zenit_json_parse("[1,2,3,4,5,6,7,8,9]");
        if (doc == NULL) { continue; }

        malloc_fail_countdown = cd;
        char *out = zenit_json_value_serialize(zenit_json_root(doc));
        malloc_fail_countdown = -1;

        if (out == NULL) {
            oom_hits++;
        }
        free(out);
        zenit_json_destroy(doc);
    }

    if (oom_hits == 0) {
        fprintf(stderr, "FAIL: no OOM hit\n");
        failures++;
    }
    printf("OK (%d OOM hits)\n", oom_hits);
}

int main(void) {
    malloc_fail_countdown = -1;

    test_create_fail();
    test_parse_fail();
    test_value_construct_fail();
    test_string_value_fail();
    test_string_value_struct_fail();
    test_array_append_grow_fail();
    test_array_insert_grow_fail();
    test_object_set_key_fail();
    test_object_set_grow_fail();
    test_object_grow_rollback();
    test_object_set_key_copy_fail();
    test_serialize_fail();
    test_serialize_value_fail();

    malloc_fail_countdown = -1;

    if (failures > 0) {
        fprintf(stderr, "\nFAILED: %d test(s) failed\n", failures);
        return 1;
    }
    printf("\nPASSED\n");
    return 0;
}
