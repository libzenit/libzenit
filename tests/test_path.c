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

#include <libzenit/path.h>
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── join ─── */

static int test_join_basic(void) {
    char *p = zenit_path_join("foo", "bar");
    ASSERT(p != NULL, "join returned NULL");
    ASSERT(strcmp(p, "foo/bar") == 0, "join basic failed");
    free(p);
    return 0;
}

static int test_join_trailing_slash_a(void) {
    char *p = zenit_path_join("foo/", "bar");
    ASSERT(p != NULL, "join trailing slash returned NULL");
    ASSERT(strcmp(p, "foo/bar") == 0, "join trailing slash a failed");
    free(p);
    return 0;
}

static int test_join_leading_slash_b(void) {
    char *p = zenit_path_join("foo", "/bar");
    ASSERT(p != NULL, "join leading slash returned NULL");
    ASSERT(strcmp(p, "foo/bar") == 0, "join leading slash b failed");
    free(p);
    return 0;
}

static int test_join_both_slashes(void) {
    char *p = zenit_path_join("foo/", "/bar");
    ASSERT(p != NULL, "join both slashes returned NULL");
    ASSERT(strcmp(p, "foo/bar") == 0, "join both slashes failed");
    free(p);
    return 0;
}

/* ─── dirname ─── */

static int test_dirname_simple(void) {
    char *d = zenit_path_dirname("foo/bar");
    ASSERT(d != NULL, "dirname returned NULL");
    ASSERT(strcmp(d, "foo") == 0, "dirname simple failed");
    free(d);
    return 0;
}

static int test_dirname_root(void) {
    char *d = zenit_path_dirname("/foo");
    ASSERT(d != NULL, "dirname root returned NULL");
    ASSERT(strcmp(d, "/") == 0, "dirname root failed");
    free(d);
    return 0;
}

static int test_dirname_no_slash(void) {
    char *d = zenit_path_dirname("foo");
    ASSERT(d != NULL, "dirname no slash returned NULL");
    ASSERT(strcmp(d, ".") == 0, "dirname no slash failed");
    free(d);
    return 0;
}

/* ─── basename ─── */

static int test_basename_simple(void) {
    char *b = zenit_path_basename("foo/bar");
    ASSERT(b != NULL, "basename returned NULL");
    ASSERT(strcmp(b, "bar") == 0, "basename simple failed");
    free(b);
    return 0;
}

static int test_basename_root(void) {
    char *b = zenit_path_basename("/foo");
    ASSERT(b != NULL, "basename root returned NULL");
    ASSERT(strcmp(b, "foo") == 0, "basename root failed");
    free(b);
    return 0;
}

static int test_basename_no_slash(void) {
    char *b = zenit_path_basename("foo");
    ASSERT(b != NULL, "basename no slash returned NULL");
    ASSERT(strcmp(b, "foo") == 0, "basename no slash failed");
    free(b);
    return 0;
}

/* ─── extension ─── */

static int test_extension_simple(void) {
    char *e = zenit_path_extension("foo.txt");
    ASSERT(e != NULL, "extension returned NULL");
    ASSERT(strcmp(e, ".txt") == 0, "extension simple failed");
    free(e);
    return 0;
}

static int test_extension_none(void) {
    char *e = zenit_path_extension("foo");
    ASSERT(e != NULL, "extension none returned NULL");
    ASSERT(strcmp(e, "") == 0, "extension none failed");
    free(e);
    return 0;
}

static int test_extension_hidden(void) {
    char *e = zenit_path_extension(".hidden");
    ASSERT(e != NULL, "extension hidden returned NULL");
    ASSERT(strcmp(e, "") == 0, "extension hidden failed");
    free(e);
    return 0;
}

static int test_extension_multi_dot(void) {
    char *e = zenit_path_extension("archive.tar.gz");
    ASSERT(e != NULL, "extension multi dot returned NULL");
    ASSERT(strcmp(e, ".gz") == 0, "extension multi dot failed");
    free(e);
    return 0;
}

/* ─── normalize ─── */

static int test_normalize_dot_dot(void) {
    char *n = zenit_path_normalize("foo/bar/../baz");
    ASSERT(n != NULL, "normalize dot dot returned NULL");
    ASSERT(strcmp(n, "foo/baz") == 0, "normalize dot dot failed");
    free(n);
    return 0;
}

static int test_normalize_double_slash(void) {
    char *n = zenit_path_normalize("foo//bar");
    ASSERT(n != NULL, "normalize double slash returned NULL");
    ASSERT(strcmp(n, "foo/bar") == 0, "normalize double slash failed");
    free(n);
    return 0;
}

static int test_normalize_absolute(void) {
    char *n = zenit_path_normalize("/foo/bar/../baz");
    ASSERT(n != NULL, "normalize absolute returned NULL");
    ASSERT(strcmp(n, "/foo/baz") == 0, "normalize absolute failed");
    free(n);
    return 0;
}

static int test_normalize_single_dot(void) {
    char *n = zenit_path_normalize("foo/./bar");
    ASSERT(n != NULL, "normalize single dot returned NULL");
    ASSERT(strcmp(n, "foo/bar") == 0, "normalize single dot failed");
    free(n);
    return 0;
}

static int test_normalize_trailing_slash(void) {
    char *n = zenit_path_normalize("foo/bar/");
    ASSERT(n != NULL, "normalize trailing slash returned NULL");
    ASSERT(strcmp(n, "foo/bar") == 0, "normalize trailing slash failed");
    free(n);
    return 0;
}

/* ─── NULL params ─── */

static int test_null_params(void) {
    ASSERT(zenit_path_join(NULL, "bar") == NULL, "join NULL a");
    ASSERT(zenit_path_join("foo", NULL) == NULL, "join NULL b");
    ASSERT(zenit_path_dirname(NULL) == NULL, "dirname NULL");
    ASSERT(zenit_path_basename(NULL) == NULL, "basename NULL");
    ASSERT(zenit_path_extension(NULL) == NULL, "extension NULL");
    ASSERT(zenit_path_normalize(NULL) == NULL, "normalize NULL");
    return 0;
}

/* ─── allocator variants ─── */

static int test_allocator_variants(void) {
    zenit_allocator_t a = ZENIT_ALLOCATOR_DEFAULT;
    char *p;

    p = zenit_path_join_with_allocator("a", "b", a);
    ASSERT(p != NULL, "join allocator returned NULL");
    ASSERT(strcmp(p, "a/b") == 0, "join allocator failed");
    free(p);

    p = zenit_path_dirname_with_allocator("a/b", a);
    ASSERT(p != NULL, "dirname allocator returned NULL");
    ASSERT(strcmp(p, "a") == 0, "dirname allocator failed");
    free(p);

    p = zenit_path_basename_with_allocator("a/b", a);
    ASSERT(p != NULL, "basename allocator returned NULL");
    ASSERT(strcmp(p, "b") == 0, "basename allocator failed");
    free(p);

    p = zenit_path_extension_with_allocator("a.txt", a);
    ASSERT(p != NULL, "extension allocator returned NULL");
    ASSERT(strcmp(p, ".txt") == 0, "extension allocator failed");
    free(p);

    p = zenit_path_normalize_with_allocator("a/./b", a);
    ASSERT(p != NULL, "normalize allocator returned NULL");
    ASSERT(strcmp(p, "a/b") == 0, "normalize allocator failed");
    free(p);

    return 0;
}

static int test_dirname_empty(void) {
    char *d = zenit_path_dirname("");
    ASSERT(d != NULL, "dirname empty returned NULL");
    ASSERT(strcmp(d, ".") == 0, "dirname empty failed");
    free(d);
    return 0;
}

static int test_basename_empty(void) {
    char *b = zenit_path_basename("");
    ASSERT(b != NULL, "basename empty returned NULL");
    ASSERT(strcmp(b, "") == 0, "basename empty failed");
    free(b);
    return 0;
}

static int test_extension_empty(void) {
    char *e = zenit_path_extension("");
    ASSERT(e != NULL, "extension empty returned NULL");
    ASSERT(strcmp(e, "") == 0, "extension empty failed");
    free(e);
    return 0;
}

static int test_normalize_empty(void) {
    char *n = zenit_path_normalize("");
    ASSERT(n != NULL, "normalize empty returned NULL");
    ASSERT(strcmp(n, "/") == 0, "normalize empty failed");
    free(n);
    return 0;
}

/* ─── allocator helpers for test_normalize_alloc_fail ─── */

typedef struct {
    int count;
    int fail_after;
} alloc_test_ctx_t;

static void* test_alloc_count(size_t size, void *ctx) {
    alloc_test_ctx_t *c = (alloc_test_ctx_t*)ctx;
    c->count++;
    if (c->count > c->fail_after) return NULL;
    return malloc(size);
}

static void test_free_count(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

static int test_normalize_empty_absolute(void) {
    char *n = zenit_path_normalize("//");
    ASSERT(n != NULL, "normalize // returned NULL");
    ASSERT(strcmp(n, "/") == 0, "normalize // should return /");
    free(n);
    return 0;
}

static int test_normalize_alloc_fail(void) {
    alloc_test_ctx_t c_ctx = { .count = 0, .fail_after = 1 };

    zenit_allocator_t a = {
        .alloc_fn = test_alloc_count,
        .realloc_fn = NULL,
        .free_fn = test_free_count,
        .ctx = &c_ctx,
    };

    /* normalize_impl allocates stack buffer (first call succeeds),
     * then allocates output string (second call fails) */
    const char *r = zenit_path_normalize_with_allocator("foo/bar", a);
    ASSERT(r == NULL, "normalize should return NULL on second alloc failure");
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        &test_join_basic,
        &test_join_trailing_slash_a,
        &test_join_leading_slash_b,
        &test_join_both_slashes,
        &test_dirname_simple,
        &test_dirname_root,
        &test_dirname_no_slash,
        &test_basename_simple,
        &test_basename_root,
        &test_basename_no_slash,
        &test_extension_simple,
        &test_extension_none,
        &test_extension_hidden,
        &test_extension_multi_dot,
        &test_normalize_dot_dot,
        &test_normalize_double_slash,
        &test_normalize_absolute,
        &test_normalize_single_dot,
        &test_normalize_trailing_slash,
        &test_null_params,
        &test_allocator_variants,
        &test_dirname_empty,
        &test_basename_empty,
        &test_extension_empty,
        &test_normalize_empty,
        &test_normalize_empty_absolute,
        &test_normalize_alloc_fail,
    };
    const char *names[] = {
        "join_basic",
        "join_trailing_slash_a",
        "join_leading_slash_b",
        "join_both_slashes",
        "dirname_simple",
        "dirname_root",
        "dirname_no_slash",
        "basename_simple",
        "basename_root",
        "basename_no_slash",
        "extension_simple",
        "extension_none",
        "extension_hidden",
        "extension_multi_dot",
        "normalize_dot_dot",
        "normalize_double_slash",
        "normalize_absolute",
        "normalize_single_dot",
        "normalize_trailing_slash",
        "null_params",
        "allocator_variants",
        "dirname_empty",
        "basename_empty",
        "extension_empty",
        "normalize_empty",
        "normalize_empty_absolute",
        "normalize_alloc_fail",
    };
    ZENIT_RUN_TESTS("path", tests, names);
}
