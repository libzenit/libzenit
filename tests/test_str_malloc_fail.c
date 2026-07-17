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

#include <libzenit/str.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int failed = 0;

    /* split: first malloc (result array) fails */
    {
        malloc_fail_countdown = 0;
        size_t count;
        char **parts = zenit_str_split("a,b,c", ",", &count);
        malloc_fail_countdown = -1;
        if (parts != NULL) {
            fprintf(stderr, "FAIL: split countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: split first alloc fail\n");
        }
        free(parts);
    }

    /* split: token malloc fails after result array succeeded.
     * "a,b,c" produces 3 tokens. The alloc sequence is:
     *   1. result array (4 pointers)
     *   2. "a" (2 bytes)
     *   3. "b" (2 bytes)
     *   4. "c" (2 bytes)
     * With countdown=3, alloc #3 ("b") fails. */
    {
        malloc_fail_countdown = 3;
        size_t count;
        char **parts = zenit_str_split("a,b,c", ",", &count);
        malloc_fail_countdown = -1;
        if (parts != NULL) {
            fprintf(stderr, "FAIL: split countdown=3 should return NULL\n");
            failed++;
        } else {
            printf("PASS: split token alloc fail\n");
        }
        free(parts);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
