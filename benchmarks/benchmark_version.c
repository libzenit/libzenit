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
#include <libzenit/version.h>

/* Benchmark: measure libzenit_version() call overhead.
 * The volatile qualifier prevents the compiler from eliding the call. */
static void bench_libzenit_version(void* ctx) {
    (void)ctx;

    volatile libzenit_version_t v = libzenit_version();
    (void)v;
}

int main(void) {
    zenit_bench_result_t r = zenit_bench_run(
        "libzenit_version", bench_libzenit_version, NULL, 100000000
    );
    zenit_bench_print(&r);
    return 0;
}
