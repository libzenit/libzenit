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

#include <libzenit/trie.h>
#include <libzenit/result.h>
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>

static int test_create_destroy(void) {
    zenit_trie_t *trie = zenit_trie_create();
    ASSERT(trie != NULL, "create returned NULL");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_empty_count(void) {
    zenit_trie_t *trie = zenit_trie_create();
    ASSERT(zenit_trie_count(trie) == 0, "count should be 0 on empty");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_search_miss_empty(void) {
    zenit_trie_t *trie = zenit_trie_create();
    ASSERT(zenit_trie_search(trie, "hello", NULL) == 0, "should miss on empty");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_insert_and_search(void) {
    zenit_trie_t *trie = zenit_trie_create();
    int val;
    ASSERT(zenit_trie_insert(trie, "hello", 42).error == ZENIT_OK, "insert");
    ASSERT(zenit_trie_count(trie) == 1, "count 1");
    ASSERT(zenit_trie_search(trie, "hello", &val) == 1, "search hit");
    ASSERT(val == 42, "value 42");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_search_miss(void) {
    zenit_trie_t *trie = zenit_trie_create();
    zenit_trie_insert(trie, "hello", 1);
    ASSERT(zenit_trie_search(trie, "world", NULL) == 0, "world should miss");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_starts_with(void) {
    zenit_trie_t *trie = zenit_trie_create();
    zenit_trie_insert(trie, "hello", 1);
    ASSERT(zenit_trie_starts_with(trie, "hel") == 1, "prefix true");
    ASSERT(zenit_trie_starts_with(trie, "xyz") == 0, "prefix false");
    ASSERT(zenit_trie_starts_with(trie, "hello") == 1, "exact prefix true");
    ASSERT(zenit_trie_starts_with(trie, "h") == 1, "single char prefix");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_shared_prefix(void) {
    zenit_trie_t *trie = zenit_trie_create();
    int val;
    ASSERT(zenit_trie_insert(trie, "hell", 1).error == ZENIT_OK, "insert hell");
    ASSERT(zenit_trie_insert(trie, "helicopter", 2).error == ZENIT_OK, "insert helicopter");
    ASSERT(zenit_trie_insert(trie, "help", 3).error == ZENIT_OK, "insert help");
    ASSERT(zenit_trie_insert(trie, "HELLO", 99).error == ZENIT_OK, "insert HELLO");
    ASSERT(zenit_trie_count(trie) == 4, "count 4");
    ASSERT(zenit_trie_search(trie, "hello", &val) == 1, "hello hit");
    ASSERT(val == 99, "HELLO overwrote hello value");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_delete(void) {
    zenit_trie_t *trie = zenit_trie_create();
    zenit_trie_insert(trie, "hell", 1);
    zenit_trie_insert(trie, "hello", 2);
    zenit_trie_insert(trie, "help", 3);
    ASSERT(zenit_trie_delete(trie, "hell").error == ZENIT_OK, "delete hell");
    ASSERT(zenit_trie_count(trie) == 2, "count 2 after delete");
    ASSERT(zenit_trie_search(trie, "hell", NULL) == 0, "hell gone");
    ASSERT(zenit_trie_search(trie, "hello", NULL) == 1, "hello still exists");
    ASSERT(zenit_trie_search(trie, "help", NULL) == 1, "help still exists");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_delete_nonexistent(void) {
    zenit_trie_t *trie = zenit_trie_create();
    zenit_trie_insert(trie, "hell", 1);
    ASSERT(zenit_trie_delete(trie, "nonexistent").error == ZENIT_ERROR_NOT_FOUND, "not found");
    ASSERT(zenit_trie_delete(trie, "hell").error == ZENIT_OK, "delete hell");
    ASSERT(zenit_trie_delete(trie, "hell").error == ZENIT_ERROR_NOT_FOUND, "already deleted");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_clear(void) {
    zenit_trie_t *trie = zenit_trie_create();
    int val;
    zenit_trie_insert(trie, "hello", 1);
    zenit_trie_insert(trie, "world", 2);
    zenit_trie_clear(trie);
    ASSERT(zenit_trie_count(trie) == 0, "count 0 after clear");
    ASSERT(zenit_trie_search(trie, "hello", NULL) == 0, "hello gone");
    ASSERT(zenit_trie_starts_with(trie, "h") == 0, "prefix gone");
    ASSERT(zenit_trie_insert(trie, "reuse", 100).error == ZENIT_OK, "reuse after clear");
    ASSERT(zenit_trie_count(trie) == 1, "count 1 after reinsert");
    ASSERT(zenit_trie_search(trie, "reuse", &val) == 1, "reuse search hit");
    ASSERT(val == 100, "reuse value 100");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_allocator_variant(void) {
    zenit_trie_t *trie = zenit_trie_create_with_allocator(ZENIT_ALLOCATOR_DEFAULT);
    int val;
    ASSERT(trie != NULL, "create_with_allocator");
    ASSERT(zenit_trie_insert(trie, "allocator_test", 7).error == ZENIT_OK, "insert");
    ASSERT(zenit_trie_search(trie, "allocator_test", &val) == 1, "search hit");
    ASSERT(val == 7, "value 7");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_null_params(void) {
    zenit_trie_t *trie = zenit_trie_create();
    zenit_trie_insert(trie, "sample", 10);
    ASSERT(zenit_trie_search(NULL, "sample", NULL) == 0, "search NULL trie");
    ASSERT(zenit_trie_search(trie, NULL, NULL) == 0, "search NULL key");
    ASSERT(zenit_trie_starts_with(NULL, "s") == 0, "starts_with NULL trie");
    ASSERT(zenit_trie_starts_with(trie, NULL) == 0, "starts_with NULL prefix");
    ASSERT(zenit_trie_insert(NULL, "key", 0).error == ZENIT_ERROR_NULL, "insert NULL trie");
    ASSERT(zenit_trie_insert(trie, NULL, 0).error == ZENIT_ERROR_NULL, "insert NULL key");
    ASSERT(zenit_trie_delete(NULL, "key").error == ZENIT_ERROR_NULL, "delete NULL trie");
    ASSERT(zenit_trie_delete(trie, NULL).error == ZENIT_ERROR_NULL, "delete NULL key");
    ASSERT(zenit_trie_count(NULL) == 0, "count NULL");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_null_out_value(void) {
    zenit_trie_t *trie = zenit_trie_create();
    zenit_trie_insert(trie, "test", 123);
    ASSERT(zenit_trie_search(trie, "test", NULL) == 1, "NULL out_value");
    zenit_trie_destroy(trie);
    return 0;
}

static int test_null_destroy_clear(void) {
    zenit_trie_destroy(NULL);
    zenit_trie_clear(NULL);
    return 0;
}

static int test_non_alpha_chars(void) {
    zenit_trie_t *trie = zenit_trie_create();
    int val;
    ASSERT(zenit_trie_insert(trie, "hello123world", 5).error == ZENIT_OK, "insert digits");
    ASSERT(zenit_trie_count(trie) == 1, "count 1");
    ASSERT(zenit_trie_search(trie, "helloworld", &val) == 1, "search helloworld");
    ASSERT(val == 5, "value 5");
    ASSERT(zenit_trie_starts_with(trie, "hello_world") == 1, "starts_with non-alpha");
    ASSERT(zenit_trie_delete(trie, "hello_world").error == ZENIT_OK, "delete non-alpha");
    ASSERT(zenit_trie_count(trie) == 0, "count 0 after delete");
    zenit_trie_destroy(trie);
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        &test_create_destroy,
        &test_empty_count,
        &test_search_miss_empty,
        &test_insert_and_search,
        &test_search_miss,
        &test_starts_with,
        &test_shared_prefix,
        &test_delete,
        &test_delete_nonexistent,
        &test_clear,
        &test_allocator_variant,
        &test_null_params,
        &test_null_out_value,
        &test_null_destroy_clear,
        &test_non_alpha_chars,
    };
    const char *names[] = {
        "create_destroy",
        "empty_count",
        "search_miss_empty",
        "insert_and_search",
        "search_miss",
        "starts_with",
        "shared_prefix",
        "delete",
        "delete_nonexistent",
        "clear",
        "allocator_variant",
        "null_params",
        "null_out_value",
        "null_destroy_clear",
        "non_alpha_chars",
    };
    ZENIT_RUN_TESTS("trie", tests, names);
}
