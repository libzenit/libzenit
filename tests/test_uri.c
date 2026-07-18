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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); return 1; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); } } while (0)

static int test_encode_unreserved(void) {
    /* Only unreserved chars — should remain unchanged */
    char *enc = zenit_uri_encode("abc123-._~");
    ASSERT(enc != NULL, "encode unreserved returned NULL");
    ASSERT(strcmp(enc, "abc123-._~") == 0, "encode unreserved failed");
    free(enc);
    return 0;
}

static int test_encode_reserved(void) {
    char *enc = zenit_uri_encode("hello world");
    ASSERT(enc != NULL, "encode space returned NULL");
    ASSERT(strcmp(enc, "hello%20world") == 0, "encode space failed");
    free(enc);

    enc = zenit_uri_encode("a/b?c=d");
    ASSERT(enc != NULL, "encode special returned NULL");
    ASSERT(strcmp(enc, "a%2Fb%3Fc%3Dd") == 0, "encode special chars failed");
    free(enc);
    return 0;
}

static int test_encode_empty(void) {
    char *enc = zenit_uri_encode("");
    ASSERT(enc != NULL, "encode empty returned NULL");
    ASSERT(strcmp(enc, "") == 0, "encode empty failed");
    free(enc);
    return 0;
}

static int test_encode_null(void) {
    ASSERT(zenit_uri_encode(NULL) == NULL, "encode NULL should return NULL");
    return 0;
}

static int test_decode_basic(void) {
    char *dec = zenit_uri_decode("hello%20world");
    ASSERT(dec != NULL, "decode returned NULL");
    ASSERT(strcmp(dec, "hello world") == 0, "decode basic failed");
    free(dec);
    return 0;
}

static int test_decode_plus(void) {
    char *dec = zenit_uri_decode("a+b+c");
    ASSERT(dec != NULL, "decode plus returned NULL");
    ASSERT(strcmp(dec, "a b c") == 0, "decode plus failed");
    free(dec);
    return 0;
}

static int test_decode_unreserved(void) {
    char *dec = zenit_uri_decode("abc123-._~");
    ASSERT(dec != NULL, "decode unreserved returned NULL");
    ASSERT(strcmp(dec, "abc123-._~") == 0, "decode unreserved failed");
    free(dec);
    return 0;
}

static int test_decode_empty(void) {
    char *dec = zenit_uri_decode("");
    ASSERT(dec != NULL, "decode empty returned NULL");
    ASSERT(strcmp(dec, "") == 0, "decode empty failed");
    free(dec);
    return 0;
}

static int test_decode_invalid(void) {
    const char *dec = zenit_uri_decode("%2");
    ASSERT(dec == NULL, "decode incomplete %%XX should return NULL");
    dec = zenit_uri_decode("%2G");
    ASSERT(dec == NULL, "decode invalid hex should return NULL");
    dec = zenit_uri_decode("a%");
    ASSERT(dec == NULL, "decode trailing %% should return NULL");
    return 0;
}

static int test_decode_null(void) {
    ASSERT(zenit_uri_decode(NULL) == NULL, "decode NULL should return NULL");
    return 0;
}

static int test_roundtrip(void) {
    const char *original = "a b c d e f g ?=hello/world:test";
    char *enc = zenit_uri_encode(original);
    ASSERT(enc != NULL, "roundtrip encode returned NULL");
    char *dec = zenit_uri_decode(enc);
    ASSERT(dec != NULL, "roundtrip decode returned NULL");
    ASSERT(strcmp(dec, original) == 0, "roundtrip mismatch");
    free(enc);
    free(dec);
    return 0;
}

int main(void) {
    int failed = 0;

    printf("=== uri ===\n");

    printf("  encode_unreserved ... "); if (test_encode_unreserved()) failed++;
    else printf("PASS\n");

    printf("  encode_reserved ... "); if (test_encode_reserved()) failed++;
    else printf("PASS\n");

    printf("  encode_empty ... "); if (test_encode_empty()) failed++;
    else printf("PASS\n");

    printf("  encode_null ... "); if (test_encode_null()) failed++;
    else printf("PASS\n");

    printf("  decode_basic ... "); if (test_decode_basic()) failed++;
    else printf("PASS\n");

    printf("  decode_plus ... "); if (test_decode_plus()) failed++;
    else printf("PASS\n");

    printf("  decode_unreserved ... "); if (test_decode_unreserved()) failed++;
    else printf("PASS\n");

    printf("  decode_empty ... "); if (test_decode_empty()) failed++;
    else printf("PASS\n");

    printf("  decode_invalid ... "); if (test_decode_invalid()) failed++;
    else printf("PASS\n");

    printf("  decode_null ... "); if (test_decode_null()) failed++;
    else printf("PASS\n");

    printf("  roundtrip ... "); if (test_roundtrip()) failed++;
    else printf("PASS\n");

    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
