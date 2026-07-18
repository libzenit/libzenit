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

#include <libzenit/dir.h>
#include <libzenit/io.h>
#include <libzenit/result.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_malloc_fail.h"

#define TMP_DIR "libzenit_test_dir_mf"

/* ─── Test: zenit_dir_iter fails on handle malloc ─── */
static int test_iter_malloc_fail(void) {
    /* First ensure the directory exists */
    zenit_result_t r = zenit_dir_create(TMP_DIR);
    if (r.error != ZENIT_OK) {
        fprintf(stderr, "FAIL: could not create temp dir\n");
        return 1;
    }

    /* Fail the very first malloc — should be the iterator struct */
    malloc_fail_countdown = 0;
    zenit_dir_iter_t *iter = zenit_dir_iter(TMP_DIR);
    malloc_fail_countdown = -1;

    if (iter != NULL) {
        fprintf(stderr, "FAIL: expected NULL on iter malloc failure\n");
        zenit_dir_iter_destroy(iter);
        return 1;
    }
    printf("PASS: dir_iter returns NULL on malloc failure\n");
    return 0;
}

/* ─── Test: zenit_dir_list handles allocation failure ─── */
static int test_list_malloc_fail(void) {
    /* Create a file so the directory has at least one entry */
    zenit_file_write(TMP_DIR "/f.txt", "x", 1);

    /*
     * The --wrap=malloc interposes on all malloc calls, including any
     * internal allocations inside libc (e.g. opendir).  Because the exact
     * count depends on the libc implementation, we cannot reliably target
     * a specific malloc call inside dir_list.  Instead, just verify that
     * dir_list handles an early allocation failure gracefully: it should
     * return either ZENIT_ERROR_ALLOC or ZENIT_ERROR_NOT_FOUND.
     */
    malloc_fail_countdown = 0;
    char **names = NULL;
    size_t count = 0;
    zenit_result_t r = zenit_dir_list(TMP_DIR, &names, &count);
    malloc_fail_countdown = -1;

    if (r.error != ZENIT_ERROR_ALLOC && r.error != ZENIT_ERROR_NOT_FOUND) {
        fprintf(stderr, "FAIL: expected ZENIT_ERROR_ALLOC or NOT_FOUND on malloc failure, got %d\n",
                r.error);
        return 1;
    }
    printf("PASS: dir_list handles malloc failure (error=%d)\n", r.error);
    return 0;
}

int main(void) {
    int failed = 0;

    /* Clean any leftover from a previous run */
    (void)zenit_dir_remove(TMP_DIR);

    failed |= test_iter_malloc_fail();
    failed |= test_list_malloc_fail();

    /* Clean up */
    (void)zenit_dir_remove(TMP_DIR);

    if (failed) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
