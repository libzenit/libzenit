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
#include <libzenit/csv.h>
#include <string.h>
#include <stdlib.h>

static int test_simple(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("a,b,c", ',', &rec);
    ASSERT(r.error == ZENIT_OK, "csv parse simple");
    ASSERT(rec.count == 3, "csv count");
    ASSERT(strcmp(rec.fields[0], "a") == 0, "field 0");
    ASSERT(strcmp(rec.fields[1], "b") == 0, "field 1");
    ASSERT(strcmp(rec.fields[2], "c") == 0, "field 2");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_quoted(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("\"hello, world\",\"foo\"\"bar\"", ',', &rec);
    ASSERT(r.error == ZENIT_OK, "csv parse quoted");
    ASSERT(rec.count == 2, "quoted count");
    ASSERT(strcmp(rec.fields[0], "hello, world") == 0, "quoted field 0");
    ASSERT(strcmp(rec.fields[1], "foo\"bar") == 0, "quoted field 1");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_empty_fields(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record(",a,,b,", ',', &rec);
    ASSERT(r.error == ZENIT_OK, "csv parse empty");
    ASSERT(rec.count == 5, "empty count");
    ASSERT(strcmp(rec.fields[0], "") == 0, "empty field 0");
    ASSERT(strcmp(rec.fields[3], "b") == 0, "empty field 3");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_serialise_simple(void) {
    zenit_csv_record_t rec;
    rec.count = 3;
    rec.fields = malloc(3 * sizeof(char *));
    rec.fields[0] = strdup("a");
    rec.fields[1] = strdup("b");
    rec.fields[2] = strdup("c");
    char *out = NULL;
    zenit_result_t r = zenit_csv_serialise_record(&rec, ',', &out);
    ASSERT(r.error == ZENIT_OK, "csv serialise");
    ASSERT(strcmp(out, "a,b,c") == 0, "csv serialise result");
    free(out);
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_serialise_quoting(void) {
    zenit_csv_record_t rec;
    rec.count = 2;
    rec.fields = malloc(2 * sizeof(char *));
    rec.fields[0] = strdup("hello, world");
    rec.fields[1] = strdup("foo\"bar");
    char *out = NULL;
    zenit_result_t r = zenit_csv_serialise_record(&rec, ',', &out);
    ASSERT(r.error == ZENIT_OK, "csv serialise quoting");
    ASSERT(strcmp(out, "\"hello, world\",\"foo\"\"bar\"") == 0, "csv serialise quoting result");
    free(out);
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_round_trip(void) {
    zenit_csv_record_t rec;
    zenit_csv_parse_record("a,\"b,c\",\"hello\"\"world\",,d", ',', &rec);
    char *out = NULL;
    zenit_csv_serialise_record(&rec, ',', &out);
    zenit_csv_record_t rec2;
    zenit_csv_parse_record(out, ',', &rec2);
    ASSERT(rec2.count == rec.count, "csv round-trip count");
    for (size_t i = 0; i < rec.count; i++) {
        ASSERT(strcmp(rec.fields[i], rec2.fields[i]) == 0, "csv round-trip field");
    }
    free(out);
    zenit_csv_record_destroy(&rec);
    zenit_csv_record_destroy(&rec2);
    PASS();
    return 0;
}

static int test_null_params(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record(NULL, ',', &rec);
    ASSERT(r.error == ZENIT_ERROR_NULL, "csv parse NULL line");
    r = zenit_csv_parse_record("a,b", ',', NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "csv parse NULL out");
    r = zenit_csv_serialise_record(NULL, ',', NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "csv serialise NULL");
    zenit_csv_record_destroy(NULL);
    PASS();
    return 0;
}

static int test_single_field(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("hello", ',', &rec);
    ASSERT(r.error == ZENIT_OK && rec.count == 1, "csv single");
    ASSERT(strcmp(rec.fields[0], "hello") == 0, "csv single field");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_tab_delimiter(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("a\tb\tc", '\t', &rec);
    ASSERT(r.error == ZENIT_OK && rec.count == 3, "csv tab");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_empty_input(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("", ',', &rec);
    ASSERT(r.error == ZENIT_OK && rec.count == 1, "csv empty input");
    ASSERT(rec.fields[0][0] == '\0', "csv empty field");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_quoted_trailing(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("\"hello\",world", ',', &rec);
    ASSERT(r.error == ZENIT_OK && rec.count == 2, "csv quoted trailing");
    ASSERT(strcmp(rec.fields[0], "hello") == 0, "qt field 0");
    ASSERT(strcmp(rec.fields[1], "world") == 0, "qt field 1");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_escaped_quote(void) {
    zenit_csv_record_t rec;
    zenit_result_t r = zenit_csv_parse_record("\"say \"\"hello\"\"\"", ',', &rec);
    ASSERT(r.error == ZENIT_OK && rec.count == 1, "csv escaped quote");
    ASSERT(strcmp(rec.fields[0], "say \"hello\"") == 0, "escaped content");
    zenit_csv_record_destroy(&rec);
    PASS();
    return 0;
}

static int test_serialise_empty(void) {
    zenit_csv_record_t rec;
    rec.count = 1;
    rec.fields = malloc(sizeof(char *));
    rec.fields[0] = strdup("");
    char *out = NULL;
    zenit_result_t r = zenit_csv_serialise_record(&rec, ',', &out);
    ASSERT(r.error == ZENIT_OK && strcmp(out, "\"\"") == 0, "csv serialise empty");
    free(out);
    free(rec.fields[0]);
    free(rec.fields);
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_simple,
        test_quoted,
        test_empty_fields,
        test_serialise_simple,
        test_serialise_quoting,
        test_round_trip,
        test_null_params,
        test_single_field,
        test_tab_delimiter,
        test_empty_input,
        test_quoted_trailing,
        test_escaped_quote,
        test_serialise_empty,
    };
    const char *names[] = {
        "simple",
        "quoted",
        "empty_fields",
        "serialise_simple",
        "serialise_quoting",
        "round_trip",
        "null_params",
        "single_field",
        "tab_delimiter",
        "empty_input",
        "quoted_trailing",
        "escaped_quote",
        "serialise_empty",
    };
    ZENIT_RUN_TESTS("csv", tests, names);
    return 0;
}
