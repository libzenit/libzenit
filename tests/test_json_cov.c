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

/*
 * JSON coverage edge-case tests.
 *
 * Uses a custom allocator with configurable failure point to reach every
 * OOM error path in the JSON parser, serializer, and value constructors.
 */

#include <libzenit/json.h>
#include <libzenit/result.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Custom allocator that fails after N allocations ─── */

static int cov_alloc_count = 0;
static int cov_fail_after = -1;

static void *cov_alloc(size_t size, void *ctx) {
    (void)ctx;
    if (cov_fail_after >= 0 && cov_alloc_count >= cov_fail_after) {
        return NULL;
    }
    cov_alloc_count++;
    return malloc(size);
}

static void *cov_realloc(void *ptr, size_t size, void *ctx) {
    (void)ctx;
    if (cov_fail_after >= 0 && cov_alloc_count >= cov_fail_after) {
        return NULL;
    }
    cov_alloc_count++;
    return realloc(ptr, size);
}

static void cov_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

static zenit_allocator_t cov_allocator(void) {
    return (zenit_allocator_t){
        .alloc_fn = cov_alloc,
        .realloc_fn = cov_realloc,
        .free_fn = cov_free,
        .ctx = NULL
    };
}

/* Try all countdown values from 0 up to limit; parse should gracefully
 * return NULL at every value (no crash or leak). */
static void try_parse_countdown(const char *input, int limit) {
    for (int i = 0; i <= limit; i++) {
        cov_alloc_count = 0;
        cov_fail_after = i;
        zenit_json_t *doc = zenit_json_parse_with_length_and_allocator(input, strlen(input), cov_allocator());
        cov_fail_after = -1;
        if (doc != NULL) {
            zenit_json_destroy(doc);
        }
    }
}

/* Try to hit string realloc OOM during parse (lines 373-375).
 * A string > 64 chars triggers buffer realloc; we make it fail. */
static void try_long_string_oom(void) {
    char input[150];
    input[0] = '"';
    for (int i = 0; i < 100; i++) {
        input[1 + i] = 'x';
    }
    input[101] = '"';
    input[102] = '\0';

    for (int i = 0; i < 15; i++) {
        cov_alloc_count = 0;
        cov_fail_after = i;
        zenit_json_t *doc = zenit_json_parse_with_length_and_allocator(input, strlen(input), cov_allocator());
        cov_fail_after = -1;
        if (doc != NULL) {
            zenit_json_destroy(doc);
        }
    }
}

/* Same for programmatic construction. */
static void try_build_countdown(int limit) {
    for (int i = 0; i <= limit; i++) {
        cov_alloc_count = 0;
        cov_fail_after = i;
        zenit_json_t *doc = zenit_json_create_with_allocator(cov_allocator());
        cov_fail_after = -1;
        if (doc != NULL) {
            zenit_json_destroy(doc);
        }
    }
}

/* ─── Static counters ─── */

static int failures = 0;
#define TEST(name) do { printf("  %s ... ", name); fflush(stdout); } while (0)
#define PASS() do { printf("PASS\n"); } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); failures++; return; } while (0)

/* ─── Test: hit every OOM path in parser ─── */

static void test_oom_parse(void) {
    TEST("OOM parse countdown 0-18");
    try_parse_countdown("null", 6);
    try_parse_countdown("\"hello\"", 9);
    try_parse_countdown("[1,2,3,4,5,6,7,8,9]", 20);
    try_parse_countdown("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5,\"f\":6,\"g\":7,\"h\":8,\"i\":9}", 30);
    try_long_string_oom();
    PASS();
}

/* ─── Test: hit OOM paths in programmatic construction ─── */

static void test_oom_build(void) {
    TEST("OOM build countdown 0-10");
    try_build_countdown(10);

    /* Also build with specific value construction failures */
    for (int i = 0; i < 20; i++) {
        cov_alloc_count = 0;
        cov_fail_after = i;
        zenit_json_t *doc = zenit_json_create_with_allocator(cov_allocator());
        cov_fail_after = -1;
        if (doc != NULL) {
            /* Try operations that trigger additional allocations */
            zenit_json_value_t *v = zenit_json_value_null(doc);
            (void)v;
            /* If we got here, try more */
            v = zenit_json_value_bool(doc, 1);
            (void)v;
            v = zenit_json_value_number(doc, 1.0);
            (void)v;
            v = zenit_json_value_string(doc, "test");
            (void)v;
            v = zenit_json_value_array(doc);
            (void)v;
            v = zenit_json_value_object(doc);
            (void)v;
            zenit_json_destroy(doc);
        }
    }
    PASS();
}

/* ─── Test: NULL doc constructors ─── */

static void test_null_doc(void) {
    TEST("NULL doc constructors");
    if (zenit_json_value_null(NULL) != NULL) { FAIL("null"); return; }
    if (zenit_json_value_bool(NULL, 1) != NULL) { FAIL("bool"); return; }
    if (zenit_json_value_number(NULL, 1.0) != NULL) { FAIL("number"); return; }
    if (zenit_json_value_string(NULL, "x") != NULL) { FAIL("string"); return; }
    if (zenit_json_value_array(NULL) != NULL) { FAIL("array"); return; }
    if (zenit_json_value_object(NULL) != NULL) { FAIL("object"); return; }
    PASS();
}

/* ─── Test: edge case operations ─── */

static void test_edge_ops(void) {
    TEST("edge case operations");
    /* array_remove NULL */
    if (zenit_json_array_remove(NULL, 0).error == ZENIT_OK) { FAIL("remove NULL"); return; }
    /* array_insert NULL */
    if (zenit_json_array_insert(NULL, 0, NULL).error == ZENIT_OK) { FAIL("insert NULL"); return; }
    /* object_remove NULL */
    if (zenit_json_object_remove(NULL, "x").error == ZENIT_OK) { FAIL("obj remove NULL"); return; }

    /* Create doc for type-mismatch tests */
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }
    zenit_json_value_t *num = zenit_json_value_number(doc, 1.0);
    if (num == NULL) { FAIL("create num"); zenit_json_destroy(doc); return; }

    /* Array ops on non-array */
    if (zenit_json_array_remove(num, 0).error == ZENIT_OK) { FAIL("remove on num"); zenit_json_destroy(doc); return; }
    if (zenit_json_array_insert(num, 0, num).error == ZENIT_OK) { FAIL("insert on num"); zenit_json_destroy(doc); return; }

    /* Object ops on non-object */
    if (zenit_json_object_remove(num, "x").error == ZENIT_OK) { FAIL("remove on num"); zenit_json_destroy(doc); return; }

    zenit_json_destroy(doc);
    PASS();
}

/* ─── Test: serialise false (hit the FALSE branch) ─── */

static void test_serialize_false(void) {
    TEST("serialize false");
    zenit_json_t *doc = zenit_json_parse("false");
    if (doc == NULL) { FAIL("parse NULL"); return; }
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    if (s == NULL || strcmp(s, "false") != 0) { FAIL("wrong"); free(s); return; }
    free(s);
    PASS();
}

/* ─── Test: serialise control characters ─── */

static void test_serialize_control(void) {
    TEST("serialize control chars");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create"); return; }

    /* String with \b, \f, \n, \r, \t */
    zenit_json_value_t *sv = zenit_json_value_string(doc, "a\bb\fc\nd\re\tf");
    if (sv == NULL) { FAIL("create str"); zenit_json_destroy(doc); return; }
    zenit_json_set_root(doc, sv);
    char *out = zenit_json_serialize(doc);
    if (out == NULL) { FAIL("serialize NULL"); zenit_json_destroy(doc); return; }
    if (strcmp(out, "\"a\\bb\\fc\\nd\\re\\tf\"") != 0) { FAIL("wrong"); free(out); zenit_json_destroy(doc); return; }
    free(out);
    zenit_json_destroy(doc);

    /* String with \x01 (control char requiring \uXXXX escape) */
    doc = zenit_json_create();
    if (doc == NULL) { FAIL("create"); return; }
    char ctrl[2] = { 0x01, 0 };
    sv = zenit_json_value_string(doc, ctrl);
    if (sv == NULL) { FAIL("create ctrl str"); zenit_json_destroy(doc); return; }
    zenit_json_set_root(doc, sv);
    out = zenit_json_serialize(doc);
    if (out == NULL) { FAIL("serialize ctrl NULL"); zenit_json_destroy(doc); return; }
    if (strcmp(out, "\"\\u0001\"") != 0) { FAIL("wrong ctrl"); free(out); zenit_json_destroy(doc); return; }
    free(out);
    zenit_json_destroy(doc);
    PASS();
}

/* ─── Test: parse errors (trailing comma, bad separator, etc.) ─── */

static void test_parse_errors(void) {
    TEST("parse error edge cases");
    const char *invalid[] = {
        "[1,]",
        "[1 2]",
        "{\"a\":1,}",
        "{\"a\" 1}",
        "{\"a\":1 \"b\":2}",
        "{\"a\":}",
        "null extra",
        "\"\\",
        "\"\\u",
    };
    for (int i = 0; i < (int)(sizeof(invalid) / sizeof(invalid[0])); i++) {
        zenit_json_t *doc = zenit_json_parse(invalid[i]);
        if (doc != NULL) { FAIL("expected NULL"); zenit_json_destroy(doc); return; }
    }
    PASS();
}

/* ─── Test: string value with NULL str param ─── */

static void test_string_null(void) {
    TEST("string value NULL str");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create"); return; }
    zenit_json_value_t *v = zenit_json_value_string(doc, NULL);
    if (v == NULL) { FAIL("value NULL"); zenit_json_destroy(doc); return; }
    const char *s = zenit_json_value_get_string(v);
    if (s == NULL || strcmp(s, "") != 0) { FAIL("wrong"); zenit_json_destroy(doc); return; }
    zenit_json_destroy(doc);
    PASS();
}

/* ─── Test: parse long string ─── */

static void test_long_string(void) {
    TEST("parse long string (>64 chars)");
    char input[256];
    char expected[256];
    input[0] = '"';
    for (int i = 0; i < 100; i++) {
        input[1 + i] = 'A' + (i % 26);
        expected[i] = 'A' + (i % 26);
    }
    input[101] = '"';
    input[102] = '\0';
    expected[100] = '\0';

    zenit_json_t *doc = zenit_json_parse(input);
    if (doc == NULL) { FAIL("parse NULL"); return; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, expected) != 0) { FAIL("wrong"); zenit_json_destroy(doc); return; }
    zenit_json_destroy(doc);
    PASS();
}

/* ─── Main ─── */

int main(void) {
    printf("=== json coverage ===\n");

    /* OOM paths via custom allocator */
    test_oom_parse();
    test_oom_build();

    /* Edge cases */
    test_null_doc();
    test_edge_ops();
    test_serialize_false();
    test_serialize_control();
    test_parse_errors();
    test_string_null();
    test_long_string();

    if (failures > 0) {
        printf("\nFAILED: %d test(s) failed\n", failures);
        return 1;
    }
    printf("\nPASSED\n");
    return 0;
}
