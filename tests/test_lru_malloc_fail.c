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

#include <libzenit/lru.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_create_family(const char *label, int with_allocator) {
    int failures = 0;
    for (int n = 1; n <= 7; n++) {
        malloc_fail_countdown = n;
        zenit_lru_t *c;
        if (with_allocator) {
            c = zenit_lru_create_with_allocator(sizeof(int), sizeof(int), 4, ZENIT_ALLOCATOR_DEFAULT);
        } else {
            c = zenit_lru_create(sizeof(int), sizeof(int), 4);
        }
        malloc_fail_countdown = -1;
        if (c != NULL) {
            zenit_lru_destroy(c);
            if (n != 7) {
                fprintf(stderr, "FAIL: %s should be NULL at countdown %d\n", label, n);
                failures++;
            }
        }
    }
    return failures;
}

int main(void) {
    int total = 0;

    total += test_create_family("create", 0);
    total += test_create_family("create_with_allocator", 1);

    if (total > 0) {
        fprintf(stderr, "%d failure(s)\n", total);
        return 1;
    }
    printf("PASS: lru_malloc_fail\n");
    return 0;
}
