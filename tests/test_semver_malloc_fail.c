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
#include <libzenit/semver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    /* Test format allocation failure */
    {
        zenit_semver_t v = { .major = 1, .minor = 0, .patch = 0 };
        char *s = (char *)0x123;

        malloc_fail_countdown = 0;
        zenit_result_t r = zenit_semver_format(&v, &s);
        malloc_fail_countdown = -1;

        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: semver format alloc fail should return ALLOC\n");
            return 1;
        }
    }

    printf("PASS: semver malloc fail\n");
    return 0;
}
