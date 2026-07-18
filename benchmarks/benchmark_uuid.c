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
#include <libzenit/uuid.h>

static void bench_uuid_generate(void *ctx) {
    (void)ctx;
    zenit_uuid_t u;
    zenit_uuid_generate(&u);
}

static void bench_uuid_format(void *ctx) {
    (void)ctx;
    zenit_uuid_t *u = (zenit_uuid_t *)ctx;
    char buf[ZENIT_UUID_STR_LEN];
    zenit_uuid_format(u, buf);
}

static void bench_uuid_parse(void *ctx) {
    (void)ctx;
    zenit_uuid_t u;
    zenit_uuid_parse("550e8400-e29b-41d4-a716-446655440000", &u);
}

int main(void) {
    zenit_uuid_t u;
    zenit_uuid_generate(&u);

    zenit_bench_result_t r;

    r = zenit_bench_run("uuid_generate", bench_uuid_generate, NULL, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("uuid_format", bench_uuid_format, &u, 1000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("uuid_parse", bench_uuid_parse, NULL, 1000000);
    zenit_bench_print(&r);

    return 0;
}
