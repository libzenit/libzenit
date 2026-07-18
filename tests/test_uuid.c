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

#include <libzenit/uuid.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Test 1: Generate a UUID and verify it has correct version/variant bits */
    {
        zenit_uuid_t uuid;
        zenit_result_t r = zenit_uuid_generate(&uuid);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: uuid_generate returned error\n");
            return 1;
        }
        /* Version 4 → bytes[6] top nibble should be 4 */
        if ((uuid.bytes[6] & 0xf0) != 0x40) {
            fprintf(stderr, "FAIL: uuid version not 4 (got 0x%02x)\n", uuid.bytes[6]);
            return 1;
        }
        /* Variant → bytes[8] top 2 bits should be 10xx xxxx */
        if ((uuid.bytes[8] & 0xc0) != 0x80) {
            fprintf(stderr, "FAIL: uuid variant not 10xx (got 0x%02x)\n", uuid.bytes[8]);
            return 1;
        }
        /* Not all zeros */
        int all_zero = 1;
        for (size_t i = 0; i < 16; i++) {
            if (uuid.bytes[i] != 0) { all_zero = 0; break; }
        }
        if (all_zero) {
            fprintf(stderr, "FAIL: uuid is all zeros\n");
            return 1;
        }
    }
    printf("PASS: uuid generate version/variant\n");

    /* Test 2: Generate and format */
    {
        zenit_uuid_t uuid;
        zenit_uuid_generate(&uuid);
        char str[ZENIT_UUID_STR_LEN];
        zenit_uuid_format(&uuid, str);
        if (strlen(str) != 36) {
            fprintf(stderr, "FAIL: uuid format length %zu != 36\n", strlen(str));
            return 1;
        }
        /* Verify format matches pattern: 8-4-4-4-12 */
        if (str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-') {
            fprintf(stderr, "FAIL: uuid format dash positions wrong\n");
            return 1;
        }
        /* All hex digits (not dash) */
        for (size_t i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) continue;
            if (!((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f'))) {
                fprintf(stderr, "FAIL: uuid format char '%c' at %zu not hex\n", str[i], i);
                return 1;
            }
        }
    }
    printf("PASS: uuid format\n");

    /* Test 3: Round-trip format then parse */
    {
        zenit_uuid_t orig;
        zenit_uuid_generate(&orig);
        char str[ZENIT_UUID_STR_LEN];
        zenit_uuid_format(&orig, str);
        zenit_uuid_t parsed;
        zenit_result_t r = zenit_uuid_parse(str, &parsed);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: uuid parse returned error\n");
            return 1;
        }
        if (!zenit_uuid_equal(&orig, &parsed)) {
            fprintf(stderr, "FAIL: uuid round-trip mismatch\n");
            return 1;
        }
    }
    printf("PASS: uuid round-trip\n");

    /* Test 4: Parse known UUID string */
    {
        zenit_uuid_t uuid;
        zenit_result_t r = zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &uuid);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: uuid parse known string\n");
            return 1;
        }
        char str[ZENIT_UUID_STR_LEN];
        zenit_uuid_format(&uuid, str);
        if (strcmp(str, "550e8400-e29b-41d4-a716-446655440000") != 0) {
            fprintf(stderr, "FAIL: uuid parse known round-trip got '%s'\n", str);
            return 1;
        }
    }
    printf("PASS: uuid parse known string\n");

    /* Test 5: Parse uppercase hex */
    {
        zenit_uuid_t uuid;
        zenit_result_t r = zenit_uuid_parse("550E8400-E29B-41D4-A716-446655440000", &uuid);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: uuid parse uppercase\n");
            return 1;
        }
        char str[ZENIT_UUID_STR_LEN];
        zenit_uuid_format(&uuid, str);
        if (strcmp(str, "550e8400-e29b-41d4-a716-446655440000") != 0) {
            fprintf(stderr, "FAIL: uuid parse uppercase round-trip got '%s'\n", str);
            return 1;
        }
    }
    printf("PASS: uuid parse uppercase\n");

    /* Test 6: Equal UUIDs */
    {
        zenit_uuid_t a;
        zenit_uuid_t b;
        zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &a);
        zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &b);
        if (!zenit_uuid_equal(&a, &b)) {
            fprintf(stderr, "FAIL: uuid equal should match\n");
            return 1;
        }
    }
    printf("PASS: uuid equal\n");

    /* Test 7: Non-equal UUIDs */
    {
        zenit_uuid_t a;
        zenit_uuid_t b;
        zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &a);
        zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440001", &b);
        if (zenit_uuid_equal(&a, &b)) {
            fprintf(stderr, "FAIL: uuid not-equal should not match\n");
            return 1;
        }
    }
    printf("PASS: uuid not-equal\n");

    /* Test 8: NULL params */
    {
        zenit_result_t r = zenit_uuid_generate(NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: uuid_generate(NULL) should return NULL\n");
            return 1;
        }
        r = zenit_uuid_format(NULL, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: uuid_format(NULL,NULL) should return NULL\n");
            return 1;
        }
        r = zenit_uuid_parse(NULL, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: uuid_parse(NULL,NULL) should return NULL\n");
            return 1;
        }
        if (zenit_uuid_equal(NULL, NULL) != 0) {
            fprintf(stderr, "FAIL: uuid_equal(NULL,NULL) should return 0\n");
            return 1;
        }
    }
    printf("PASS: uuid NULL params\n");

    /* Test 9: Parse invalid strings */
    {
        zenit_uuid_t u;
        /* Wrong length */
        zenit_result_t r = zenit_uuid_parse("short", &u);
        if (r.error != ZENIT_ERROR_PARAM) {
            fprintf(stderr, "FAIL: uuid parse short string\n");
            return 1;
        }
        /* Invalid hex */
        r = zenit_uuid_parse("zzzzzzzz-zzzz-zzzz-zzzz-zzzzzzzzzzzz", &u);
        if (r.error != ZENIT_ERROR_PARAM) {
            fprintf(stderr, "FAIL: uuid parse invalid hex\n");
            return 1;
        }
        /* Missing dashes */
        r = zenit_uuid_parse("550e8400e29b41d4a716446655440000", &u);
        if (r.error != ZENIT_ERROR_PARAM) {
            fprintf(stderr, "FAIL: uuid parse no dashes\n");
            return 1;
        }
    }
    printf("PASS: uuid invalid parse\n");

    printf("PASS: uuid\n");
    return 0;
}
