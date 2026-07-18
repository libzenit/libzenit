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
#include <libzenit/logger.h>
#include <stdio.h>

int main(void) {
    /* Test create failure */
    {
        malloc_fail_countdown = 0;
        zenit_logger_t *log = zenit_logger_create(NULL, NULL);
        malloc_fail_countdown = -1;

        if (log != NULL) {
            fprintf(stderr, "FAIL: logger create should fail on malloc fail\n");
            zenit_logger_destroy(log);
            return 1;
        }
    }

    printf("PASS: logger malloc fail\n");
    return 0;
}
