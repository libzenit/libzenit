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
#include <libzenit/csv.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Test create failure (fields array alloc) */
    {
        zenit_csv_record_t rec;
        malloc_fail_countdown = 0;
        zenit_result_t r = zenit_csv_parse_record("a,b,c", ',', &rec);
        malloc_fail_countdown = -1;

        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: csv parse should fail on alloc\n");
            return 1;
        }
        /* Make sure destroy is safe even after partial init */
        zenit_csv_record_destroy(&rec);
    }

    printf("PASS: csv malloc fail\n");
    return 0;
}
