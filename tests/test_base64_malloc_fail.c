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

#include <libzenit/base64.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int failed = 0;

    /* encode: first malloc fails */
    {
        malloc_fail_countdown = 0;
        char *enc = zenit_base64_encode((const unsigned char *)"hello", 5);
        malloc_fail_countdown = -1;
        if (enc != NULL) {
            fprintf(stderr, "FAIL: encode with malloc_fail_countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: encode malloc fail\n");
        }
        free(enc);
    }

    /* decode: first malloc fails */
    {
        malloc_fail_countdown = 0;
        size_t len;
        unsigned char *dec = zenit_base64_decode("aGVsbG8=", &len);
        malloc_fail_countdown = -1;
        if (dec != NULL) {
            fprintf(stderr, "FAIL: decode with malloc_fail_countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: decode malloc fail\n");
        }
        free(dec);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
