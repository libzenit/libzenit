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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); return 1; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); } } while (0)

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
    char *j = zenit_str_join(NULL, 0, ",");
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
    int failed = 0;

    printf("=== str ===\n");

    /* trim */
    printf("  trim_basic ... "); if (test_trim_basic()) failed++; else printf("PASS\n");
    printf("  trim_left ... "); if (test_trim_left()) failed++; else printf("PASS\n");
    printf("  trim_right ... "); if (test_trim_right()) failed++; else printf("PASS\n");
    printf("  trim_none ... "); if (test_trim_none()) failed++; else printf("PASS\n");
    printf("  trim_all_whitespace ... "); if (test_trim_all_whitespace()) failed++; else printf("PASS\n");
    printf("  trim_empty ... "); if (test_trim_empty()) failed++; else printf("PASS\n");
    printf("  trim_null ... "); if (test_trim_null()) failed++; else printf("PASS\n");

    /* split */
    printf("  split_basic ... "); if (test_split_basic()) failed++; else printf("PASS\n");
    printf("  split_consecutive ... "); if (test_split_consecutive()) failed++; else printf("PASS\n");
    printf("  split_empty ... "); if (test_split_empty()) failed++; else printf("PASS\n");
    printf("  split_no_delimiter ... "); if (test_split_no_delimiter()) failed++; else printf("PASS\n");
    printf("  split_null_params ... "); if (test_split_null_params()) failed++; else printf("PASS\n");

    /* join */
    printf("  join_basic ... "); if (test_join_basic()) failed++; else printf("PASS\n");
    printf("  join_empty_delim ... "); if (test_join_empty_delim()) failed++; else printf("PASS\n");
    printf("  join_empty_parts ... "); if (test_join_empty_parts()) failed++; else printf("PASS\n");
    printf("  join_zero_count ... "); if (test_join_zero_count()) failed++; else printf("PASS\n");
    printf("  join_one_part ... "); if (test_join_one_part()) failed++; else printf("PASS\n");
    printf("  join_null_param ... "); if (test_join_null_param()) failed++; else printf("PASS\n");

    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
