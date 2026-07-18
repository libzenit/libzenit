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
#include <libzenit/logger.h>

/* Null sink that discards messages */
static void null_sink(const char *message, void *ctx) {
    (void)message;
    (void)ctx;
}

static void bench_logger_info(void *ctx) {
    zenit_logger_t *log = (zenit_logger_t *)ctx;
    zenit_logger_info(log, "benchmark message: %d", 42);
}

static void bench_logger_filtered(void *ctx) {
    zenit_logger_t *log = (zenit_logger_t *)ctx;
    /* TRACE level, but min level is WARN → filtered out fast */
    zenit_logger_trace(log, "filtered message: %d", 99);
}

int main(void) {
    /* Logger with null sink — measures overhead of formatting + dispatch */
    zenit_logger_t *log = zenit_logger_create(null_sink, NULL);

    zenit_bench_result_t r;

    r = zenit_bench_run("logger_info", bench_logger_info, log, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("logger_filtered", bench_logger_filtered, log, 1000000);
    zenit_bench_print(&r);

    zenit_logger_destroy(log);

    /* Measure filtered path with level filter */
    zenit_logger_t *filtered = zenit_logger_create(null_sink, NULL);
    zenit_logger_set_level(filtered, ZENIT_LOG_WARN);

    r = zenit_bench_run("logger_filtered_fast", bench_logger_filtered, filtered, 1000000);
    zenit_bench_print(&r);

    zenit_logger_destroy(filtered);

    return 0;
}
