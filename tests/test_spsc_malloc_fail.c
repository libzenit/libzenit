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

#include "test_malloc_fail.h"
#include <libzenit/spsc.h>
#include <stdio.h>

int main(void) {
    /* Test create failure (first malloc for struct) */
    {
        malloc_fail_countdown = 0;
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 16);
        malloc_fail_countdown = -1;

        if (s != NULL) {
            fprintf(stderr, "FAIL: spsc create should fail on alloc\n");
            zenit_spsc_destroy(s);
            return 1;
        }
    }

    /* Test create failure (second malloc for buffer) */
    {
        malloc_fail_countdown = 1;
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 16);
        malloc_fail_countdown = -1;

        if (s != NULL) {
            fprintf(stderr, "FAIL: spsc create should fail on buffer alloc\n");
            zenit_spsc_destroy(s);
            return 1;
        }
    }

    printf("PASS: spsc malloc fail\n");
    return 0;
}
