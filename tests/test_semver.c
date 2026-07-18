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

#include "test_runner.h"
#include <libzenit/semver.h>
#include <string.h>
#include <stdlib.h>

static int test_parse_basic(void) {
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse("1.2.3", &v);
    ASSERT(r.error == ZENIT_OK, "parse basic");
    ASSERT(v.major == 1 && v.minor == 2 && v.patch == 3, "parse fields");
    PASS();
    return 0;
}

static int test_parse_prerelease(void) {
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse("1.0.0-alpha.1", &v);
    ASSERT(r.error == ZENIT_OK, "parse prerelease");
    ASSERT(strcmp(v.pre_release, "alpha.1") == 0, "prerelease field");
    PASS();
    return 0;
}

static int test_parse_build(void) {
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse("1.0.0+20260101", &v);
    ASSERT(r.error == ZENIT_OK, "parse build");
    ASSERT(strcmp(v.build, "20260101") == 0, "build field");
    PASS();
    return 0;
}

static int test_parse_rc_build(void) {
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse("2.0.0-rc.1+build.42", &v);
    ASSERT(r.error == ZENIT_OK, "parse rc+build");
    ASSERT(strcmp(v.pre_release, "rc.1") == 0, "prerelease rc");
    ASSERT(strcmp(v.build, "build.42") == 0, "build rc");
    PASS();
    return 0;
}

static int test_format_basic(void) {
    zenit_semver_t v = { .major = 1, .minor = 2, .patch = 3 };
    char *s = NULL;
    zenit_result_t r = zenit_semver_format(&v, &s);
    ASSERT(r.error == ZENIT_OK, "format basic");
    ASSERT(strcmp(s, "1.2.3") == 0, "format result");
    free(s);
    PASS();
    return 0;
}

static int test_format_prerelease(void) {
    zenit_semver_t v = { .major = 1, .minor = 0, .patch = 0 };
    snprintf(v.pre_release, sizeof(v.pre_release), "%s", "alpha");
    char *s = NULL;
    zenit_semver_format(&v, &s);
    ASSERT(strcmp(s, "1.0.0-alpha") == 0, "format prerelease");
    free(s);
    PASS();
    return 0;
}

static int test_round_trip(void) {
    zenit_semver_t v;
    zenit_semver_parse("3.2.1-beta.2+build.99", &v);
    char *s = NULL;
    zenit_semver_format(&v, &s);
    ASSERT(strcmp(s, "3.2.1-beta.2+build.99") == 0, "round-trip");
    free(s);
    PASS();
    return 0;
}

static int test_compare_equal(void) {
    zenit_semver_t a;
    zenit_semver_t b;
    zenit_semver_parse("1.2.3", &a);
    zenit_semver_parse("1.2.3", &b);
    ASSERT(zenit_semver_compare(&a, &b) == 0, "equal compare");
    PASS();
    return 0;
}

static int test_compare_major(void) {
    zenit_semver_t a;
    zenit_semver_t b;
    zenit_semver_parse("2.0.0", &a);
    zenit_semver_parse("1.0.0", &b);
    ASSERT(zenit_semver_compare(&a, &b) > 0, "major compare");
    PASS();
    return 0;
}

static int test_compare_prerelease(void) {
    zenit_semver_t a;
    zenit_semver_t b;
    zenit_semver_parse("1.0.0-alpha", &a);
    zenit_semver_parse("1.0.0-alpha.1", &b);
    ASSERT(zenit_semver_compare(&a, &b) < 0, "prerelease compare");
    PASS();
    return 0;
}

static int test_compare_numeric(void) {
    zenit_semver_t a;
    zenit_semver_t b;
    zenit_semver_parse("1.0.0-1", &a);
    zenit_semver_parse("1.0.0-2", &b);
    ASSERT(zenit_semver_compare(&a, &b) < 0, "numeric prerelease");
    PASS();
    return 0;
}

static int test_compare_equal_prerelease(void) {
    zenit_semver_t a;
    zenit_semver_t b;
    zenit_semver_parse("1.0.0-alpha.1", &a);
    zenit_semver_parse("1.0.0-alpha.1", &b);
    ASSERT(zenit_semver_compare(&a, &b) == 0, "equal prerelease");
    PASS();
    return 0;
}

static int test_null_params(void) {
    ASSERT(zenit_semver_compare(NULL, NULL) == 0, "compare NULL");
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse(NULL, &v);
    ASSERT(r.error == ZENIT_ERROR_NULL, "parse NULL");
    r = zenit_semver_parse("1.0.0", NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "parse NULL out");
    r = zenit_semver_format(NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "format NULL");
    PASS();
    return 0;
}

static int test_invalid(void) {
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse("1.2", &v);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "invalid 1.2");
    r = zenit_semver_parse("abc", &v);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "invalid abc");
    r = zenit_semver_parse("01.2.3", &v);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "leading zero");
    r = zenit_semver_parse("1.2.3-", &v);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "empty prerelease");
    r = zenit_semver_parse("1.2.3+", &v);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "empty build");
    PASS();
    return 0;
}

static int test_overflow(void) {
    zenit_semver_t v;
    zenit_result_t r = zenit_semver_parse("9999999999.0.0", &v);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "overflow major");
    PASS();
    return 0;
}

static int test_allocator(void) {
    zenit_semver_t v = { .major = 1, .minor = 0, .patch = 0 };
    char *s = NULL;
    zenit_result_t r = zenit_semver_format_with_allocator(&v, &s, ZENIT_ALLOCATOR_DEFAULT);
    ASSERT(r.error == ZENIT_OK, "format allocator");
    free(s);
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_parse_basic,
        test_parse_prerelease,
        test_parse_build,
        test_parse_rc_build,
        test_format_basic,
        test_format_prerelease,
        test_round_trip,
        test_compare_equal,
        test_compare_major,
        test_compare_prerelease,
        test_compare_numeric,
        test_compare_equal_prerelease,
        test_null_params,
        test_invalid,
        test_overflow,
        test_allocator,
    };
    const char *names[] = {
        "parse_basic",
        "parse_prerelease",
        "parse_build",
        "parse_rc_build",
        "format_basic",
        "format_prerelease",
        "round_trip",
        "compare_equal",
        "compare_major",
        "compare_prerelease",
        "compare_numeric",
        "compare_equal_prerelease",
        "null_params",
        "invalid",
        "overflow",
        "allocator",
    };
    ZENIT_RUN_TESTS("semver", tests, names);
    return 0;
}
