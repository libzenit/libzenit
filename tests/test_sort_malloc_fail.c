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

#include <libzenit/sort.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

static int test_stable_sort_malloc_fail(void) {
    int arr[] = {3, 1, 4, 1, 5, 9};
    malloc_fail_countdown = 0;
    zenit_result_t r = zenit_sort_stable(arr, 6, sizeof(int), cmp_int);
    malloc_fail_countdown = -1;
    if (r.error != ZENIT_ERROR_ALLOC) {
        fprintf(stderr, "FAIL: expected ZENIT_ERROR_ALLOC, got %d\n", r.error);
        return 1;
    }
    printf("PASS: stable_sort returns ZENIT_ERROR_ALLOC on malloc failure\n");
    return 0;
}

int main(void) {
    int failures = 0;
    failures |= test_stable_sort_malloc_fail();
    return failures;
}
