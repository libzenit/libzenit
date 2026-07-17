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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); return 1; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); } } while (0)

static int test_encode_basic(void) {
    const unsigned char data[] = "hello";
    char *enc = zenit_base64_encode(data, 5);
    ASSERT(enc != NULL, "encode returned NULL");
    ASSERT(strcmp(enc, "aGVsbG8=") == 0, "encode 'hello' failed");
    free(enc);
    return 0;
}

static int test_encode_padding(void) {
    /* 1 byte -> 2 padding chars */
    {
        const unsigned char data[] = "a";
        char *enc = zenit_base64_encode(data, 1);
        ASSERT(enc != NULL, "encode 1 byte returned NULL");
        ASSERT(strcmp(enc, "YQ==") == 0, "encode 'a' failed");
        free(enc);
    }
    /* 2 bytes -> 1 padding char */
    {
        const unsigned char data[] = "ab";
        char *enc = zenit_base64_encode(data, 2);
        ASSERT(enc != NULL, "encode 2 bytes returned NULL");
        ASSERT(strcmp(enc, "YWI=") == 0, "encode 'ab' failed");
        free(enc);
    }
    /* 3 bytes -> no padding */
    {
        const unsigned char data[] = "abc";
        char *enc = zenit_base64_encode(data, 3);
        ASSERT(enc != NULL, "encode 3 bytes returned NULL");
        ASSERT(strcmp(enc, "YWJj") == 0, "encode 'abc' failed");
        free(enc);
    }
    return 0;
}

static int test_encode_empty(void) {
    char *enc = zenit_base64_encode((const unsigned char *)"", 0);
    ASSERT(enc != NULL, "encode empty returned NULL");
    ASSERT(strcmp(enc, "") == 0, "encode empty failed");
    free(enc);
    return 0;
}

static int test_encode_binary(void) {
    /* All byte values 0x00-0xFF */
    unsigned char data[256];
    for (int i = 0; i < 256; i++) data[i] = (unsigned char)i;

    char *enc = zenit_base64_encode(data, 256);
    ASSERT(enc != NULL, "encode 256 bytes returned NULL");

    size_t dec_len;
    unsigned char *dec = zenit_base64_decode(enc, &dec_len);
    ASSERT(dec != NULL, "decode with out_len returned NULL");
    ASSERT(dec_len == 256, "decode length mismatch");
    ASSERT(memcmp(dec, data, 256) == 0, "decode 256 bytes content mismatch");
    free(dec);
    free(enc);
    return 0;
}

static int test_decode_basic(void) {
    size_t len;
    unsigned char *dec = zenit_base64_decode("aGVsbG8=", &len);
    ASSERT(dec != NULL, "decode 'hello' returned NULL");
    ASSERT(len == 5, "decode 'hello' length mismatch");
    ASSERT(memcmp(dec, "hello", 5) == 0, "decode 'hello' content mismatch");
    free(dec);
    return 0;
}

static int test_decode_no_padding(void) {
    size_t len;
    unsigned char *dec = zenit_base64_decode("YWJj", &len);
    ASSERT(dec != NULL, "decode 'abc' returned NULL");
    ASSERT(len == 3, "decode 'abc' length mismatch");
    ASSERT(memcmp(dec, "abc", 3) == 0, "decode 'abc' content mismatch");
    free(dec);
    return 0;
}

static int test_decode_invalid(void) {
    size_t len;
    /* Invalid character in d0/d1 */
    unsigned char *dec = zenit_base64_decode("aGVs!G8=", &len);
    ASSERT(dec == NULL, "decode invalid d0/d1 should return NULL");
    /* Invalid character in d2 (not padding) */
    dec = zenit_base64_decode("AA!!", &len);
    ASSERT(dec == NULL, "decode invalid d2 should return NULL");
    /* Invalid character in d3 (not padding) */
    dec = zenit_base64_decode("AAA!", &len);
    ASSERT(dec == NULL, "decode invalid d3 should return NULL");
    /* Short input */
    dec = zenit_base64_decode("abc", &len);
    ASSERT(dec == NULL, "decode short input should return NULL");
    /* Wrong length (not multiple of 4) */
    dec = zenit_base64_decode("abcde", &len);
    ASSERT(dec == NULL, "decode wrong length should return NULL");
    return 0;
}

static int test_decode_null_params(void) {
    size_t len;
    ASSERT(zenit_base64_decode(NULL, &len) == NULL, "decode NULL input");
    {
        unsigned char *d = zenit_base64_decode("aGVsbG8=", NULL);
        ASSERT(d != NULL, "decode NULL out_len should still succeed");
        free(d);
    }
    return 0;
}

static int test_encode_null_data(void) {
    const char *enc = zenit_base64_encode(NULL, 10);
    ASSERT(enc == NULL, "encode NULL data should return NULL");
    return 0;
}

int main(void) {
    int failed = 0;

    printf("=== base64 ===\n");

    printf("  encode_basic ... "); if (test_encode_basic()) failed++;
    else printf("PASS\n");

    printf("  encode_padding ... "); if (test_encode_padding()) failed++;
    else printf("PASS\n");

    printf("  encode_empty ... "); if (test_encode_empty()) failed++;
    else printf("PASS\n");

    printf("  encode_binary ... "); if (test_encode_binary()) failed++;
    else printf("PASS\n");

    printf("  decode_basic ... "); if (test_decode_basic()) failed++;
    else printf("PASS\n");

    printf("  decode_no_padding ... "); if (test_decode_no_padding()) failed++;
    else printf("PASS\n");

    printf("  decode_invalid ... "); if (test_decode_invalid()) failed++;
    else printf("PASS\n");

    printf("  decode_null_params ... "); if (test_decode_null_params()) failed++;
    else printf("PASS\n");

    printf("  encode_null_data ... "); if (test_encode_null_data()) failed++;
    else printf("PASS\n");

    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
