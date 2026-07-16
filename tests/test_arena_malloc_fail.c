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

#include <libzenit/arena.h>
#include <stdio.h>
#include <stdlib.h>

/* Control malloc/calloc failures across allocation sites.
 * -1  → never fail
 *  0  → fail next call
 *  >0 → fail after N successful calls
 */
static int malloc_fail_countdown = -1;

void *__real_malloc(size_t size);
void *__wrap_malloc(size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_malloc(size);
}

void *__real_calloc(size_t nmemb, size_t size);
void *__wrap_calloc(size_t nmemb, size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_calloc(nmemb, size);
}

/* Helper: run a test block with forced malloc/calloc failure at step N.
 * Resets countdown to -1 before any output to keep printf/stdio safe. */
#define FAIL_AT(step, label, body) do { \
    malloc_fail_countdown = (step);     \
    body                                \
    malloc_fail_countdown = -1;         \
    printf("PASS: %s\n", (label));      \
} while(0)

/* ─── Test: arena_create fails on handle malloc ─── */
static int test_create_handle_fail(void) {
    int ok = 1;
    FAIL_AT(0, "arena_create returns NULL on handle malloc failure", {
        zenit_arena_t *a = zenit_arena_create(1024 * 1024, 1024);
        if (a != NULL) { ok = 0; zenit_arena_destroy(a); }
    });
    return ok ? 0 : 1;
}

/* ─── Test: arena_create fails on memory calloc ─── */
static int test_create_memory_fail(void) {
    int ok = 1;
    FAIL_AT(1, "arena_create returns NULL on memory calloc failure", {
        zenit_arena_t *a = zenit_arena_create(1024 * 1024, 1024);
        if (a != NULL) { ok = 0; zenit_arena_destroy(a); }
    });
    return ok ? 0 : 1;
}

/* ─── Test: arena_create fails on bitmap calloc ─── */
static int test_create_bitmap_fail(void) {
    int ok = 1;
    FAIL_AT(2, "arena_create returns NULL on bitmap calloc failure", {
        zenit_arena_t *a = zenit_arena_create(1024 * 1024, 1024);
        if (a != NULL) { ok = 0; zenit_arena_destroy(a); }
    });
    return ok ? 0 : 1;
}

/* ─── Test: arena_acquire rolls back bitmap when malloc fails ─── */
static int test_acquire_ua_fail(void) {
    /* Create arena with no forced failures */
    malloc_fail_countdown = -1;
    zenit_arena_t *a = zenit_arena_create(100 * 1024 * 1024, 10 * 1024 * 1024);
    if (a == NULL) {
        fprintf(stderr, "FAIL: arena_create\n");
        return 1;
    }

    /* First acquire — fail malloc for the usable_arena handle */
    malloc_fail_countdown = 0;
    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * 1024 * 1024);
    if (ua != NULL) {
        fprintf(stderr, "FAIL: expected NULL on ua malloc failure\n");
        zenit_arena_release(a, ua);
        zenit_arena_destroy(a);
        malloc_fail_countdown = -1;
        return 1;
    }

    /* Reset countdown and verify bitmap was rolled back:
     * acquiring the same 30 MB should now succeed. */
    malloc_fail_countdown = -1;
    ua = zenit_arena_acquire(a, 30 * 1024 * 1024);
    if (ua == NULL) {
        fprintf(stderr, "FAIL: bitmap not rolled back after failed acquire\n");
        zenit_arena_destroy(a);
        return 1;
    }

    zenit_arena_release(a, ua);
    zenit_arena_destroy(a);

    printf("PASS: arena_acquire rolls back bitmap on malloc failure\n");
    return 0;
}

int main(void) {
    int ret = 0;

    ret |= test_create_handle_fail();
    ret |= test_create_memory_fail();
    ret |= test_create_bitmap_fail();
    ret |= test_acquire_ua_fail();

    return ret;
}
