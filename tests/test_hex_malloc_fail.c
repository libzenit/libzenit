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

#include <libzenit/hex.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int failed = 0;

    /* encode: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *enc = zenit_hex_encode((const unsigned char *)"hello", 5);
        malloc_fail_countdown = -1;
        if (enc != NULL) {
            fprintf(stderr, "FAIL: hex_encode alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: hex_encode alloc fail\n");
        }
        free(enc);
    }

    /* decode: first alloc fails */
    {
        malloc_fail_countdown = 0;
        size_t len;
        unsigned char *dec = zenit_hex_decode("68656c6c6f", &len);
        malloc_fail_countdown = -1;
        if (dec != NULL) {
            fprintf(stderr, "FAIL: hex_decode alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: hex_decode alloc fail\n");
        }
        free(dec);
    }

    /* encode_with_allocator: first alloc fails */
    {
        malloc_fail_countdown = 0;
        char *enc = zenit_hex_encode_with_allocator((const unsigned char *)"hello", 5, ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (enc != NULL) {
            fprintf(stderr, "FAIL: hex_encode_with_allocator alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: hex_encode_with_allocator alloc fail\n");
        }
        free(enc);
    }

    /* decode_with_allocator: first alloc fails */
    {
        malloc_fail_countdown = 0;
        size_t len;
        unsigned char *dec = zenit_hex_decode_with_allocator("68656c6c6f", &len, ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (dec != NULL) {
            fprintf(stderr, "FAIL: hex_decode_with_allocator alloc fail should return NULL\n");
            failed++;
        } else {
            printf("PASS: hex_decode_with_allocator alloc fail\n");
        }
        free(dec);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
