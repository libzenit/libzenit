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
 * Targeted coverage test for JSON module.
 *
 * Hits remaining uncovered lines that require specific edge cases
 * or error conditions not covered by test_json.c or test_json_malloc_fail.c.
 */

#include <libzenit/json.h>
#include <libzenit/result.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int failures = 0;
#define TEST(name) do { printf("  %s ... ", name); fflush(stdout); } while (0)
#define PASS() do { printf("PASS\n"); } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); failures++; } while (0)

/* 1. Hit value constructors with NULL doc (lines 774, 784, 795, 806, 822, 832) */
static void test_null_doc_constructors(void) {
    TEST("value constructors with NULL doc");
    if (zenit_json_value_null(NULL) != NULL) { FAIL("null"); return; }
    if (zenit_json_value_bool(NULL, 1) != NULL) { FAIL("bool"); return; }
    if (zenit_json_value_number(NULL, 1.0) != NULL) { FAIL("number"); return; }
    if (zenit_json_value_string(NULL, "x") != NULL) { FAIL("string"); return; }
    if (zenit_json_value_array(NULL) != NULL) { FAIL("array"); return; }
    if (zenit_json_value_object(NULL) != NULL) { FAIL("object"); return; }
    PASS();
}

/* 2. array_remove on non-array and NULL (lines 877, 880) */
static void test_array_remove_edge(void) {
    TEST("array_remove edge cases");
    if (zenit_json_array_remove(NULL, 0).error == ZENIT_OK) { FAIL("remove NULL"); return; }
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }
    zenit_json_value_t *num = zenit_json_value_number(doc, 1.0);
    if (zenit_json_array_remove(num, 0).error == ZENIT_OK) { FAIL("remove on num"); zenit_json_destroy(doc); return; }
    zenit_json_destroy(doc);
    PASS();
}

/* 3. array_insert on non-array (line 897) */
static void test_array_insert_edge(void) {
    TEST("array_insert on non-array");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }
    zenit_json_value_t *num = zenit_json_value_number(doc, 1.0);
    if (zenit_json_array_insert(num, 0, num).error == ZENIT_OK) { FAIL("insert on num"); zenit_json_destroy(doc); return; }
    zenit_json_destroy(doc);
    PASS();
}

/* 4. object_set key dup OOM (line 980) — tested via malloc_fail */
/* 5. object_remove NULL and non-object (lines 991, 994) */
static void test_object_remove_edge(void) {
    TEST("object_remove edge cases");
    if (zenit_json_object_remove(NULL, "x").error == ZENIT_OK) { FAIL("remove NULL obj"); return; }
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }
    zenit_json_value_t *num = zenit_json_value_number(doc, 1.0);
    if (zenit_json_object_remove(num, "x").error == ZENIT_OK) { FAIL("remove on num"); zenit_json_destroy(doc); return; }
    zenit_json_destroy(doc);
    PASS();
}

/* 6. Serialize strings with control chars (lines 1033-1037, 1041-1042) */
static void test_serialize_control_strings(void) {
    TEST("serialize control strings");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }

    /* String with \b, \f, \n, \r, \t */
    zenit_json_value_t *s = zenit_json_value_string(doc, "a\bb\fc\nd\re\tf");
    if (s == NULL) { FAIL("create string"); zenit_json_destroy(doc); return; }
    zenit_json_set_root(doc, s);

    char *out = zenit_json_serialize(doc);
    if (out == NULL) { FAIL("serialize NULL"); zenit_json_destroy(doc); return; }
    /* Expected: "a\bb\fc\nd\re\tf" */
    if (strcmp(out, "\"a\\bb\\fc\\nd\\re\\tf\"") != 0) {
        FAIL("serialize control");
        fprintf(stderr, "  got: %s\n", out);
        free(out);
        zenit_json_destroy(doc);
        return;
    }
    free(out);
    zenit_json_destroy(doc);
    PASS();
}

/* 7. Serialize string with control char < 0x20 (not one of the named escapes)
 *    This triggers the \uXXXX path in serializer (lines 1041-1042) */
static void test_serialize_control_hex(void) {
    TEST("serialize control hex");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }

    /* String with \x01 (control char that's not \b,\f,\n,\r,\t) */
    char str_with_ctrl[2] = { 0x01, 0 };
    zenit_json_value_t *s = zenit_json_value_string(doc, str_with_ctrl);
    if (s == NULL) { FAIL("create string"); zenit_json_destroy(doc); return; }
    zenit_json_set_root(doc, s);

    char *out = zenit_json_serialize(doc);
    if (out == NULL) { FAIL("serialize NULL"); zenit_json_destroy(doc); return; }
    if (strcmp(out, "\"\\u0001\"") != 0) {
        FAIL("serialize hex control");
        fprintf(stderr, "  got: %s\n", out);
        free(out);
        zenit_json_destroy(doc);
        return;
    }
    free(out);
    zenit_json_destroy(doc);
    PASS();
}

/* 8. Parse with trailing garbage (line 676) */
static void test_parse_trailing_garbage(void) {
    TEST("parse trailing garbage");
    zenit_json_t *doc = zenit_json_parse("null extra");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 9. Object set with key_dup OOM — test value_serialize_into fails (line 1141-1142) */
/* 10. string value with NULL arg (line 814) */
static void test_string_value_null(void) {
    TEST("string value with NULL str");
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { FAIL("create doc"); return; }
    zenit_json_value_t *val = zenit_json_value_string(doc, NULL);
    if (val == NULL) { FAIL("value NULL"); zenit_json_destroy(doc); return; }
    const char *s = zenit_json_value_get_string(val);
    if (s == NULL || strcmp(s, "") != 0) { FAIL("wrong value"); zenit_json_destroy(doc); return; }
    zenit_json_destroy(doc);
    PASS();
}

/* 11. Parse errors: trailing comma in array [1,] (lines 549-550) */
static void test_parse_trailing_comma_array(void) {
    TEST("parse [1,]");
    zenit_json_t *doc = zenit_json_parse("[1,]");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 12. Parse errors: trailing comma in object {"a":1,} (line 615-616) */
static void test_parse_trailing_comma_object(void) {
    TEST("parse {\"a\":1,}");
    zenit_json_t *doc = zenit_json_parse("{\"a\":1,}");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 13. Unterminated backslash at end of string (lines 303-305) */
static void test_parse_unterminated_escape(void) {
    TEST("parse unterminated escape");
    /* JSON text: "\ (backslash at end with no following char) */
    /* In C: "\"\\" = " (escaped quote) + \ (escaped backslash) */
    zenit_json_t *doc = zenit_json_parse("\"\\");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 14. Truncated unicode escape (lines 321-323) */
static void test_parse_truncated_unicode(void) {
    TEST("parse truncated \\u");
    /* JSON text: "\u (backslash-u at end with no hex digits) */
    /* In C: "\"\\u" = " + \ + u */
    zenit_json_t *doc = zenit_json_parse("\"\\u");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 15. Array with element not followed by , or ] (lines 549-550) */
static void test_parse_array_bad_separator(void) {
    TEST("parse [1 2]");
    zenit_json_t *doc = zenit_json_parse("[1 2]");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 16. Object with missing colon after key (lines 581-583) */
static void test_parse_object_missing_colon(void) {
    TEST("parse {\"a\" 1}");
    zenit_json_t *doc = zenit_json_parse("{\"a\" 1}");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 17. Object with missing comma between pairs (lines 615-616) */
static void test_parse_object_missing_comma(void) {
    TEST("parse {\"a\":1 \"b\":2}");
    zenit_json_t *doc = zenit_json_parse("{\"a\":1 \"b\":2}");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 18. Object with invalid value after colon (lines 589-590) */
static void test_parse_object_invalid_value(void) {
    TEST("parse {\"a\":}");
    zenit_json_t *doc = zenit_json_parse("{\"a\":}");
    if (doc != NULL) { FAIL("should be NULL"); zenit_json_destroy(doc); return; }
    PASS();
}

/* 19. array_insert with NULL arr (line 898) */
static void test_array_insert_null(void) {
    TEST("array_insert NULL");
    if (zenit_json_array_insert(NULL, 0, NULL).error == ZENIT_OK) { FAIL("insert NULL arr"); return; }
    PASS();
}

/* 20. Serialize false (line 1075) */
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

int main(void) {
    printf("=== json coverage ===\n");
    test_null_doc_constructors();
    test_array_remove_edge();
    test_array_insert_edge();
    test_object_remove_edge();
    test_serialize_control_strings();
    test_serialize_control_hex();
    test_parse_trailing_garbage();
    test_string_value_null();
    test_parse_trailing_comma_array();
    test_parse_trailing_comma_object();
    test_parse_unterminated_escape();
    test_parse_truncated_unicode();
    test_parse_array_bad_separator();
    test_parse_object_missing_colon();
    test_parse_object_missing_comma();
    test_parse_object_invalid_value();
    test_array_insert_null();
    test_serialize_false();

    if (failures > 0) {
        printf("\nFAILED: %d test(s) failed\n", failures);
        return 1;
    }
    printf("\nPASSED\n");
    return 0;
}
