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

#include <libzenit/allocator.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_malloc_fail.h"

/* Need __wrap_realloc for default_realloc testing */
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

static int test_default_alloc_fail(void) {
    malloc_fail_countdown = 0;
    void *p = zenit_default_alloc(64, NULL);
    malloc_fail_countdown = -1;
    if (p != NULL) {
        fprintf(stderr, "FAIL: zenit_default_alloc should return NULL on malloc failure\n");
        return 1;
    }
    printf("  default_alloc_fail ... PASS\n");
    return 0;
}

static int test_default_realloc_fail(void) {
    void *p = zenit_default_alloc(64, NULL);
    if (p == NULL) {
        fprintf(stderr, "FAIL: setup alloc failed\n");
        return 1;
    }
    malloc_fail_countdown = 0;
    void *q = zenit_default_realloc(p, 128, NULL);
    malloc_fail_countdown = -1;
    if (q != NULL) {
        fprintf(stderr, "FAIL: zenit_default_realloc should return NULL on realloc failure\n");
        free(p);
        return 1;
    }
    /* p is still valid after realloc failure */
    free(p);
    printf("  default_realloc_fail ... PASS\n");
    return 0;
}

static int test_allocator_realloc_default_fail(void) {
    /* Test that zenit_allocator_realloc with default allocator returns NULL
     * when the underlying realloc fails */
    void *p = zenit_default_alloc(32, NULL);
    if (p == NULL) {
        fprintf(stderr, "FAIL: setup alloc failed\n");
        return 1;
    }

    malloc_fail_countdown = 0;
    void *q = zenit_allocator_realloc(ZENIT_ALLOCATOR_DEFAULT, p, 32, 64);
    malloc_fail_countdown = -1;
    if (q != NULL) {
        fprintf(stderr, "FAIL: zenit_allocator_realloc should return NULL on realloc failure\n");
        free(p);
        return 1;
    }
    /* p is still valid after realloc failure */
    free(p);
    printf("  allocator_realloc_default_fail ... PASS\n");
    return 0;
}

int main(void) {
    int failed = 0;
    printf("=== allocator malloc fail ===\n");
    if (test_default_alloc_fail()) failed++;
    if (test_default_realloc_fail()) failed++;
    if (test_allocator_realloc_default_fail()) failed++;
    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
