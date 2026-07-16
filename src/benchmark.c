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

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

/* Return wall-clock time in seconds using the best high-resolution clock
 * available on the target platform. */
static double bench_time_s(void) {
#if defined(_WIN32)
    /* Windows high-performance counter (nanosecond resolution) */
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    /* POSIX monotonic clock — immune to wall-clock adjustments */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

zenit_bench_result_t zenit_bench_run(
    const char* name,
    zenit_bench_fn_t fn,
    void* ctx,
    long iterations
) {
    /* Warm-up: one call to stabilise caches and branch predictors */
    fn(ctx);

    /* Timed run */
    double start = bench_time_s();
    for (long i = 0; i < iterations; i++) {
        fn(ctx);
    }
    double end = bench_time_s();

    double elapsed = end - start;
    double ops_per_sec = (elapsed > 0.0) ? (double)iterations / elapsed : 0.0;

    /* Use designated initializer (C99) */
    zenit_bench_result_t result = {
        .name = name,
        .elapsed_s = elapsed,
        .iterations = iterations,
        .ops_per_sec = ops_per_sec
    };
    return result;
}

void zenit_bench_print(const zenit_bench_result_t* result) {
    printf("%-24s  %10ld iters  %8.4f s  %12.0f ops/s\n",
           result->name,
           result->iterations,
           result->elapsed_s,
           result->ops_per_sec);
}
