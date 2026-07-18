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
#include <libzenit/semver.h>
#include <stdlib.h>

static void bench_semver_parse(void *ctx) {
    (void)ctx;
    zenit_semver_t v;
    zenit_semver_parse("3.2.1-beta.2+build.99", &v);
}

static void bench_semver_format(void *ctx) {
    (void)ctx;
    const zenit_semver_t *v = (const zenit_semver_t *)ctx;
    char *s = NULL;
    zenit_semver_format(v, &s);
    free(s);
}

static void bench_semver_compare(void *ctx) {
    (void)ctx;
    const zenit_semver_t *versions = (const zenit_semver_t *)ctx;
    zenit_semver_compare(&versions[0], &versions[1]);
}

int main(void) {
    zenit_semver_t v;
    zenit_semver_parse("3.2.1-beta.2+build.99", &v);

    zenit_semver_t va;
    zenit_semver_t vb;
    zenit_semver_parse("1.2.3", &va);
    zenit_semver_parse("1.2.4", &vb);
    zenit_semver_t versions[2] = { va, vb };

    zenit_bench_result_t r;

    r = zenit_bench_run("semver_parse", bench_semver_parse, NULL, 1000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("semver_format", bench_semver_format, &v, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("semver_compare", bench_semver_compare, versions, 1000000);
    zenit_bench_print(&r);

    return 0;
}
