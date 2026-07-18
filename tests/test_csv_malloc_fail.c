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
    int failed = 0;

    /* Test fields array alloc failure */
    {
        zenit_csv_record_t rec;
        malloc_fail_countdown = 0;
        zenit_result_t r = zenit_csv_parse_record("a,b,c", ',', &rec);
        malloc_fail_countdown = -1;
        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: csv parse alloc fail fields\n");
            failed++;
        }
        zenit_csv_record_destroy(&rec);
    }

    /* Test first field buffer alloc failure */
    {
        zenit_csv_record_t rec;
        malloc_fail_countdown = 2;
        zenit_result_t r = zenit_csv_parse_record("a,b,c", ',', &rec);
        malloc_fail_countdown = -1;
        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: csv parse alloc fail buffer\n");
            failed++;
        }
        zenit_csv_record_destroy(&rec);
    }

    /* Test serialise alloc failure */
    {
        zenit_csv_record_t rec;
        rec.count = 1;
        rec.fields = calloc(1, sizeof(char *));
        rec.fields[0] = strdup("hello");

        malloc_fail_countdown = 0;
        char *out = (char *)0x123;
        zenit_result_t r = zenit_csv_serialise_record(&rec, ',', &out);
        malloc_fail_countdown = -1;
        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: csv serialise alloc fail\n");
            failed++;
        }

        free(rec.fields[0]);
        free(rec.fields);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }

    printf("PASS: csv malloc fail\n");
    return 0;
}
