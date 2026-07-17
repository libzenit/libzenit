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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); return 1; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); } } while (0)

static int test_encode_basic(void) {
    const unsigned char data[] = "hello";
    char *enc = zenit_hex_encode(data, 5);
    ASSERT(enc != NULL, "encode returned NULL");
    ASSERT(strcmp(enc, "68656c6c6f") == 0, "encode 'hello' failed");
    free(enc);
    return 0;
}

static int test_encode_empty(void) {
    char *enc = zenit_hex_encode((const unsigned char *)"", 0);
    ASSERT(enc != NULL, "encode empty returned NULL");
    ASSERT(strcmp(enc, "") == 0, "encode empty failed");
    free(enc);
    return 0;
}

static int test_encode_binary(void) {
    unsigned char data[256];
    for (int i = 0; i < 256; i++) data[i] = (unsigned char)i;

    char *enc = zenit_hex_encode(data, 256);
    ASSERT(enc != NULL, "encode 256 bytes returned NULL");
    /* Check that all hex chars are lowercase */
    for (size_t i = 0; enc[i] != '\0'; i++) {
        if (enc[i] >= 'A' && enc[i] <= 'F') {
            FAIL("encode produced uppercase hex");
        }
    }
    /* Round-trip */
    size_t dec_len;
    unsigned char *dec = zenit_hex_decode(enc, &dec_len);
    ASSERT(dec != NULL, "decode 256 bytes returned NULL");
    ASSERT(dec_len == 256, "decode length mismatch");
    ASSERT(memcmp(dec, data, 256) == 0, "decode content mismatch");
    free(dec);
    free(enc);
    return 0;
}

static int test_decode_basic(void) {
    size_t len;
    unsigned char *dec = zenit_hex_decode("68656c6c6f", &len);
    ASSERT(dec != NULL, "decode 'hello' returned NULL");
    ASSERT(len == 5, "decode length mismatch");
    ASSERT(memcmp(dec, "hello", 5) == 0, "decode content mismatch");
    free(dec);
    return 0;
}

static int test_decode_odd_length(void) {
    /* "abc" -> "abcd" equivalent to "0abc" */
    size_t len;
    unsigned char *dec = zenit_hex_decode("abc", &len);
    ASSERT(dec != NULL, "decode odd length returned NULL");
    ASSERT(len == 2, "decode odd length should produce 2 bytes");
    ASSERT(dec[0] == 0x0A && dec[1] == 0xBC, "decode odd length content");
    free(dec);
    return 0;
}

static int test_decode_uppercase(void) {
    size_t len;
    unsigned char *dec = zenit_hex_decode("68656C6C6F", &len);
    ASSERT(dec != NULL, "decode uppercase returned NULL");
    ASSERT(len == 5, "decode uppercase length mismatch");
    ASSERT(memcmp(dec, "hello", 5) == 0, "decode uppercase content mismatch");
    free(dec);
    return 0;
}

static int test_decode_invalid(void) {
    size_t len;
    unsigned char *dec = zenit_hex_decode("6g", &len);
    ASSERT(dec == NULL, "decode invalid hex should return NULL");
    /* Invalid first char in odd-length input */
    dec = zenit_hex_decode("xyz", &len);
    ASSERT(dec == NULL, "decode invalid odd first char should return NULL");
    return 0;
}

static int test_encode_len(void) {
    ASSERT(zenit_hex_encode_len(0) == 1, "encode_len 0");
    ASSERT(zenit_hex_encode_len(1) == 3, "encode_len 1");
    ASSERT(zenit_hex_encode_len(5) == 11, "encode_len 5");
    return 0;
}

static int test_decode_len(void) {
    ASSERT(zenit_hex_decode_len(NULL) == 0, "decode_len NULL");
    ASSERT(zenit_hex_decode_len("") == 0, "decode_len empty");
    ASSERT(zenit_hex_decode_len("a") == 1, "decode_len odd");
    ASSERT(zenit_hex_decode_len("ab") == 1, "decode_len even");
    return 0;
}

static int test_decode_empty(void) {
    size_t len;
    unsigned char *dec = zenit_hex_decode("", &len);
    ASSERT(dec != NULL, "decode empty returned NULL");
    ASSERT(len == 0, "decode empty length should be 0");
    free(dec);
    return 0;
}

static int test_decode_null_params(void) {
    size_t len;
    ASSERT(zenit_hex_decode(NULL, &len) == NULL, "decode NULL input");
    ASSERT(zenit_hex_decode("68656c6c6f", NULL) == NULL, "decode NULL out_len");
    return 0;
}

static int test_encode_null_data(void) {
    char *enc = zenit_hex_encode(NULL, 10);
    ASSERT(enc == NULL, "encode NULL data should return NULL");
    return 0;
}

int main(void) {
    int failed = 0;

    printf("=== hex ===\n");

    printf("  encode_basic ... "); if (test_encode_basic()) failed++;
    else printf("PASS\n");

    printf("  encode_empty ... "); if (test_encode_empty()) failed++;
    else printf("PASS\n");

    printf("  encode_binary ... "); if (test_encode_binary()) failed++;
    else printf("PASS\n");

    printf("  decode_basic ... "); if (test_decode_basic()) failed++;
    else printf("PASS\n");

    printf("  decode_odd_length ... "); if (test_decode_odd_length()) failed++;
    else printf("PASS\n");

    printf("  decode_uppercase ... "); if (test_decode_uppercase()) failed++;
    else printf("PASS\n");

    printf("  decode_invalid ... "); if (test_decode_invalid()) failed++;
    else printf("PASS\n");

    printf("  decode_empty ... "); if (test_decode_empty()) failed++;
    else printf("PASS\n");

    printf("  decode_null_params ... "); if (test_decode_null_params()) failed++;
    else printf("PASS\n");

    printf("  encode_null_data ... "); if (test_encode_null_data()) failed++;
    else printf("PASS\n");

    printf("  encode_len ... "); if (test_encode_len()) failed++;
    else printf("PASS\n");

    printf("  decode_len ... "); if (test_decode_len()) failed++;
    else printf("PASS\n");

    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
