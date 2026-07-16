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
#include <stdio.h>

static int call_count = 0;

static void dummy_fn(void* ctx) {
    (void)ctx;
    call_count++;
}

int main(void) {
    call_count = 0;

    /* Run a benchmark with a trivial no-op function */
    zenit_bench_result_t r = zenit_bench_run("dummy", dummy_fn, NULL, 1000);

    /* Verify every field in the result */
    if (r.name == NULL) {
        fprintf(stderr, "FAIL: name is NULL\n");
        return 1;
    }
    if (r.elapsed_s <= 0.0) {
        fprintf(stderr, "FAIL: elapsed_s (%f) not positive\n", r.elapsed_s);
        return 1;
    }
    if (r.iterations != 1000) {
        fprintf(stderr, "FAIL: iterations == %ld, expected 1000\n", r.iterations);
        return 1;
    }
    if (r.ops_per_sec <= 0.0) {
        fprintf(stderr, "FAIL: ops_per_sec (%f) not positive\n", r.ops_per_sec);
        return 1;
    }

    /* 1 warm-up + 1000 timed calls */
    if (call_count != 1001) {
        fprintf(stderr, "FAIL: fn called %d times, expected 1001\n", call_count);
        return 1;
    }

    /* Print must not crash */
    zenit_bench_print(&r);

    printf("PASS: benchmark API\n");
    return 0;
}
