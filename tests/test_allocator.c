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
#include <string.h>

typedef struct {
    int fail_countdown;
} test_ctx_t;

static void *test_alloc(size_t size, void *ctx) {
    test_ctx_t *tc = (test_ctx_t *)ctx;
    if (tc->fail_countdown == 0) {
        return NULL;
    }
    if (tc->fail_countdown > 0) {
        tc->fail_countdown--;
    }
    return malloc(size);
}

static void test_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

static int test_default_alloc_realloc_free(void) {
    void *p = zenit_default_alloc(64, NULL);
    if (p == NULL) {
        fprintf(stderr, "FAIL: default_alloc returned NULL\n");
        return 1;
    }
    memset(p, 0xAB, 64);

    void *q = zenit_default_realloc(p, 128, NULL);
    if (q == NULL) {
        fprintf(stderr, "FAIL: default_realloc returned NULL\n");
        free(p);
        return 1;
    }
    /* Check first 64 bytes survived the realloc */
    unsigned char *buf = (unsigned char *)q;
    for (int i = 0; i < 64; i++) {
        if (buf[i] != 0xAB) {
            fprintf(stderr, "FAIL: default_realloc data corrupted at byte %d\n", i);
            zenit_default_free(q, NULL);
            return 1;
        }
    }

    zenit_default_free(q, NULL);

    printf("PASS: default_alloc_realloc_free\n");
    return 0;
}

static int test_alloc_zero_zeroes_memory(void) {
    zenit_allocator_t a = ZENIT_ALLOCATOR_DEFAULT;
    void *p = zenit_allocator_alloc_zero(a, 10, sizeof(int));
    if (p == NULL) {
        fprintf(stderr, "FAIL: alloc_zero returned NULL\n");
        return 1;
    }
    int *arr = (int *)p;
    for (int i = 0; i < 10; i++) {
        if (arr[i] != 0) {
            fprintf(stderr, "FAIL: alloc_zero memory not zeroed at index %d\n", i);
            free(p);
            return 1;
        }
    }
    free(p);
    printf("PASS: alloc_zero_zeroes_memory\n");
    return 0;
}

static int test_realloc_fallback_no_realloc_fn(void) {
    test_ctx_t ctx = { .fail_countdown = -1 };
    zenit_allocator_t a = {
        .alloc_fn = test_alloc,
        .realloc_fn = NULL,
        .free_fn = test_free,
        .ctx = &ctx
    };

    void *p = a.alloc_fn(32, a.ctx);
    if (p == NULL) {
        fprintf(stderr, "FAIL: custom alloc returned NULL\n");
        return 1;
    }
    memset(p, 0xCD, 32);

    void *q = zenit_allocator_realloc(a, p, 32, 64);
    if (q == NULL) {
        fprintf(stderr, "FAIL: realloc fallback returned NULL\n");
        a.free_fn(p, a.ctx);
        return 1;
    }

    unsigned char *buf = (unsigned char *)q;
    for (int i = 0; i < 32; i++) {
        if (buf[i] != 0xCD) {
            fprintf(stderr, "FAIL: realloc fallback data corrupted at byte %d\n", i);
            a.free_fn(q, a.ctx);
            return 1;
        }
    }

    a.free_fn(q, a.ctx);
    printf("PASS: realloc_fallback_no_realloc_fn\n");
    return 0;
}

static int test_realloc_fallback_null_ptr(void) {
    test_ctx_t ctx = { .fail_countdown = -1 };
    zenit_allocator_t a = {
        .alloc_fn = test_alloc,
        .realloc_fn = NULL,
        .free_fn = test_free,
        .ctx = &ctx
    };

    /* Passing NULL ptr should behave like alloc */
    void *p = zenit_allocator_realloc(a, NULL, 0, 64);
    if (p == NULL) {
        fprintf(stderr, "FAIL: realloc fallback NULL ptr returned NULL\n");
        return 1;
    }
    a.free_fn(p, a.ctx);
    printf("PASS: realloc_fallback_null_ptr\n");
    return 0;
}

static int test_realloc_fallback_shrink(void) {
    test_ctx_t ctx = { .fail_countdown = -1 };
    zenit_allocator_t a = {
        .alloc_fn = test_alloc,
        .realloc_fn = NULL,
        .free_fn = test_free,
        .ctx = &ctx
    };

    void *p = a.alloc_fn(64, a.ctx);
    if (p == NULL) {
        fprintf(stderr, "FAIL: custom alloc returned NULL\n");
        return 1;
    }
    memset(p, 0xEF, 64);

    /* Shrink from 64 to 16 — only 16 bytes should be copied */
    void *q = zenit_allocator_realloc(a, p, 64, 16);
    if (q == NULL) {
        fprintf(stderr, "FAIL: realloc fallback shrink returned NULL\n");
        a.free_fn(p, a.ctx);
        return 1;
    }

    unsigned char *buf = (unsigned char *)q;
    for (int i = 0; i < 16; i++) {
        if (buf[i] != 0xEF) {
            fprintf(stderr, "FAIL: realloc fallback shrink data corrupted at byte %d\n", i);
            a.free_fn(q, a.ctx);
            return 1;
        }
    }

    a.free_fn(q, a.ctx);
    printf("PASS: realloc_fallback_shrink\n");
    return 0;
}

static int test_realloc_fallback_alloc_failure(void) {
    test_ctx_t ctx = { .fail_countdown = 0 };
    zenit_allocator_t a = {
        .alloc_fn = test_alloc,
        .realloc_fn = NULL,
        .free_fn = test_free,
        .ctx = &ctx
    };

    /* Allocation failure — should return NULL */
    void *p = zenit_allocator_realloc(a, NULL, 0, 64);
    if (p != NULL) {
        fprintf(stderr, "FAIL: realloc fallback should return NULL on alloc failure\n");
        a.free_fn(p, a.ctx);
        return 1;
    }
    printf("PASS: realloc_fallback_alloc_failure\n");
    return 0;
}

int main(void) {
    int failed = 0;

    printf("=== allocator ===\n");

    printf("  default_alloc_realloc_free ... ");
    if (test_default_alloc_realloc_free()) failed++;
    else printf("PASS\n");

    printf("  alloc_zero_zeroes_memory ... ");
    if (test_alloc_zero_zeroes_memory()) failed++;
    else printf("PASS\n");

    printf("  realloc_fallback_no_realloc_fn ... ");
    if (test_realloc_fallback_no_realloc_fn()) failed++;
    else printf("PASS\n");

    printf("  realloc_fallback_null_ptr ... ");
    if (test_realloc_fallback_null_ptr()) failed++;
    else printf("PASS\n");

    printf("  realloc_fallback_shrink ... ");
    if (test_realloc_fallback_shrink()) failed++;
    else printf("PASS\n");

    printf("  realloc_fallback_alloc_failure ... ");
    if (test_realloc_fallback_alloc_failure()) failed++;
    else printf("PASS\n");

    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
