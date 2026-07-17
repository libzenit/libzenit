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

#include <libzenit/benchmark.h>
#include <libzenit/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A moderately complex JSON input for parsing benchmarks */
static const char *sample_json =
    "{"
    "  \"name\": \"LibZenit\","
    "  \"version\": 1,"
    "  \"description\": \"Portable C library for systems programming\","
    "  \"features\": [\"json\", \"state\", \"arena\", \"ring\", \"vector\"],"
    "  \"metadata\": {"
    "    \"active\": true,"
    "    \"count\": 42,"
    "    \"ratio\": 3.14,"
    "    \"tags\": [\"c\", \"library\", \"embedded\"],"
    "    \"config\": {"
    "      \"debug\": false,"
    "      \"level\": 5"
    "    }"
    "  }"
    "}";

/* ─── Benchmark: parse ─── */

static void bench_parse(void *ctx) {
    (void)ctx;
    zenit_json_t *doc = zenit_json_parse(sample_json);
    if (doc == NULL) {
        fprintf(stderr, "bench_parse: parse returned NULL\n");
        return;
    }
    zenit_json_destroy(doc);
}

/* ─── Benchmark: serialize ─── */

static void bench_serialize(void *ctx) {
    (void)ctx;
    zenit_json_t *doc = zenit_json_parse(sample_json);
    if (doc == NULL) {
        fprintf(stderr, "bench_serialize: parse returned NULL\n");
        return;
    }
    char *out = zenit_json_serialize(doc);
    free(out);
    zenit_json_destroy(doc);
}

/* ─── Benchmark: programmatic construction ─── */

static void bench_build(void *ctx) {
    (void)ctx;
    zenit_json_t *doc = zenit_json_create();
    if (doc == NULL) { return; }

    zenit_json_value_t *obj = zenit_json_value_object(doc);
    zenit_json_set_root(doc, obj);
    zenit_json_object_set(obj, "name", zenit_json_value_string(doc, "test"));
    zenit_json_object_set(obj, "value", zenit_json_value_number(doc, 42.0));
    zenit_json_object_set(obj, "flag", zenit_json_value_bool(doc, 1));

    zenit_json_value_t *arr = zenit_json_value_array(doc);
    for (int i = 0; i < 10; i++) {
        zenit_json_array_append(arr, zenit_json_value_number(doc, (double)i));
    }
    zenit_json_object_set(obj, "items", arr);

    zenit_json_destroy(doc);
}

int main(void) {
    /* Parse */
    zenit_bench_result_t r1 = zenit_bench_run("json_parse", bench_parse, NULL, 50000);
    zenit_bench_print(&r1);

    /* Parse + serialize */
    zenit_bench_result_t r2 = zenit_bench_run("json_serialize", bench_serialize, NULL, 50000);
    zenit_bench_print(&r2);

    /* Build programmatically */
    zenit_bench_result_t r3 = zenit_bench_run("json_build", bench_build, NULL, 50000);
    zenit_bench_print(&r3);

    return 0;
}
