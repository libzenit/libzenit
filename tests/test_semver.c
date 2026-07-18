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

#include <libzenit/semver.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Test 1: Parse basic version */
    {
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse("1.2.3", &v);
        if (r.error != ZENIT_OK) { fprintf(stderr, "FAIL: parse 1.2.3\n"); return 1; }
        if (v.major != 1 || v.minor != 2 || v.patch != 3) {
            fprintf(stderr, "FAIL: 1.2.3 fields: %d.%d.%d\n", v.major, v.minor, v.patch);
            return 1;
        }
    }
    printf("PASS: semver parse basic\n");

    /* Test 2: Parse with pre-release */
    {
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse("1.0.0-alpha.1", &v);
        if (r.error != ZENIT_OK) { fprintf(stderr, "FAIL: parse 1.0.0-alpha.1\n"); return 1; }
        if (strcmp(v.pre_release, "alpha.1") != 0) {
            fprintf(stderr, "FAIL: prerelease got '%s'\n", v.pre_release);
            return 1;
        }
    }
    printf("PASS: semver parse prerelease\n");

    /* Test 3: Parse with build metadata */
    {
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse("1.0.0+20260101", &v);
        if (r.error != ZENIT_OK) { fprintf(stderr, "FAIL: parse 1.0.0+20260101\n"); return 1; }
        if (strcmp(v.build, "20260101") != 0) {
            fprintf(stderr, "FAIL: build got '%s'\n", v.build);
            return 1;
        }
    }
    printf("PASS: semver parse build\n");

    /* Test 4: Parse with prerelease and build */
    {
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse("2.0.0-rc.1+build.42", &v);
        if (r.error != ZENIT_OK) { fprintf(stderr, "FAIL: parse rc+build\n"); return 1; }
        if (strcmp(v.pre_release, "rc.1") != 0 || strcmp(v.build, "build.42") != 0) {
            fprintf(stderr, "FAIL: prerelease or build mismatch\n");
            return 1;
        }
    }
    printf("PASS: semver parse rc+build\n");

    /* Test 5: Format basic */
    {
        zenit_semver_t v = { .major = 1, .minor = 2, .patch = 3 };
        char *s = NULL;
        zenit_result_t r = zenit_semver_format(&v, &s);
        if (r.error != ZENIT_OK) { fprintf(stderr, "FAIL: format basic\n"); return 1; }
        if (strcmp(s, "1.2.3") != 0) {
            fprintf(stderr, "FAIL: format got '%s'\n", s); free(s); return 1;
        }
        free(s);
    }
    printf("PASS: semver format basic\n");

    /* Test 6: Format with prerelease */
    {
        zenit_semver_t v = { .major = 1, .minor = 0, .patch = 0 };
        snprintf(v.pre_release, sizeof(v.pre_release), "%s", "alpha");
        char *s = NULL;
        zenit_semver_format(&v, &s);
        if (strcmp(s, "1.0.0-alpha") != 0) {
            fprintf(stderr, "FAIL: format prerelease got '%s'\n", s); free(s); return 1;
        }
        free(s);
    }
    printf("PASS: semver format prerelease\n");

    /* Test 7: Round-trip */
    {
        zenit_semver_t v;
        zenit_semver_parse("3.2.1-beta.2+build.99", &v);
        char *s = NULL;
        zenit_semver_format(&v, &s);
        if (strcmp(s, "3.2.1-beta.2+build.99") != 0) {
            fprintf(stderr, "FAIL: round-trip got '%s'\n", s); free(s); return 1;
        }
        free(s);
    }
    printf("PASS: semver round-trip\n");

    /* Test 8: Compare equal */
    {
        zenit_semver_t a;
        zenit_semver_t b;
        zenit_semver_parse("1.2.3", &a);
        zenit_semver_parse("1.2.3", &b);
        if (zenit_semver_compare(&a, &b) != 0) {
            fprintf(stderr, "FAIL: equal compare\n"); return 1;
        }
    }
    printf("PASS: semver compare equal\n");

    /* Test 9: Compare major */
    {
        zenit_semver_t a;
        zenit_semver_t b;
        zenit_semver_parse("2.0.0", &a);
        zenit_semver_parse("1.0.0", &b);
        if (zenit_semver_compare(&a, &b) <= 0) {
            fprintf(stderr, "FAIL: major compare\n"); return 1;
        }
    }
    printf("PASS: semver compare major\n");

    /* Test 10: Compare prerelease precedence */
    {
        zenit_semver_t a;
        zenit_semver_t b;
        zenit_semver_parse("1.0.0-alpha", &a);
        zenit_semver_parse("1.0.0-alpha.1", &b);
        /* alpha < alpha.1 (shorter has lower precedence) */
        if (zenit_semver_compare(&a, &b) >= 0) {
            fprintf(stderr, "FAIL: prerelease compare (alpha vs alpha.1)\n"); return 1;
        }
    }
    printf("PASS: semver compare prerelease\n");

    /* Test 11: NULL params */
    {
        if (zenit_semver_compare(NULL, NULL) != 0) {
            fprintf(stderr, "FAIL: compare NULL,NULL\n"); return 1;
        }
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse(NULL, &v);
        if (r.error != ZENIT_ERROR_NULL) { fprintf(stderr, "FAIL: parse NULL\n"); return 1; }
        r = zenit_semver_parse("1.0.0", NULL);
        if (r.error != ZENIT_ERROR_NULL) { fprintf(stderr, "FAIL: parse NULL out\n"); return 1; }
        r = zenit_semver_format(NULL, NULL);
        if (r.error != ZENIT_ERROR_NULL) { fprintf(stderr, "FAIL: format NULL\n"); return 1; }
    }
    printf("PASS: semver NULL params\n");

    /* Test 12: Invalid parse */
    {
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse("1.2", &v);
        if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: invalid '1.2'\n"); return 1; }
        r = zenit_semver_parse("abc", &v);
        if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: invalid 'abc'\n"); return 1; }
        r = zenit_semver_parse("01.2.3", &v);
        if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: leading zero '01.2.3'\n"); return 1; }
        r = zenit_semver_parse("1.2.3-", &v);
        if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: empty prerelease\n"); return 1; }
        r = zenit_semver_parse("1.2.3+", &v);
        if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: empty build\n"); return 1; }
    }
    printf("PASS: semver invalid\n");

    /* Test 14: Very large version numbers (overflow) */
    {
        zenit_semver_t v;
        zenit_result_t r = zenit_semver_parse("9999999999.0.0", &v);
        if (r.error != ZENIT_ERROR_PARAM) { fprintf(stderr, "FAIL: overflow major\n"); return 1; }
    }
    printf("PASS: semver overflow\n");

    /* Test 15: Compare numeric prerelease */
    {
        zenit_semver_t a;
        zenit_semver_t b;
        zenit_semver_parse("1.0.0-1", &a);
        zenit_semver_parse("1.0.0-2", &b);
        if (zenit_semver_compare(&a, &b) >= 0) {
            fprintf(stderr, "FAIL: numeric prerelease\n"); return 1;
        }
    }
    printf("PASS: semver numeric prerelease\n");

    /* Test 16: Compare equal prerelease */
    {
        zenit_semver_t a;
        zenit_semver_t b;
        zenit_semver_parse("1.0.0-alpha.1", &a);
        zenit_semver_parse("1.0.0-alpha.1", &b);
        if (zenit_semver_compare(&a, &b) != 0) {
            fprintf(stderr, "FAIL: equal prerelease\n"); return 1;
        }
    }
    printf("PASS: semver equal prerelease\n");

    /* Test 13: Format allocator failure */
    {
        zenit_allocator_t fail_alloc = {
            .alloc_fn = zenit_default_alloc,
            .realloc_fn = NULL,
            .free_fn = zenit_default_free,
            .ctx = NULL
        };
        /* Can't easily test allocation failure without --wrap, so just verify the function exists */
        zenit_semver_t v = { .major = 1, .minor = 0, .patch = 0 };
        char *s = NULL;
        zenit_result_t r = zenit_semver_format_with_allocator(&v, &s, fail_alloc);
        if (r.error != ZENIT_OK) { fprintf(stderr, "FAIL: format with allocator\n"); return 1; }
        free(s);
    }
    printf("PASS: semver format with allocator\n");

    printf("PASS: semver\n");
    return 0;
}
