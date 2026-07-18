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

#include <libzenit/str.h>
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── trim ─── */

static int test_trim_basic(void) {
    char *t = zenit_str_trim("  hello  ");
    ASSERT(t != NULL, "trim returned NULL");
    ASSERT(strcmp(t, "hello") == 0, "trim basic failed");
    free(t);
    return 0;
}

static int test_trim_left(void) {
    char *t = zenit_str_trim("\t\n hello");
    ASSERT(t != NULL, "trim left returned NULL");
    ASSERT(strcmp(t, "hello") == 0, "trim left failed");
    free(t);
    return 0;
}

static int test_trim_right(void) {
    char *t = zenit_str_trim("hello \t\n ");
    ASSERT(t != NULL, "trim right returned NULL");
    ASSERT(strcmp(t, "hello") == 0, "trim right failed");
    free(t);
    return 0;
}

static int test_trim_none(void) {
    char *t = zenit_str_trim("hello");
    ASSERT(t != NULL, "trim none returned NULL");
    ASSERT(strcmp(t, "hello") == 0, "trim none failed");
    free(t);
    return 0;
}

static int test_trim_all_whitespace(void) {
    char *t = zenit_str_trim("   \t\n  ");
    ASSERT(t != NULL, "trim all whitespace returned NULL");
    ASSERT(strcmp(t, "") == 0, "trim all whitespace should return empty");
    free(t);
    return 0;
}

static int test_trim_empty(void) {
    char *t = zenit_str_trim("");
    ASSERT(t != NULL, "trim empty returned NULL");
    ASSERT(strcmp(t, "") == 0, "trim empty should return empty");
    free(t);
    return 0;
}

static int test_trim_null(void) {
    ASSERT(zenit_str_trim(NULL) == NULL, "trim NULL should return NULL");
    return 0;
}

/* ─── split ─── */

static int test_split_basic(void) {
    size_t count;
    char **parts = zenit_str_split("a,b,c", ",", &count);
    ASSERT(parts != NULL, "split returned NULL");
    ASSERT(count == 3, "split count should be 3");
    ASSERT(strcmp(parts[0], "a") == 0, "split parts[0]");
    ASSERT(strcmp(parts[1], "b") == 0, "split parts[1]");
    ASSERT(strcmp(parts[2], "c") == 0, "split parts[2]");
    ASSERT(parts[3] == NULL, "split should be NULL-terminated");
    for (size_t i = 0; i < count; i++) free(parts[i]);
    free(parts);
    return 0;
}

static int test_split_consecutive(void) {
    size_t count;
    char **parts = zenit_str_split("a,,c", ",", &count);
    ASSERT(parts != NULL, "split consecutive returned NULL");
    ASSERT(count == 3, "split consecutive count should be 3");
    ASSERT(strcmp(parts[0], "a") == 0, "split consecutive parts[0]");
    ASSERT(strcmp(parts[1], "") == 0, "split consecutive parts[1]");
    ASSERT(strcmp(parts[2], "c") == 0, "split consecutive parts[2]");
    for (size_t i = 0; i < count; i++) free(parts[i]);
    free(parts);
    return 0;
}

static int test_split_empty(void) {
    size_t count;
    char **parts = zenit_str_split("", ",", &count);
    ASSERT(parts != NULL, "split empty returned NULL");
    /* An empty string split produces one empty token */
    ASSERT(count == 1, "split empty count should be 1");
    ASSERT(strcmp(parts[0], "") == 0, "split empty part should be empty");
    ASSERT(parts[1] == NULL, "split empty should be NULL-terminated");
    for (size_t i = 0; i < count; i++) free(parts[i]);
    free(parts);
    return 0;
}

static int test_split_no_delimiter(void) {
    size_t count;
    char **parts = zenit_str_split("hello", ",", &count);
    ASSERT(parts != NULL, "split no delim returned NULL");
    ASSERT(count == 1, "split no delim count should be 1");
    ASSERT(strcmp(parts[0], "hello") == 0, "split no delim content");
    for (size_t i = 0; i < count; i++) free(parts[i]);
    free(parts);
    return 0;
}

static int test_split_many(void) {
    /* 10 tokens triggers the realloc path (cap starts at 4) */
    size_t count;
    char **parts = zenit_str_split("a,b,c,d,e,f,g,h,i,j", ",", &count);
    ASSERT(parts != NULL, "split many returned NULL");
    ASSERT(count == 10, "split many count should be 10");
    for (size_t i = 0; i < count; i++) {
        ASSERT(strlen(parts[i]) == 1, "split many part length");
        ASSERT(parts[i][0] == 'a' + (char)i, "split many part content");
    }
    ASSERT(parts[count] == NULL, "split many NULL-terminated");
    for (size_t i = 0; i < count; i++) free(parts[i]);
    free(parts);
    return 0;
}

static int test_split_free_func(void) {
    size_t count;
    char **parts = zenit_str_split("a,b,c", ",", &count);
    ASSERT(parts != NULL, "split");
    zenit_str_split_free(parts, count, ZENIT_ALLOCATOR_DEFAULT);
    PASS();
    return 0;
}

static int test_split_null_params(void) {
    size_t count;
    ASSERT(zenit_str_split(NULL, ",", &count) == NULL, "split NULL s");
    ASSERT(zenit_str_split("hello", NULL, &count) == NULL, "split NULL delim");
    ASSERT(zenit_str_split("hello", ",", NULL) == NULL, "split NULL out_count");
    return 0;
}

/* ─── join ─── */

static int test_join_basic(void) {
    const char *parts[] = {"a", "b", "c"};
    char *j = zenit_str_join(parts, 3, ",");
    ASSERT(j != NULL, "join returned NULL");
    ASSERT(strcmp(j, "a,b,c") == 0, "join basic failed");
    free(j);
    return 0;
}

static int test_join_empty_delim(void) {
    const char *parts[] = {"a", "b", "c"};
    char *j = zenit_str_join(parts, 3, "");
    ASSERT(j != NULL, "join empty delim returned NULL");
    ASSERT(strcmp(j, "abc") == 0, "join empty delim failed");
    free(j);
    return 0;
}

static int test_join_empty_parts(void) {
    const char *j = zenit_str_join(NULL, 0, ",");
    ASSERT(j == NULL, "join NULL parts should return NULL");
    return 0;
}

static int test_join_zero_count(void) {
    char *j = zenit_str_join(NULL, 0, ",");
    ASSERT(j == NULL, "join zero count with NULL parts");
    const char *parts[] = {"a"};
    j = zenit_str_join(parts, 0, ",");
    ASSERT(j != NULL, "join zero count with valid parts");
    ASSERT(strcmp(j, "") == 0, "join zero count should be empty");
    free(j);
    return 0;
}

static int test_join_one_part(void) {
    const char *parts[] = {"hello"};
    char *j = zenit_str_join(parts, 1, ",");
    ASSERT(j != NULL, "join one part returned NULL");
    ASSERT(strcmp(j, "hello") == 0, "join one part failed");
    free(j);
    return 0;
}

static int test_join_null_param(void) {
    ASSERT(zenit_str_join(NULL, 1, ",") == NULL, "join NULL parts");
    const char *parts[] = {"a"};
    ASSERT(zenit_str_join(parts, 1, NULL) == NULL, "join NULL delim");
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        &test_trim_basic,
        &test_trim_left,
        &test_trim_right,
        &test_trim_none,
        &test_trim_all_whitespace,
        &test_trim_empty,
        &test_trim_null,
        &test_split_basic,
        &test_split_consecutive,
        &test_split_empty,
        &test_split_no_delimiter,
        &test_split_many,
        &test_split_null_params,
        &test_split_free_func,
        &test_join_basic,
        &test_join_empty_delim,
        &test_join_empty_parts,
        &test_join_zero_count,
        &test_join_one_part,
        &test_join_null_param,
    };
    const char *names[] = {
        "trim_basic",
        "trim_left",
        "trim_right",
        "trim_none",
        "trim_all_whitespace",
        "trim_empty",
        "trim_null",
        "split_basic",
        "split_consecutive",
        "split_empty",
        "split_no_delimiter",
        "split_many",
        "split_null_params",
        "join_basic",
        "join_empty_delim",
        "join_empty_parts",
        "join_zero_count",
        "join_one_part",
        "join_null_param",
    };
    ZENIT_RUN_TESTS("str", tests, names);
}
