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

#define TEST(label) do { printf("  %s ... ", label); fflush(stdout); } while(0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); failures++; } while(0)

static zenit_json_t *doc_create(void) {
    malloc_fail_countdown = -1;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); }
    return doc;
}

static void test_create_fail(void) {
    TEST("create on malloc fail");
    malloc_fail_countdown = 0;
    zenit_json_t *doc = zenit_json_create();
    malloc_fail_countdown = -1;
    if (doc != NULL) { FAIL("expected NULL"); zenit_json_destroy(doc); }
    printf("%s\n", doc == NULL ? "OK" : "");
}

static void test_parse_fail(void) {
    TEST("parse on malloc fail");
    malloc_fail_countdown = 0;
    zenit_json_t *doc = zenit_json_parse("null");
    malloc_fail_countdown = -1;
    if (doc != NULL) { FAIL("expected NULL"); zenit_json_destroy(doc); }
    printf("%s\n", doc == NULL ? "OK" : "");
}

static void test_value_construct_fail(void) {
    TEST("value constructor on malloc fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    malloc_fail_countdown = 0;
    const zenit_json_value_t *val = zenit_json_value_number(doc, 42.0);
    malloc_fail_countdown = -1;
    if (val != NULL) { FAIL("value should be NULL"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_string_value_fail(void) {
    TEST("string value with malloc fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    malloc_fail_countdown = 1;
    const zenit_json_value_t *val = zenit_json_value_string(doc, "hello");
    malloc_fail_countdown = -1;
    if (val != NULL) { FAIL("value should be NULL"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_string_value_struct_fail(void) {
    TEST("string value struct on malloc fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    malloc_fail_countdown = 0;
    const zenit_json_value_t *val = zenit_json_value_string(doc, "hello");
    malloc_fail_countdown = -1;
    if (val != NULL) { FAIL("value should be NULL"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_array_append_grow_fail(void) {
    TEST("array append grow fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    zenit_json_value_t *arr = zenit_json_value_array(doc);
    if (arr == NULL) { FAIL("create arr"); zenit_json_destroy(doc); printf("\n"); return; }
    zenit_json_value_t *item = zenit_json_value_number(doc, 1.0);
    if (item == NULL) { FAIL("create item"); zenit_json_destroy(doc); printf("\n"); return; }
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_array_append(arr, item);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { FAIL("expected ALLOC"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_array_insert_grow_fail(void) {
    TEST("array insert grow fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    zenit_json_value_t *arr = zenit_json_value_array(doc);
    if (arr == NULL) { FAIL("create arr"); zenit_json_destroy(doc); printf("\n"); return; }
    for (int i = 0; i < 8; i++) {
        zenit_json_value_t *n = zenit_json_value_number(doc, (double)i);
        if (n == NULL) { FAIL("create num"); zenit_json_destroy(doc); printf("\n"); return; }
        if (zenit_json_array_append(arr, n).error != ZENIT_OK) { FAIL("append"); zenit_json_destroy(doc); printf("\n"); return; }
    }
    zenit_json_value_t *insert_item = zenit_json_value_number(doc, -1.0);
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_array_insert(arr, 0, insert_item);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { FAIL("expected ALLOC"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_object_set_key_fail(void) {
    TEST("object set key fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { FAIL("create obj"); zenit_json_destroy(doc); printf("\n"); return; }
    zenit_json_value_t *v = zenit_json_value_number(doc, 1.0);
    if (v == NULL) { FAIL("create val"); zenit_json_destroy(doc); printf("\n"); return; }
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_object_set(obj, "a", v);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { FAIL("expected ALLOC"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void fill_object(zenit_json_value_t *obj, zenit_json_t *doc, int n) {
    for (int i = 0; i < n; i++) {
        char key[8];
        snprintf(key, sizeof(key), "k%d", i);
        zenit_json_value_t *nv = zenit_json_value_number(doc, (double)i);
        if (nv == NULL) { FAIL("create val"); return; }
        if (zenit_json_object_set(obj, key, nv).error != ZENIT_OK) { FAIL("set"); return; }
    }
}

static void test_object_set_grow_fail(void) {
    TEST("object set grow fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { FAIL("create obj"); zenit_json_destroy(doc); printf("\n"); return; }
    fill_object(obj, doc, 8);
    zenit_json_value_t *v9 = zenit_json_value_number(doc, 8.0);
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_json_object_set(obj, "k8", v9);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { FAIL("expected ALLOC on grow"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_object_grow_rollback(void) {
    TEST("object grow rollback");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { FAIL("create obj"); zenit_json_destroy(doc); printf("\n"); return; }
    fill_object(obj, doc, 8);
    zenit_json_value_t *v9 = zenit_json_value_number(doc, 8.0);
    malloc_fail_countdown = 1;
    zenit_result_t r = zenit_json_object_set(obj, "k8", v9);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { FAIL("expected ALLOC on rollback"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_object_set_key_copy_fail(void) {
    TEST("object set key copy fail");
    zenit_json_t *doc = doc_create();
    if (doc == NULL) { printf("\n"); return; }
    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { FAIL("create obj"); zenit_json_destroy(doc); printf("\n"); return; }
    zenit_json_value_t *v = zenit_json_value_number(doc, 1.0);
    if (v == NULL) { FAIL("create val"); zenit_json_destroy(doc); printf("\n"); return; }
    malloc_fail_countdown = 2;
    zenit_result_t r = zenit_json_object_set(obj, "mykey", v);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) { FAIL("expected ALLOC on key copy"); }
    zenit_json_destroy(doc);
    printf("OK\n");
}

static void test_serialize_oom(const char *label, char *(*fn)(const zenit_json_value_t *)) {
    TEST(label);
    int oom_hits = 0;
    for (int cd = 0; cd < 60; cd++) {
        malloc_fail_countdown = -1;
        zenit_json_t *doc = zenit_json_parse("[1,2,3,4,5,6,7,8,9]");
        if (doc == NULL) { continue; }
        malloc_fail_countdown = cd;
        char *out = fn(zenit_json_root(doc));
        malloc_fail_countdown = -1;
        if (out == NULL) { oom_hits++; }
        free(out);
        zenit_json_destroy(doc);
    }
    if (oom_hits == 0) { FAIL("no OOM hit"); }
    printf("OK (%d OOM hits)\n", oom_hits);
}

static void test_serialize_fail(void) {
    test_serialize_oom("serialize on malloc fail", zenit_json_value_serialize);
}

static void test_serialize_value_fail(void) {
    test_serialize_oom("value serialize on malloc fail", zenit_json_value_serialize);
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
