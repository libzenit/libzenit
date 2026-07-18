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
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int failed = 0;

    /* join: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *r = zenit_path_join("foo", "bar");
        malloc_fail_countdown = -1;
        if (r != NULL) {
            fprintf(stderr, "FAIL: join countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: join alloc fail\n");
        }
        free(r);
    }

    /* dirname: alloc fails */
    {
        malloc_fail_countdown = 0;
        char *r = zenit_path_dirname("foo/bar");
        malloc_fail_countdown = -1;
        if (r != NULL) {
            fprintf(stderr, "FAIL: dirname countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: dirname alloc fail\n");
        }
        free(r);
    }

    /* basename: alloc fails */
    {
        malloc_fail_countdown = 0;
        char *r = zenit_path_basename("foo/bar");
        malloc_fail_countdown = -1;
        if (r != NULL) {
            fprintf(stderr, "FAIL: basename countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: basename alloc fail\n");
        }
        free(r);
    }

    /* extension: alloc fails */
    {
        malloc_fail_countdown = 0;
        char *r = zenit_path_extension("foo.txt");
        malloc_fail_countdown = -1;
        if (r != NULL) {
            fprintf(stderr, "FAIL: extension countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: extension alloc fail\n");
        }
        free(r);
    }

    /* normalize: first alloc (stack buffer) fails */
    {
        malloc_fail_countdown = 0;
        char *r = zenit_path_normalize("foo/bar");
        malloc_fail_countdown = -1;
        if (r != NULL) {
            fprintf(stderr, "FAIL: normalize countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: normalize alloc fail\n");
        }
        free(r);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
