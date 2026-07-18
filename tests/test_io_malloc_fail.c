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

#include <libzenit/io.h>
#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Custom allocator that fails on the Nth call */
typedef struct {
    int fail_after;
    int call_count;
} test_io_ctx_t;

static void *test_io_alloc(size_t size, void *ctx) {
    test_io_ctx_t *tc = (test_io_ctx_t *)ctx;
    tc->call_count++;
    if (tc->fail_after >= 0 && tc->call_count > tc->fail_after) {
        return NULL;
    }
    return malloc(size);
}

static void test_io_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

#define TMPFILE "libzenit_test_io_mf.tmp"

int main(void) {
    int failed = 0;

    /* Create temp file */
    {
        zenit_result_t r = zenit_file_write(TMPFILE, "hello", 5);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: could not create temp file\n");
            return 1;
        }
    }

    /* Test: file_read_with_allocator where alloc fails */
    {
        test_io_ctx_t ctx = { .fail_after = 0, .call_count = 0 };
        zenit_allocator_t a = {
            .alloc_fn = test_io_alloc,
            .free_fn = test_io_free,
            .ctx = &ctx
        };
        void *buf = NULL;
        size_t len = 0;
        zenit_result_t r = zenit_file_read_with_allocator(TMPFILE, &buf, &len, a);
        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: file_read alloc fail should return ZENIT_ERROR_ALLOC, got %d\n", r.error);
            failed++;
        } else {
            printf("PASS: file_read alloc fail\n");
        }
        free(buf);
    }

    /* Clean up */
    zenit_file_delete(TMPFILE);

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
