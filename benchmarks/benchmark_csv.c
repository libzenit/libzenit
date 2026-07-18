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
#include <libzenit/csv.h>
#include <stdlib.h>
#include <string.h>

static void bench_csv_parse(void *ctx) {
    (void)ctx;
    zenit_csv_record_t rec;
    zenit_csv_parse_record("a,b,c,1,2,3,\"hello world\",\"quoted,field\",foo", ',', &rec);
    zenit_csv_record_destroy(&rec);
}

static void bench_csv_serialise(void *ctx) {
    (void)ctx;
    zenit_csv_record_t *rec = (zenit_csv_record_t *)ctx;
    char *out = NULL;
    zenit_csv_serialise_record(rec, ',', &out);
    free(out);
}

int main(void) {
    /* Build a record for serialisation benchmark */
    zenit_csv_record_t rec;
    rec.count = 9;
    rec.fields = malloc(9 * sizeof(char *));
    rec.fields[0] = strdup("a");
    rec.fields[1] = strdup("b");
    rec.fields[2] = strdup("c");
    rec.fields[3] = strdup("1");
    rec.fields[4] = strdup("2");
    rec.fields[5] = strdup("3");
    rec.fields[6] = strdup("hello world");
    rec.fields[7] = strdup("quoted,field");
    rec.fields[8] = strdup("foo");

    zenit_bench_result_t r;

    r = zenit_bench_run("csv_parse", bench_csv_parse, NULL, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("csv_serialise", bench_csv_serialise, &rec, 100000);
    zenit_bench_print(&r);

    for (size_t i = 0; i < rec.count; i++) free(rec.fields[i]);
    free(rec.fields);

    return 0;
}
