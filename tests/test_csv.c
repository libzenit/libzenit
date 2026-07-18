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

#include <libzenit/csv.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Test 1: Parse simple CSV */
    {
        zenit_csv_record_t rec;
        zenit_result_t r = zenit_csv_parse_record("a,b,c", ',', &rec);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: csv parse simple\n");
            return 1;
        }
        if (rec.count != 3 || strcmp(rec.fields[0], "a") || strcmp(rec.fields[1], "b") || strcmp(rec.fields[2], "c")) {
            fprintf(stderr, "FAIL: csv parse fields mismatch\n");
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv parse simple\n");

    /* Test 2: Parse with quoted fields */
    {
        zenit_csv_record_t rec;
        zenit_result_t r = zenit_csv_parse_record("\"hello, world\",\"foo\"\"bar\"", ',', &rec);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: csv parse quoted\n");
            return 1;
        }
        if (rec.count != 2 ||
            strcmp(rec.fields[0], "hello, world") ||
            strcmp(rec.fields[1], "foo\"bar")) {
            fprintf(stderr, "FAIL: csv parse quoted fields: '%s' '%s'\n", rec.fields[0], rec.fields[1]);
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv parse quoted\n");

    /* Test 3: Parse empty fields */
    {
        zenit_csv_record_t rec;
        zenit_result_t r = zenit_csv_parse_record(",a,,b,", ',', &rec);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: csv parse empty\n");
            return 1;
        }
        if (rec.count != 5 ||
            strcmp(rec.fields[0], "") ||
            strcmp(rec.fields[1], "a") ||
            strcmp(rec.fields[2], "") ||
            strcmp(rec.fields[3], "b") ||
            strcmp(rec.fields[4], "")) {
            fprintf(stderr, "FAIL: csv parse empty fields\n");
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv parse empty\n");

    /* Test 4: Serialise simple record */
    {
        zenit_csv_record_t rec;
        rec.count = 3;
        rec.fields = malloc(3 * sizeof(char *));
        rec.fields[0] = strdup("a");
        rec.fields[1] = strdup("b");
        rec.fields[2] = strdup("c");

        char *out = NULL;
        zenit_result_t r = zenit_csv_serialise_record(&rec, ',', &out);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: csv serialise\n");
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        if (strcmp(out, "a,b,c") != 0) {
            fprintf(stderr, "FAIL: csv serialise got '%s'\n", out);
            free(out);
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        free(out);
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv serialise\n");

    /* Test 5: Serialise with quoting */
    {
        zenit_csv_record_t rec;
        rec.count = 2;
        rec.fields = malloc(2 * sizeof(char *));
        rec.fields[0] = strdup("hello, world");
        rec.fields[1] = strdup("foo\"bar");

        char *out = NULL;
        zenit_result_t r = zenit_csv_serialise_record(&rec, ',', &out);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: csv serialise quoting\n");
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        if (strcmp(out, "\"hello, world\",\"foo\"\"bar\"") != 0) {
            fprintf(stderr, "FAIL: csv serialise quoting got '%s'\n", out);
            free(out);
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        free(out);
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv serialise quoting\n");

    /* Test 6: Round-trip */
    {
        zenit_csv_record_t rec;
        zenit_csv_parse_record("a,\"b,c\",\"hello\"\"world\",,d", ',', &rec);
        char *out = NULL;
        zenit_csv_serialise_record(&rec, ',', &out);

        zenit_csv_record_t rec2;
        zenit_csv_parse_record(out, ',', &rec2);

        if (rec2.count != rec.count) {
            fprintf(stderr, "FAIL: csv round-trip count %zu != %zu\n", rec2.count, rec.count);
            free(out);
            zenit_csv_record_destroy(&rec);
            zenit_csv_record_destroy(&rec2);
            return 1;
        }
        for (size_t i = 0; i < rec.count; i++) {
            if (strcmp(rec.fields[i], rec2.fields[i]) != 0) {
                fprintf(stderr, "FAIL: csv round-trip field %zu mismatch\n", i);
                free(out);
                zenit_csv_record_destroy(&rec);
                zenit_csv_record_destroy(&rec2);
                return 1;
            }
        }

        free(out);
        zenit_csv_record_destroy(&rec);
        zenit_csv_record_destroy(&rec2);
    }
    printf("PASS: csv round-trip\n");

    /* Test 7: NULL params */
    {
        zenit_csv_record_t rec;
        zenit_result_t r = zenit_csv_parse_record(NULL, ',', &rec);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: csv parse NULL line\n");
            return 1;
        }
        r = zenit_csv_parse_record("a,b", ',', NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: csv parse NULL out\n");
            return 1;
        }
        r = zenit_csv_serialise_record(NULL, ',', NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: csv serialise NULL\n");
            return 1;
        }
        zenit_csv_record_destroy(NULL);
    }
    printf("PASS: csv NULL params\n");

    /* Test 8: Single field */
    {
        zenit_csv_record_t rec;
        zenit_result_t r = zenit_csv_parse_record("hello", ',', &rec);
        if (r.error != ZENIT_OK || rec.count != 1 || strcmp(rec.fields[0], "hello") != 0) {
            fprintf(stderr, "FAIL: csv single field\n");
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv single field\n");

    /* Test 9: Tab delimiter */
    {
        zenit_csv_record_t rec;
        zenit_result_t r = zenit_csv_parse_record("a\tb\tc", '\t', &rec);
        if (r.error != ZENIT_OK || rec.count != 3) {
            fprintf(stderr, "FAIL: csv tab delimiter\n");
            zenit_csv_record_destroy(&rec);
            return 1;
        }
        zenit_csv_record_destroy(&rec);
    }
    printf("PASS: csv tab delimiter\n");

    printf("PASS: csv\n");
    return 0;
}
