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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_malloc_fail.h"

typedef struct {
    char data[128];
    int key;
} large_elem_t;

static int cmp_large(const void *a, const void *b) {
    int ka = ((const large_elem_t *)a)->key;
    int kb = ((const large_elem_t *)b)->key;
    if (ka < kb) return -1;
    if (ka > kb) return 1;
    return 0;
}

int main(void) {
    large_elem_t arr[4];
    arr[0].key = 3;
    arr[1].key = 1;
    arr[2].key = 4;
    arr[3].key = 2;

    /* Force malloc to fail — tests the silent return in swap_elements */
    malloc_fail_countdown = 0;
    zenit_sort_quick(arr, 4, sizeof(large_elem_t), cmp_large);
    malloc_fail_countdown = -1;

    printf("PASS: sort with malloc failure does not crash\n");
    return 0;
}
