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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_malloc_fail.h"

void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_realloc(ptr, size);
}

static int failures = 0;
#define FAIL(msg) do { \
    fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    failures++; \
} while(0)

/* ─── Test: create fails on first malloc ─── */
static int test_create_fail(void) {
    malloc_fail_countdown = 0;
    zenit_string_t *s = zenit_string_create();
    malloc_fail_countdown = -1;
    if (s != NULL) {
        FAIL("expected NULL on malloc failure");
        zenit_string_destroy(s);
        return 1;
    }
    return 0;
}

/* ─── Test: create_from fails on malloc ─── */
static int test_create_from_fail(void) {
    malloc_fail_countdown = 0;
    zenit_string_t *s = zenit_string_create_from("hello");
    malloc_fail_countdown = -1;
    if (s != NULL) {
        FAIL("expected NULL on malloc failure");
        zenit_string_destroy(s);
        return 1;
    }
    return 0;
}

/* ─── Test: append eventually fails after several successes ─── */
static int test_append_fail_after_success(void) {
    /* Allow 50 allocations to succeed before failing.
       This gives create enough room and lets several appends succeed
       before the vector exhausts its reserved capacity and tries to grow. */
    malloc_fail_countdown = 50;

    zenit_string_t *s = zenit_string_create();
    if (s == NULL) {
        FAIL("create should succeed");
        return 1;
    }

    /* Append many small strings — will eventually hit malloc failure
       when the vector needs to grow and countdown is exhausted */
    int ok_count = 0;
    for (int i = 0; i < 1000; i++) {
        zenit_result_t r = zenit_string_append_cstr(s, "x");
        if (r.error == ZENIT_OK) {
            ok_count++;
        } else {
            break;
        }
    }

    if (ok_count == 0) {
        FAIL("expected at least one successful append");
        zenit_string_destroy(s);
        return 1;
    }

    /* Verify that partial content is valid (string invariants hold) */
    const char *cstr = zenit_string_cstr(s);
    if (cstr == NULL) {
        FAIL("cstr should not be NULL after partial append");
        zenit_string_destroy(s);
        return 1;
    }

    /* Verify null termination */
    size_t len = strlen(cstr);
    if (len != zenit_string_length(s)) {
        FAIL("strlen and length mismatch");
        zenit_string_destroy(s);
        return 1;
    }

    malloc_fail_countdown = -1;
    zenit_string_destroy(s);
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_fail();
    ret |= test_create_from_fail();
    ret |= test_append_fail_after_success();

    /* Reset countdown to avoid interfering with gcov/stdio cleanup at exit */
    malloc_fail_countdown = -1;

    if (failures > 0 || ret != 0) {
        fprintf(stderr, "FAIL: %d test(s) had errors\n", failures);
        return 1;
    }

    printf("PASS: string malloc fail tests\n");
    return 0;
}
