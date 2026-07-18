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

#include <libzenit/uri.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int failed = 0;

    /* encode: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *enc = zenit_uri_encode("hello world");
        malloc_fail_countdown = -1;
        if (enc != NULL) {
            fprintf(stderr, "FAIL: uri_encode alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: uri_encode alloc fail\n");
        }
        free(enc);
    }

    /* decode: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *dec = zenit_uri_decode("hello%20world");
        malloc_fail_countdown = -1;
        if (dec != NULL) {
            fprintf(stderr, "FAIL: uri_decode alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: uri_decode alloc fail\n");
        }
        free(dec);
    }

    /* encode_with_allocator: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *enc = zenit_uri_encode_with_allocator("hello world", ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (enc != NULL) {
            fprintf(stderr, "FAIL: uri_encode_with_allocator alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: uri_encode_with_allocator alloc fail\n");
        }
        free(enc);
    }

    /* decode_with_allocator: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *dec = zenit_uri_decode_with_allocator("hello%20world", ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (dec != NULL) {
            fprintf(stderr, "FAIL: uri_decode_with_allocator alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: uri_decode_with_allocator alloc fail\n");
        }
        free(dec);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
