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

#include <libzenit/string.h>
#include <string.h>

#include "test_runner.h"

/* ─── Test: create and destroy empty string ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");
    ASSERT(zenit_string_length(s) == 0, "length should be 0");
    ASSERT(zenit_string_empty(s), "should be empty");
    ASSERT(zenit_string_cstr(s) != NULL, "cstr should not be NULL");
    ASSERT(strcmp(zenit_string_cstr(s), "") == 0, "cstr should be empty string");
    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: create_from with NULL and empty ─── */
static int test_create_from_null_empty(void) {
    TEST("create_from null/empty");
    zenit_string_t *s = zenit_string_create_from(NULL);
    ASSERT(s != NULL, "create_from(NULL) should succeed");
    ASSERT(zenit_string_length(s) == 0, "length should be 0");
    zenit_string_destroy(s);

    s = zenit_string_create_from("");
    ASSERT(s != NULL, "create_from(\"\") should succeed");
    ASSERT(zenit_string_length(s) == 0, "length should be 0");
    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: create_from with content ─── */
static int test_create_from_content(void) {
    TEST("create_from content");
    zenit_string_t *s = zenit_string_create_from("hello");
    ASSERT(s != NULL, "expected non-NULL");
    ASSERT(zenit_string_length(s) == 5, "length should be 5");
    ASSERT(strcmp(zenit_string_cstr(s), "hello") == 0, "content mismatch");
    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: destroy NULL is safe ─── */
static int test_destroy_null(void) {
    TEST("destroy NULL is safe");
    zenit_string_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: append small data ─── */
static int test_append_small(void) {
    TEST("append small");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    zenit_result_t r = zenit_string_append(s, "abc", 3);
    ASSERT(r.error == ZENIT_OK, "append failed");
    ASSERT(zenit_string_length(s) == 3, "length should be 3");
    ASSERT(strcmp(zenit_string_cstr(s), "abc") == 0, "content mismatch");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: append multiple times ─── */
static int test_append_multiple(void) {
    TEST("append multiple");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    ASSERT(zenit_string_append(s, "a", 1).error == ZENIT_OK, "append a");
    ASSERT(zenit_string_append(s, "b", 1).error == ZENIT_OK, "append b");
    ASSERT(zenit_string_append(s, "c", 1).error == ZENIT_OK, "append c");
    ASSERT(zenit_string_length(s) == 3, "length should be 3");
    ASSERT(strcmp(zenit_string_cstr(s), "abc") == 0, "content mismatch");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: append with zero length is no-op ─── */
static int test_append_zero_len(void) {
    TEST("append zero length");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    ASSERT(zenit_string_append(s, NULL, 0).error == ZENIT_OK, "append(NULL, 0) should be OK");
    ASSERT(zenit_string_length(s) == 0, "length should still be 0");

    ASSERT(zenit_string_append(s, "x", 0).error == ZENIT_OK, "append(x, 0) should be OK");
    ASSERT(zenit_string_length(s) == 0, "length should still be 0");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: append with len but NULL data returns error ─── */
static int test_append_null_data(void) {
    TEST("append NULL data with len > 0");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    zenit_result_t r = zenit_string_append(s, NULL, 5);
    ASSERT(r.error == ZENIT_ERROR_NULL, "expected NULL error");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: append_cstr ─── */
static int test_append_cstr(void) {
    TEST("append_cstr");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    ASSERT(zenit_string_append_cstr(s, "hello ").error == ZENIT_OK, "append first");
    ASSERT(zenit_string_append_cstr(s, "world").error == ZENIT_OK, "append second");
    ASSERT(zenit_string_length(s) == 11, "length should be 11");
    ASSERT(strcmp(zenit_string_cstr(s), "hello world") == 0, "content mismatch");

    /* Append NULL cstr — should be no-op */
    ASSERT(zenit_string_append_cstr(s, NULL).error == ZENIT_OK, "append NULL cstr");
    ASSERT(zenit_string_length(s) == 11, "length unchanged");

    /* Append empty cstr — should be no-op */
    ASSERT(zenit_string_append_cstr(s, "").error == ZENIT_OK, "append empty cstr");
    ASSERT(zenit_string_length(s) == 11, "length unchanged");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: cstr after various operations ─── */
static int test_cstr_content(void) {
    TEST("cstr content");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    ASSERT(zenit_string_append_cstr(s, "Hello").error == ZENIT_OK, "append");
    ASSERT(strcmp(zenit_string_cstr(s), "Hello") == 0, "cstr match");
    ASSERT((int)zenit_string_cstr(s)[5] == 0, "null terminator at position 5");

    ASSERT(zenit_string_append(s, " World!", 7).error == ZENIT_OK, "append more");
    ASSERT(strcmp(zenit_string_cstr(s), "Hello World!") == 0, "cstr after append");
    ASSERT((int)zenit_string_cstr(s)[12] == 0, "null terminator at end");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: length ─── */
static int test_length(void) {
    TEST("length");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");
    ASSERT(zenit_string_length(s) == 0, "empty length");

    ASSERT(zenit_string_append_cstr(s, "12345").error == ZENIT_OK, "append");
    ASSERT(zenit_string_length(s) == 5, "length 5");

    ASSERT(zenit_string_append_cstr(s, "67890").error == ZENIT_OK, "append more");
    ASSERT(zenit_string_length(s) == 10, "length 10");

    ASSERT(zenit_string_length(NULL) == 0, "NULL length is 0");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: capacity ─── */
static int test_capacity(void) {
    TEST("capacity");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");
    ASSERT(zenit_string_capacity(s) >= 1, "capacity at least 1 for null");

    /* Append enough to force growth */
    for (int i = 0; i < 100; i++) {
        ASSERT(zenit_string_append_cstr(s, "x").error == ZENIT_OK, "append");
    }
    ASSERT(zenit_string_capacity(s) >= 100, "capacity should be >= 100");
    ASSERT(zenit_string_capacity(NULL) == 0, "NULL capacity is 0");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    TEST("clear");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    ASSERT(zenit_string_append_cstr(s, "hello").error == ZENIT_OK, "append");
    ASSERT(zenit_string_length(s) == 5, "length before clear");

    zenit_string_clear(s);
    ASSERT(zenit_string_length(s) == 0, "length after clear");
    ASSERT(zenit_string_empty(s), "empty after clear");
    ASSERT(strcmp(zenit_string_cstr(s), "") == 0, "cstr after clear");

    /* Can re-use after clear */
    ASSERT(zenit_string_append_cstr(s, "world").error == ZENIT_OK, "append after clear");
    ASSERT(strcmp(zenit_string_cstr(s), "world") == 0, "cstr after re-use");

    /* Clear NULL is safe */
    zenit_string_clear(NULL);

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: reserve ─── */
static int test_reserve(void) {
    TEST("reserve");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    ASSERT(zenit_string_reserve(s, 100).error == ZENIT_OK, "reserve");
    ASSERT(zenit_string_capacity(s) >= 100, "capacity >= 100");

    /* Reserve smaller — no-op */
    ASSERT(zenit_string_reserve(s, 10).error == ZENIT_OK, "reserve smaller");
    ASSERT(zenit_string_capacity(s) >= 100, "capacity unchanged");

    /* Reserve NULL returns NULL error */
    ASSERT(zenit_string_reserve(NULL, 10).error == ZENIT_ERROR_NULL, "reserve NULL");

    /* Content still intact */
    ASSERT(zenit_string_append_cstr(s, "test").error == ZENIT_OK, "append after reserve");
    ASSERT(strcmp(zenit_string_cstr(s), "test") == 0, "content after reserve");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: shrink_to_fit ─── */
static int test_shrink_to_fit(void) {
    TEST("shrink_to_fit");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    /* Append enough to trigger growth */
    for (int i = 0; i < 50; i++) {
        ASSERT(zenit_string_append_cstr(s, "x").error == ZENIT_OK, "append");
    }

    size_t cap_before = zenit_string_capacity(s);
    ASSERT(cap_before >= 50, "capacity >= 50 before shrink");

    ASSERT(zenit_string_shrink_to_fit(s).error == ZENIT_OK, "shrink");
    /* After shrink, capacity should be at least length + 1 (for null terminator) */
    ASSERT(zenit_string_capacity(s) <= cap_before, "capacity after shrink <= before");
    ASSERT(zenit_string_length(s) == 50, "length still 50");
    ASSERT(strcmp(zenit_string_cstr(s), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") == 0, "content after shrink");

    /* Shrink NULL returns NULL error */
    ASSERT(zenit_string_shrink_to_fit(NULL).error == ZENIT_ERROR_NULL, "shrink NULL");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: empty ─── */
static int test_empty(void) {
    TEST("empty");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");
    ASSERT(zenit_string_empty(s), "empty after create");

    ASSERT(zenit_string_append_cstr(s, "x").error == ZENIT_OK, "append");
    ASSERT(!zenit_string_empty(s), "not empty after append");

    zenit_string_clear(s);
    ASSERT(zenit_string_empty(s), "empty after clear");

    ASSERT(zenit_string_empty(NULL), "NULL is empty");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: NULL handling for all mutator functions ─── */
static int test_null_params(void) {
    TEST("NULL params");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    /* append with NULL string */
    ASSERT(zenit_string_append(NULL, "x", 1).error == ZENIT_ERROR_NULL, "append NULL string");

    /* append_cstr with NULL string */
    ASSERT(zenit_string_append_cstr(NULL, "x").error == ZENIT_ERROR_NULL, "append_cstr NULL string");

    /* cstr with NULL */
    ASSERT(zenit_string_cstr(NULL) == NULL, "cstr NULL returns NULL");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: many appends (1000) ─── */
static int test_many_appends(void) {
    TEST("many appends (1000)");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    for (int i = 0; i < 1000; i++) {
        ASSERT(zenit_string_append_cstr(s, "x").error == ZENIT_OK, "append");
    }

    ASSERT(zenit_string_length(s) == 1000, "length should be 1000");

    /* Verify content */
    const char *cstr = zenit_string_cstr(s);
    ASSERT(cstr != NULL, "cstr not NULL");
    for (int i = 0; i < 1000; i++) {
        ASSERT(cstr[i] == 'x', "char mismatch");
    }
    ASSERT(cstr[1000] == '\0', "null terminator at 1000");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

/* ─── Test: large append ─── */
static int test_large_append(void) {
    TEST("large append");
    zenit_string_t *s = zenit_string_create();
    ASSERT(s != NULL, "expected non-NULL");

    char buf[256];
    memset(buf, 'A', 256);

    for (int i = 0; i < 100; i++) {
        ASSERT(zenit_string_append(s, buf, 256).error == ZENIT_OK, "append large");
    }

    ASSERT(zenit_string_length(s) == 25600, "length should be 25600");

    const char *cstr = zenit_string_cstr(s);
    ASSERT(cstr != NULL, "cstr not NULL");
    for (size_t i = 0; i < 25600; i++) {
        ASSERT(cstr[i] == 'A', "char mismatch");
    }
    ASSERT(cstr[25600] == '\0', "null terminator at end");

    zenit_string_destroy(s);
    PASS();
    return 0;
}

int main(void) {
    TEST_ENTRY tests[] = {
        { test_create_destroy, "create/destroy" },
        { test_create_from_null_empty, "create_from null/empty" },
        { test_create_from_content, "create_from content" },
        { test_destroy_null, "destroy NULL" },
        { test_append_small, "append small" },
        { test_append_multiple, "append multiple" },
        { test_append_zero_len, "append zero len" },
        { test_append_null_data, "append NULL data" },
        { test_append_cstr, "append_cstr" },
        { test_cstr_content, "cstr content" },
        { test_length, "length" },
        { test_capacity, "capacity" },
        { test_clear, "clear" },
        { test_reserve, "reserve" },
        { test_shrink_to_fit, "shrink_to_fit" },
        { test_empty, "empty" },
        { test_null_params, "NULL params" },
        { test_many_appends, "many appends" },
        { test_large_append, "large append" },
        { 0, 0 }
    };
    return test_run_all("string", tests);
}
