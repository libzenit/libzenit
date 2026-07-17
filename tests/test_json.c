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
#include <libzenit/result.h>
#include "test_runner.h"
#include <string.h>
#include <math.h>

/* ─── Parse: null, true, false ─── */

static int test_parse_null(void) {
    TEST("parse null");
    zenit_json_t *doc = zenit_json_parse("null");
    if (doc == NULL) { FAIL("parse returned NULL"); return 1; }
    if (!zenit_json_value_is_null(zenit_json_root(doc))) { FAIL("not null"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_true(void) {
    TEST("parse true");
    zenit_json_t *doc = zenit_json_parse("true");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (!zenit_json_value_get_bool(zenit_json_root(doc))) { FAIL("not true"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_false(void) {
    TEST("parse false");
    zenit_json_t *doc = zenit_json_parse("false");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_value_get_bool(zenit_json_root(doc))) { FAIL("not false"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Parse: numbers ─── */

static int test_parse_numbers(void) {
    TEST("parse numbers");
    struct { const char *input; double expected; } cases[] = {
        {"0", 0.0}, {"42", 42.0}, {"-17", -17.0}, {"3.14", 3.14},
        {"-0.5", -0.5}, {"1e10", 1e10}, {"2.5e-3", 2.5e-3},
        {"-1E+2", -100.0}, {"0.0", 0.0}, {"100000", 100000.0},
    };
    int n = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < n; i++) {
        zenit_json_t *doc = zenit_json_parse(cases[i].input);
        if (doc == NULL) { FAIL("parse NULL"); return 1; }
        double got = zenit_json_value_get_number(zenit_json_root(doc));
        if (fabs(got - cases[i].expected) > 1e-15) { FAIL("value mismatch"); zenit_json_destroy(doc); return 1; }
        zenit_json_destroy(doc);
    }
    PASS(); return 0;
}

/* ─── Parse: strings ─── */

static int test_parse_string_simple(void) {
    TEST("parse string");
    zenit_json_t *doc = zenit_json_parse("\"hello\"");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, "hello") != 0) { FAIL("value wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_string_empty(void) {
    TEST("parse empty string");
    zenit_json_t *doc = zenit_json_parse("\"\"");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, "") != 0) { FAIL("value wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_string_escapes(void) {
    TEST("parse string escapes");
    zenit_json_t *doc = zenit_json_parse("\"quote:\\\"backslash:\\\\slash:/\"");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, "quote:\"backslash:\\slash:/") != 0) { FAIL("escapes wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_string_unicode(void) {
    TEST("parse string unicode");
    zenit_json_t *doc = zenit_json_parse("\"\\u0048\\u0069\"");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, "Hi") != 0) { FAIL("unicode wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_string_control(void) {
    TEST("parse string control chars");
    zenit_json_t *doc = zenit_json_parse("\"line1\\nline2\\ttab\"");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, "line1\nline2\ttab") != 0) { FAIL("control wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Parse: arrays ─── */

static int test_parse_array_empty(void) {
    TEST("parse []");
    zenit_json_t *doc = zenit_json_parse("[]");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_array_count(zenit_json_root(doc)) != 0) { FAIL("not empty"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_array_numbers(void) {
    TEST("parse [1,2,3]");
    zenit_json_t *doc = zenit_json_parse("[1,2,3]");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_array_count(zenit_json_root(doc)) != 3) { FAIL("count wrong"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_array_get(zenit_json_root(doc), 0)) != 1.0) { FAIL("[0] wrong"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_array_get(zenit_json_root(doc), 2)) != 3.0) { FAIL("[2] wrong"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_get(zenit_json_root(doc), 5) != NULL) { FAIL("OOB not NULL"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_array_mixed(void) {
    TEST("parse mixed array");
    zenit_json_t *doc = zenit_json_parse("[null, true, \"hi\"]");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_array_count(zenit_json_root(doc)) != 3) { FAIL("count"); zenit_json_destroy(doc); return 1; }
    if (!zenit_json_value_is_null(zenit_json_array_get(zenit_json_root(doc), 0))) { FAIL("[0] not null"); zenit_json_destroy(doc); return 1; }
    if (!zenit_json_value_get_bool(zenit_json_array_get(zenit_json_root(doc), 1))) { FAIL("[1] not true"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_array_nested(void) {
    TEST("parse nested array");
    zenit_json_t *doc = zenit_json_parse("[[1,[]],[{}]]");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_array_count(zenit_json_root(doc)) != 2) { FAIL("count"); zenit_json_destroy(doc); return 1; }
    const zenit_json_value_t *inner = zenit_json_array_get(zenit_json_root(doc), 0);
    if (inner == NULL || zenit_json_value_type(inner) != ZENIT_JSON_ARRAY) { FAIL("inner not array"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_count(inner) != 2) { FAIL("inner count"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Parse: objects ─── */

static int test_parse_object_empty(void) {
    TEST("parse {}");
    zenit_json_t *doc = zenit_json_parse("{}");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_object_count(zenit_json_root(doc)) != 0) { FAIL("not empty"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_object_simple(void) {
    TEST("parse object");
    zenit_json_t *doc = zenit_json_parse("{\"a\":1,\"b\":2}");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_object_count(zenit_json_root(doc)) != 2) { FAIL("count"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_object_get(zenit_json_root(doc), "a")) != 1.0) { FAIL("get a"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_object_get(zenit_json_root(doc), "b")) != 2.0) { FAIL("get b"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_get(zenit_json_root(doc), "nonexistent") != NULL) { FAIL("get nonexistent"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_object_key_index(void) {
    TEST("parse object key/index");
    zenit_json_t *doc = zenit_json_parse("{\"x\":10,\"y\":20}");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *k0 = zenit_json_object_key(zenit_json_root(doc), 0);
    const char *k1 = zenit_json_object_key(zenit_json_root(doc), 1);
    if (k0 == NULL || k1 == NULL) { FAIL("key NULL"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_object_value_at(zenit_json_root(doc), 0)) != 10.0) { FAIL("value_at 0"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_key(zenit_json_root(doc), 5) != NULL) { FAIL("key OOB"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_value_at(zenit_json_root(doc), 5) != NULL) { FAIL("value_at OOB"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_object_nested(void) {
    TEST("parse nested object");
    zenit_json_t *doc = zenit_json_parse("{\"obj\":{\"inner\":true}}");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const zenit_json_value_t *inner = zenit_json_object_get(zenit_json_root(doc), "obj");
    if (inner == NULL || zenit_json_value_type(inner) != ZENIT_JSON_OBJECT) { FAIL("inner not obj"); zenit_json_destroy(doc); return 1; }
    if (!zenit_json_value_get_bool(zenit_json_object_get(inner, "inner"))) { FAIL("inner bool"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Parse: complex nested ─── */

static int test_parse_complex(void) {
    TEST("parse complex nested");
    const char *input =
        "{"
        "  \"name\": \"LibZenit\","
        "  \"version\": 1,"
        "  \"features\": [\"json\", \"state\", \"arena\"],"
        "  \"metadata\": {"
        "    \"active\": true,"
        "    \"count\": 42,"
        "    \"ratio\": 3.14"
        "  }"
        "}";
    zenit_json_t *doc = zenit_json_parse(input);
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const zenit_json_value_t *root = zenit_json_root(doc);
    if (zenit_json_value_type(root) != ZENIT_JSON_OBJECT) { FAIL("root not obj"); zenit_json_destroy(doc); return 1; }
    const char *name = zenit_json_value_get_string(zenit_json_object_get(root, "name"));
    if (name == NULL || strcmp(name, "LibZenit") != 0) { FAIL("name wrong"); zenit_json_destroy(doc); return 1; }
    if (fabs(zenit_json_value_get_number(zenit_json_object_get(root, "version")) - 1.0) > 1e-15) { FAIL("version wrong"); zenit_json_destroy(doc); return 1; }
    const zenit_json_value_t *features = zenit_json_object_get(root, "features");
    if (features == NULL || zenit_json_value_type(features) != ZENIT_JSON_ARRAY) { FAIL("features not array"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_count(features) != 3) { FAIL("features count"); zenit_json_destroy(doc); return 1; }
    const zenit_json_value_t *meta = zenit_json_object_get(root, "metadata");
    if (meta == NULL || zenit_json_value_type(meta) != ZENIT_JSON_OBJECT) { FAIL("meta not obj"); zenit_json_destroy(doc); return 1; }
    if (!zenit_json_value_get_bool(zenit_json_object_get(meta, "active"))) { FAIL("meta.active not true"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Serialize: round-trip ─── */

static int test_roundtrip_null(void) {
    TEST("roundtrip null");
    zenit_json_t *doc = zenit_json_parse("null");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "null") == 0);
    free(s);
    if (!ok) { FAIL("null"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_bool(void) {
    TEST("roundtrip bool");
    zenit_json_t *doc = zenit_json_parse("true");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "true") == 0);
    free(s);
    if (!ok) { FAIL("true"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_number(void) {
    TEST("roundtrip number");
    zenit_json_t *doc = zenit_json_parse("42");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "42") == 0);
    free(s);
    if (!ok) { FAIL("42"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_negative(void) {
    TEST("roundtrip negative");
    zenit_json_t *doc = zenit_json_parse("-17");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "-17") == 0);
    free(s);
    if (!ok) { FAIL("-17"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_float(void) {
    TEST("roundtrip float");
    zenit_json_t *doc = zenit_json_parse("3.14");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "3.14") == 0);
    free(s);
    if (!ok) { FAIL("3.14"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_string(void) {
    TEST("roundtrip string");
    zenit_json_t *doc = zenit_json_parse("\"hello\"");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "\"hello\"") == 0);
    free(s);
    if (!ok) { FAIL("hello"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_array(void) {
    TEST("roundtrip array");
    zenit_json_t *doc = zenit_json_parse("[1,2,3]");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "[1,2,3]") == 0);
    free(s);
    if (!ok) { FAIL("[1,2,3]"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_object(void) {
    TEST("roundtrip object");
    zenit_json_t *doc = zenit_json_parse("{\"a\":1,\"b\":2}");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "{\"a\":1,\"b\":2}") == 0);
    free(s);
    if (!ok) { FAIL("obj"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_escaped_string(void) {
    TEST("roundtrip escaped string");
    zenit_json_t *doc = zenit_json_parse("\"quote:\\\"here\"");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "\"quote:\\\"here\"") == 0);
    free(s);
    if (!ok) { FAIL("escape"); return 1; }
    PASS(); return 0;
}

static int test_roundtrip_whitespace(void) {
    TEST("roundtrip with whitespace");
    zenit_json_t *doc = zenit_json_parse("  null  ");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    int ok = (s != NULL && strcmp(s, "null") == 0);
    free(s);
    if (!ok) { FAIL("whitespace"); return 1; }
    PASS(); return 0;
}

/* ─── Programmatic construction ─── */

static int test_programmatic(void) {
    TEST("programmatic construction");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create NULL"); return 1; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (obj == NULL) { FAIL("obj NULL"); zenit_json_destroy(doc); return 1; }

    if (zenit_json_object_set(obj, "name", zenit_json_value_string(doc, "Zenit")).error != ZENIT_OK) { FAIL("set name"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "count", zenit_json_value_number(doc, 99)).error != ZENIT_OK) { FAIL("set count"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "active", zenit_json_value_bool(doc, 1)).error != ZENIT_OK) { FAIL("set active"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "data", zenit_json_value_null(doc)).error != ZENIT_OK) { FAIL("set null"); zenit_json_destroy(doc); return 1; }

    zenit_json_value_t *arr = zenit_json_value_array(doc);
    if (arr == NULL) { FAIL("arr NULL"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_append(arr, zenit_json_value_number(doc, 10)).error != ZENIT_OK) { FAIL("append 10"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_append(arr, zenit_json_value_number(doc, 20)).error != ZENIT_OK) { FAIL("append 20"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "items", arr).error != ZENIT_OK) { FAIL("set items"); zenit_json_destroy(doc); return 1; }

    zenit_json_set_root(doc, obj);

    /* Verify */
    const char *name = zenit_json_value_get_string(zenit_json_object_get(obj, "name"));
    if (name == NULL || strcmp(name, "Zenit") != 0) { FAIL("get name"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_object_get(obj, "count")) != 99.0) { FAIL("get count"); zenit_json_destroy(doc); return 1; }
    if (!zenit_json_value_get_bool(zenit_json_object_get(obj, "active"))) { FAIL("get active"); zenit_json_destroy(doc); return 1; }
    if (!zenit_json_value_is_null(zenit_json_object_get(obj, "data"))) { FAIL("get data"); zenit_json_destroy(doc); return 1; }
    const zenit_json_value_t *got_arr = zenit_json_object_get(obj, "items");
    if (got_arr == NULL || zenit_json_value_type(got_arr) != ZENIT_JSON_ARRAY) { FAIL("items type"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_count(got_arr) != 2) { FAIL("items count"); zenit_json_destroy(doc); return 1; }

    /* Serialize */
    char *out = zenit_json_serialize(doc);
    if (out == NULL) { FAIL("serialize NULL"); zenit_json_destroy(doc); return 1; }
    free(out);
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Array operations ─── */

static int test_array_ops(void) {
    TEST("array operations");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create NULL"); return 1; }
    zenit_json_value_t *arr = zenit_json_value_array(doc);
    zenit_json_set_root(doc, arr);

    for (int i = 0; i < 100; i++) {
        if (zenit_json_array_append(arr, zenit_json_value_number(doc, (double)i)).error != ZENIT_OK) { FAIL("append"); zenit_json_destroy(doc); return 1; }
    }
    if (zenit_json_array_count(arr) != 100) { FAIL("count"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_array_get(arr, 99)) != 99.0) { FAIL("last"); zenit_json_destroy(doc); return 1; }

    if (zenit_json_array_insert(arr, 0, zenit_json_value_number(doc, -1)).error != ZENIT_OK) { FAIL("insert"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_count(arr) != 101) { FAIL("count after insert"); zenit_json_destroy(doc); return 1; }

    if (zenit_json_array_remove(arr, 0).error != ZENIT_OK) { FAIL("remove"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_count(arr) != 100) { FAIL("count after remove"); zenit_json_destroy(doc); return 1; }

    if (zenit_json_array_get(arr, 999) != NULL) { FAIL("get OOB"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_append(NULL, arr).error == ZENIT_OK) { FAIL("append NULL"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_remove(arr, 999).error == ZENIT_OK) { FAIL("remove OOB"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_insert(arr, 999, arr).error == ZENIT_OK) { FAIL("insert OOB"); zenit_json_destroy(doc); return 1; }

    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Object operations ─── */

static int test_object_ops(void) {
    TEST("object operations");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create NULL"); return 1; }
    zenit_json_value_t *obj = zenit_json_value_object(doc);
    zenit_json_set_root(doc, obj);

    if (zenit_json_object_set(obj, "key", zenit_json_value_number(doc, 1.0)).error != ZENIT_OK) { FAIL("set"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "key", zenit_json_value_number(doc, 2.0)).error != ZENIT_OK) { FAIL("overwrite"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_count(obj) != 1) { FAIL("count after overwrite"); zenit_json_destroy(doc); return 1; }

    if (zenit_json_object_remove(obj, "key").error != ZENIT_OK) { FAIL("remove"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_count(obj) != 0) { FAIL("count after remove"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_remove(obj, "key").error == ZENIT_OK) { FAIL("remove again"); zenit_json_destroy(doc); return 1; }

    for (int i = 0; i < 50; i++) {
        char key[16];
        snprintf(key, sizeof(key), "k%d", i);
        if (zenit_json_object_set(obj, key, zenit_json_value_number(doc, (double)i)).error != ZENIT_OK) { FAIL("set many"); zenit_json_destroy(doc); return 1; }
    }
    if (zenit_json_object_count(obj) != 50) { FAIL("count many"); zenit_json_destroy(doc); return 1; }

    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Object NULL safety ─── */

static int test_object_null_safety(void) {
    TEST("object NULL safety");
    if (zenit_json_object_count(NULL) != 0) { FAIL("count NULL"); return 1; }
    if (zenit_json_object_key(NULL, 0) != NULL) { FAIL("key NULL"); return 1; }
    if (zenit_json_object_value_at(NULL, 0) != NULL) { FAIL("value_at NULL"); return 1; }
    if (zenit_json_object_get(NULL, "x") != NULL) { FAIL("get NULL doc"); return 1; }
    if (zenit_json_object_set(NULL, "x", NULL).error == ZENIT_OK) { FAIL("set NULL obj"); return 1; }
    PASS(); return 0;
}

/* ─── Value serialization ─── */

static int test_value_serialize(void) {
    TEST("value serialization");
    zenit_json_t *doc = zenit_json_parse("{\"a\":[1,2,3]}");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const zenit_json_value_t *val = zenit_json_object_get(zenit_json_root(doc), "a");
    if (val == NULL) { FAIL("get a NULL"); zenit_json_destroy(doc); return 1; }
    char *out = zenit_json_value_serialize(val);
    if (out == NULL) { FAIL("value_serialize NULL"); zenit_json_destroy(doc); return 1; }
    if (strcmp(out, "[1,2,3]") != 0) { FAIL("wrong value"); free(out); zenit_json_destroy(doc); return 1; }
    free(out);
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Parse errors ─── */

static int test_parse_errors(void) {
    TEST("parse errors");
    const char *invalid[] = {
        "", "nul", "tru", "fals", "\"unterminated", "[1,2", "{missing",
        "{:}", "[,]", "{,\"a\":1}", "01", "+1", "1.", "1.e", "-",
        "\"\\x\"", "\"\\uGGGG\"", "trailing garbage after value 42",
    };
    int n = sizeof(invalid) / sizeof(invalid[0]);
    for (int i = 0; i < n; i++) {
        zenit_json_t *doc = zenit_json_parse(invalid[i]);
        if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return 1; }
    }
    PASS(); return 0;
}

/* ─── NULL safety ─── */

static int test_null_safety(void) {
    TEST("NULL safety");
    zenit_json_destroy(NULL);
    if (zenit_json_root(NULL) != NULL) { FAIL("root NULL"); return 1; }
    if (zenit_json_set_root(NULL, NULL).error == ZENIT_OK) { FAIL("set_root NULL"); return 1; }
    if (zenit_json_value_type(NULL) != ZENIT_JSON_NULL) { FAIL("type NULL"); return 1; }
    if (!zenit_json_value_is_null(NULL)) { FAIL("is_null NULL"); return 1; }
    if (zenit_json_value_get_bool(NULL) != 0) { FAIL("get_bool NULL"); return 1; }
    if (zenit_json_value_get_number(NULL) != 0.0) { FAIL("get_number NULL"); return 1; }
    if (zenit_json_value_get_string(NULL) != NULL) { FAIL("get_string NULL"); return 1; }
    if (zenit_json_serialize(NULL) != NULL) { FAIL("serialize NULL"); return 1; }
    char *s = zenit_json_value_serialize(NULL);
    if (s == NULL || strcmp(s, "null") != 0) { FAIL("value_serialize(NULL)"); free(s); return 1; }
    free(s);
    PASS(); return 0;
}

/* ─── Parse with length ─── */

static int test_parse_with_length(void) {
    TEST("parse with length");
    zenit_json_t *doc = zenit_json_parse_with_length("null  extra", 4);
    if (doc == NULL) { FAIL("parse_with_length NULL"); return 1; }
    if (!zenit_json_value_is_null(zenit_json_root(doc))) { FAIL("not null"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);

    doc = zenit_json_parse_with_length("true  ", 6);
    if (doc == NULL) { FAIL("ws NULL"); return 1; }
    if (!zenit_json_value_get_bool(zenit_json_root(doc))) { FAIL("not true"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);

    doc = zenit_json_parse_with_length_and_allocator("false", 5, ZENIT_ALLOCATOR_DEFAULT);
    if (doc == NULL) { FAIL("alloc NULL"); return 1; }
    if (zenit_json_value_get_bool(zenit_json_root(doc))) { FAIL("not false"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);

    if (zenit_json_parse_with_length(NULL, 0) != NULL) { FAIL("NULL input"); return 1; }
    PASS(); return 0;
}

/* ─── Create with allocator ─── */

static int test_create_with_allocator(void) {
    TEST("create with allocator");
    zenit_json_t *doc = zenit_json_create_with_allocator(ZENIT_ALLOCATOR_DEFAULT);
    if (doc == NULL) { FAIL("NULL"); return 1; }
    if (zenit_json_root(doc) != NULL) { FAIL("root not NULL"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Mismatched getters ─── */

static int test_mismatched_getters(void) {
    TEST("mismatched getters");
    zenit_json_t *doc = zenit_json_parse("42");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_value_get_bool(zenit_json_root(doc)) != 0) { FAIL("bool from num"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_string(zenit_json_root(doc)) != NULL) { FAIL("string from num"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);

    doc = zenit_json_parse("\"hello\"");
    if (doc == NULL) { FAIL("parse str NULL"); return 1; }
    if (zenit_json_value_get_number(zenit_json_root(doc)) != 0.0) { FAIL("num from str"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);

    doc = zenit_json_parse("null");
    if (zenit_json_value_get_bool(zenit_json_root(doc)) != 0) { FAIL("bool from null"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_string(zenit_json_root(doc)) != NULL) { FAIL("string from null"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Non-array/non-object operations ─── */

static int test_non_container_ops(void) {
    TEST("non-container ops");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create NULL"); return 1; }

    zenit_json_value_t *num = zenit_json_value_number(doc, 1.0);
    if (zenit_json_array_count(num) != 0) { FAIL("array_count on num"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_array_get(num, 0) != NULL) { FAIL("array_get on num"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_count(num) != 0) { FAIL("object_count on num"); zenit_json_destroy(doc); return 1; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    if (zenit_json_array_append(obj, num).error == ZENIT_OK) { FAIL("append to obj"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(num, "k", num).error == ZENIT_OK) { FAIL("set on num"); zenit_json_destroy(doc); return 1; }

    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Additional coverage tests ─── */

static int test_parse_string_all_escapes(void) {
    TEST("parse all escape sequences");
    /* Test \/, \b, \f, \r which were not covered */
    zenit_json_t *doc = zenit_json_parse("\"slash:\\/back:\\bform:\\freturn:\\r\"");
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL) { FAIL("str NULL"); zenit_json_destroy(doc); return 1; }
    if (strcmp(s, "slash:/back:\bform:\freturn:\r") != 0) { FAIL("escapes wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_string_multibyte_utf8(void) {
    TEST("parse multibyte UTF-8");
    /* 2-byte UTF-8: \u0080 -> 0xC2 0x80 */
    zenit_json_t *doc = zenit_json_parse("\"\\u0080\\u07FF\"");
    if (doc == NULL) { FAIL("parse 2byte NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL) { FAIL("str NULL"); zenit_json_destroy(doc); return 1; }
    /* \u0080 = 2 bytes, \u07FF = 2 bytes */
    if (strlen(s) != 4) { FAIL("2byte wrong length"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);

    /* 3-byte UTF-8: \u0800 -> 0xE0 0xA0 0x80 */
    doc = zenit_json_parse("\"\\u0800\\uFFFF\"");
    if (doc == NULL) { FAIL("parse 3byte NULL"); return 1; }
    s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL) { FAIL("str NULL"); zenit_json_destroy(doc); return 1; }
    if (strlen(s) != 6) { FAIL("3byte wrong length"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_long_string(void) {
    TEST("parse long string (>64 chars)");
    /* Build a 100-char string to trigger buffer realloc in parser */
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
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    const char *s = zenit_json_value_get_string(zenit_json_root(doc));
    if (s == NULL || strcmp(s, expected) != 0) { FAIL("long string wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_parse_with_allocator_direct(void) {
    TEST("parse with allocator direct");
    zenit_json_t *doc = zenit_json_parse_with_allocator("null", ZENIT_ALLOCATOR_DEFAULT);
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (!zenit_json_value_is_null(zenit_json_root(doc))) { FAIL("not null"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_object_remove_shift(void) {
    TEST("object remove shift");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create NULL"); return 1; }
    zenit_json_value_t *obj = zenit_json_value_object(doc);
    zenit_json_set_root(doc, obj);

    /* Add 3 keys, remove the first one — triggers shift of remaining two */
    if (zenit_json_object_set(obj, "a", zenit_json_value_number(doc, 1)).error != ZENIT_OK) { FAIL("set a"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "b", zenit_json_value_number(doc, 2)).error != ZENIT_OK) { FAIL("set b"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_set(obj, "c", zenit_json_value_number(doc, 3)).error != ZENIT_OK) { FAIL("set c"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_remove(obj, "a").error != ZENIT_OK) { FAIL("remove a"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_object_count(obj) != 2) { FAIL("count after remove"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_object_get(obj, "b")) != 2.0) { FAIL("b wrong"); zenit_json_destroy(doc); return 1; }
    if (zenit_json_value_get_number(zenit_json_object_get(obj, "c")) != 3.0) { FAIL("c wrong"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

static int test_serialize_backslash(void) {
    TEST("serialize backslash");
    zenit_json_t *doc = zenit_json_parse("\"a\\\\b\"");
    char *s = zenit_json_serialize(doc);
    zenit_json_destroy(doc);
    if (s == NULL || strcmp(s, "\"a\\\\b\"") != 0) { FAIL("backslash serialize"); free(s); return 1; }
    free(s);
    PASS(); return 0;
}

static int test_parse_array_growth(void) {
    TEST("parse array with many elements (growth)");
    /* Build a JSON array with 20 elements to exercise array_grow */
    char input[512] = "[";
    for (int i = 0; i < 20; i++) {
        if (i > 0) strcat(input, ",");
        char num[8];
        snprintf(num, sizeof(num), "%d", i);
        strcat(input, num);
    }
    strcat(input, "]");
    zenit_json_t *doc = zenit_json_parse(input);
    if (doc == NULL) { FAIL("parse NULL"); return 1; }
    if (zenit_json_array_count(zenit_json_root(doc)) != 20) { FAIL("count"); zenit_json_destroy(doc); return 1; }
    zenit_json_destroy(doc);
    PASS(); return 0;
}

/* ─── Main ─── */

int main(void) {
    TEST_ENTRY tests[] = {
        { test_parse_null, "parse null" },
        { test_parse_true, "parse true" },
        { test_parse_false, "parse false" },
        { test_parse_numbers, "parse numbers" },
        { test_parse_string_simple, "parse string" },
        { test_parse_string_empty, "parse empty string" },
        { test_parse_string_escapes, "parse string escapes" },
        { test_parse_string_unicode, "parse string unicode" },
        { test_parse_string_control, "parse string control" },
        { test_parse_string_all_escapes, "parse all escapes" },
        { test_parse_string_multibyte_utf8, "parse multibyte utf8" },
        { test_parse_long_string, "parse long string" },
        { test_parse_array_empty, "parse []" },
        { test_parse_array_numbers, "parse [1,2,3]" },
        { test_parse_array_mixed, "parse mixed array" },
        { test_parse_array_nested, "parse nested array" },
        { test_parse_array_growth, "parse array growth" },
        { test_parse_object_empty, "parse {}" },
        { test_parse_object_simple, "parse object" },
        { test_parse_object_key_index, "parse key/index" },
        { test_parse_object_nested, "parse nested obj" },
        { test_parse_complex, "parse complex" },
        { test_roundtrip_null, "roundtrip null" },
        { test_roundtrip_bool, "roundtrip bool" },
        { test_roundtrip_number, "roundtrip number" },
        { test_roundtrip_negative, "roundtrip negative" },
        { test_roundtrip_float, "roundtrip float" },
        { test_roundtrip_string, "roundtrip string" },
        { test_roundtrip_array, "roundtrip array" },
        { test_roundtrip_object, "roundtrip object" },
        { test_roundtrip_escaped_string, "roundtrip escape" },
        { test_roundtrip_whitespace, "roundtrip ws" },
        { test_serialize_backslash, "serialize backslash" },
        { test_programmatic, "programmatic" },
        { test_array_ops, "array ops" },
        { test_object_ops, "object ops" },
        { test_object_null_safety, "obj NULL safety" },
        { test_object_remove_shift, "object remove shift" },
        { test_value_serialize, "value serialize" },
        { test_parse_errors, "parse errors" },
        { test_null_safety, "NULL safety" },
        { test_parse_with_length, "parse with length" },
        { test_parse_with_allocator_direct, "parse with allocator" },
        { test_create_with_allocator, "create with alloc" },
        { test_mismatched_getters, "mismatched getters" },
        { test_non_container_ops, "non-container ops" },
        { 0, 0 }
    };
    return test_run_all("json", tests);
}
