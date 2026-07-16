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

#ifndef LIBZENIT_BENCHMARK_H
#define LIBZENIT_BENCHMARK_H

#include <stddef.h>

/**
 * @brief Result of a single benchmark run.
 *
 * Contains the benchmark name, wall-clock time, number of iterations, and
 * throughput in operations per second.
 */
typedef struct {
    const char* name;       /**< Human-readable benchmark label. */
    double elapsed_s;        /**< Wall-clock time in seconds. */
    long iterations;         /**< Number of times the function was called. */
    double ops_per_sec;      /**< Throughput (iterations / elapsed_s). */
} zenit_bench_result_t;

/**
 * @brief Pointer to the function under benchmark.
 *
 * @param ctx Opaque context pointer supplied at benchmark creation.
 */
typedef void (*zenit_bench_fn_t)(void* ctx);

/**
 * @brief Run a benchmark.
 *
 * Calls @p fn @p iterations times while measuring wall-clock time.
 * A single warm-up iteration executes before the timed loop to stabilise
 * CPU caches and branch predictors.
 *
 * @param name       Human-readable label for the benchmark.
 * @param fn         Function to benchmark.
 * @param ctx        Opaque context passed to @p fn on every call.
 * @param iterations Number of times to invoke @p fn.
 * @return A filled #zenit_bench_result_t.
 */
zenit_bench_result_t zenit_bench_run(
    const char* name,
    zenit_bench_fn_t fn,
    void* ctx,
    long iterations
);

/**
 * @brief Print a benchmark result to stdout.
 *
 * Output format (aligned columns):
 * @verbatim
 * name                      1000000 iters    0.1234 s    8101234 ops/s
 * @endverbatim
 *
 * @param result Pointer to a benchmark result. Must not be NULL.
 */
void zenit_bench_print(const zenit_bench_result_t* result);

#endif
