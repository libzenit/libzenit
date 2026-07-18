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
#include <libzenit/path.h>
#include <stdlib.h>
#include <string.h>

static const char *join_a = "/usr/local";
static const char *join_b = "lib/zenit";

static const char *dirname_input = "/usr/local/lib/zenit";

static const char *basename_input = "/usr/local/lib/zenit";

static const char *normalize_input = "/usr/local/../lib/./zenit";

static void bench_join_fn(void *ctx) {
    (void)ctx;
    char *r = zenit_path_join(join_a, join_b);
    if (r) free(r);
}

static void bench_dirname_fn(void *ctx) {
    (void)ctx;
    char *r = zenit_path_dirname(dirname_input);
    if (r) free(r);
}

static void bench_basename_fn(void *ctx) {
    (void)ctx;
    char *r = zenit_path_basename(basename_input);
    if (r) free(r);
}

static void bench_normalize_fn(void *ctx) {
    (void)ctx;
    char *r = zenit_path_normalize(normalize_input);
    if (r) free(r);
}

int main(void) {
    /* Join */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "path_join", bench_join_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Dirname */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "path_dirname", bench_dirname_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Basename */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "path_basename", bench_basename_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Normalize */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "path_normalize", bench_normalize_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    return 0;
}
