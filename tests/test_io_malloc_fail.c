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
#include <libzenit/result.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <string.h>

#define TMPFILE "/tmp/libzenit_test_io_mf.tmp"

int main(void) {
    int failed = 0;

    /* Create a temp file first so read will reach the alloc call */
    {
        zenit_result_t r = zenit_file_write(TMPFILE, "hello", 5);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: could not create temp file\n");
            return 1;
        }
    }

    /* file_read: first alloc fails */
    {
        void *buf = NULL;
        size_t len = 0;

        malloc_fail_countdown = 0;
        zenit_result_t r = zenit_file_read(TMPFILE, &buf, &len);
        malloc_fail_countdown = -1;

        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: file_read alloc fail should return ZENIT_ERROR_ALLOC, got %d\n", r.error);
            failed++;
        } else {
            printf("PASS: file_read alloc fail\n");
        }
        free(buf);
    }

    /* file_read_with_allocator: first alloc fails */
    {
        void *buf = NULL;
        size_t len = 0;

        malloc_fail_countdown = 0;
        zenit_result_t r = zenit_file_read_with_allocator(TMPFILE, &buf, &len, ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;

        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: file_read_with_allocator alloc fail should return ZENIT_ERROR_ALLOC, got %d\n", r.error);
            failed++;
        } else {
            printf("PASS: file_read_with_allocator alloc fail\n");
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
