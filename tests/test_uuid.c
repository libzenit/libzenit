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

#include "test_runner.h"
#include <libzenit/uuid.h>
#include <string.h>

static int test_generate_version_variant(void) {
    zenit_uuid_t uuid;
    zenit_result_t r = zenit_uuid_generate(&uuid);
    ASSERT(r.error == ZENIT_OK, "uuid_generate returned error");
    ASSERT((uuid.bytes[6] & 0xf0) == 0x40, "uuid version not 4");
    ASSERT((uuid.bytes[8] & 0xc0) == 0x80, "uuid variant not 10xx");
    int all_zero = 1;
    for (size_t i = 0; i < 16; i++) {
        if (uuid.bytes[i] != 0) { all_zero = 0; break; }
    }
    ASSERT(!all_zero, "uuid is all zeros");
    PASS();
    return 0;
}

static int test_format(void) {
    zenit_uuid_t uuid;
    zenit_uuid_generate(&uuid);
    char str[ZENIT_UUID_STR_LEN];
    zenit_uuid_format(&uuid, str);
    ASSERT(strlen(str) == 36, "uuid format length != 36");
    ASSERT(str[8] == '-' && str[13] == '-' && str[18] == '-' && str[23] == '-', "uuid format dash positions wrong");
    PASS();
    return 0;
}

static int test_round_trip(void) {
    zenit_uuid_t orig;
    zenit_uuid_generate(&orig);
    char str[ZENIT_UUID_STR_LEN];
    zenit_uuid_format(&orig, str);
    zenit_uuid_t parsed;
    zenit_result_t r = zenit_uuid_parse(str, &parsed);
    ASSERT(r.error == ZENIT_OK, "uuid parse returned error");
    ASSERT(zenit_uuid_equal(&orig, &parsed), "uuid round-trip mismatch");
    PASS();
    return 0;
}

static int test_parse_known(void) {
    zenit_uuid_t uuid;
    zenit_result_t r = zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &uuid);
    ASSERT(r.error == ZENIT_OK, "uuid parse known string");
    char str[ZENIT_UUID_STR_LEN];
    zenit_uuid_format(&uuid, str);
    ASSERT(strcmp(str, "550e8400-e29b-41d4-a716-446655440000") == 0, "uuid parse known round-trip");
    PASS();
    return 0;
}

static int test_parse_uppercase(void) {
    zenit_uuid_t uuid;
    zenit_result_t r = zenit_uuid_parse("550E8400-E29B-41D4-A716-446655440000", &uuid);
    ASSERT(r.error == ZENIT_OK, "uuid parse uppercase");
    char str[ZENIT_UUID_STR_LEN];
    zenit_uuid_format(&uuid, str);
    ASSERT(strcmp(str, "550e8400-e29b-41d4-a716-446655440000") == 0, "uuid parse uppercase round-trip");
    PASS();
    return 0;
}

static int test_equal(void) {
    zenit_uuid_t a;
    zenit_uuid_t b;
    zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &a);
    zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &b);
    ASSERT(zenit_uuid_equal(&a, &b), "uuid equal should match");
    PASS();
    return 0;
}

static int test_not_equal(void) {
    zenit_uuid_t a;
    zenit_uuid_t b;
    zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &a);
    zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440001", &b);
    ASSERT(!zenit_uuid_equal(&a, &b), "uuid not-equal should not match");
    PASS();
    return 0;
}

static int test_null_params(void) {
    zenit_result_t r = zenit_uuid_generate(NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "uuid_generate(NULL)");
    r = zenit_uuid_format(NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "uuid_format(NULL)");
    r = zenit_uuid_parse(NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "uuid_parse(NULL)");
    ASSERT(zenit_uuid_equal(NULL, NULL) == 0, "uuid_equal(NULL)");
    PASS();
    return 0;
}

static int test_invalid(void) {
    zenit_uuid_t u;
    zenit_result_t r = zenit_uuid_parse("short", &u);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "uuid parse short");
    r = zenit_uuid_parse("zzzzzzzz-zzzz-zzzz-zzzz-zzzzzzzzzzzz", &u);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "uuid parse invalid hex");
    r = zenit_uuid_parse("550e8400e29b41d4a716446655440000", &u);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "uuid parse no dashes");
    /* 36-char string with wrong dash positions */
    r = zenit_uuid_parse("550e8400-e29b41d4-a716-446655440000-", &u);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "uuid parse wrong dash pos");
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_generate_version_variant,
        test_format,
        test_round_trip,
        test_parse_known,
        test_parse_uppercase,
        test_equal,
        test_not_equal,
        test_null_params,
        test_invalid,
    };
    const char *names[] = {
        "generate_version_variant",
        "format",
        "round_trip",
        "parse_known",
        "parse_uppercase",
        "equal",
        "not_equal",
        "null_params",
        "invalid",
    };
    ZENIT_RUN_TESTS("uuid", tests, names);
    return 0;
}
