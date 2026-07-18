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
#include <libzenit/result.h>

static void bench_error_string(void *ctx) {
    (void)ctx;
    /* Exercise every error code */
    (void)zenit_error_string(ZENIT_OK);
    (void)zenit_error_string(ZENIT_ERROR_NULL);
    (void)zenit_error_string(ZENIT_ERROR_ALLOC);
    (void)zenit_error_string(ZENIT_ERROR_NOT_FOUND);
    (void)zenit_error_string(ZENIT_ERROR_FULL);
    (void)zenit_error_string(ZENIT_ERROR_EMPTY);
    (void)zenit_error_string(ZENIT_ERROR_PARAM);
    (void)zenit_error_string(ZENIT_ERROR_CORRUPT);
    (void)zenit_error_string(ZENIT_ERROR_DOUBLE_FREE);
    (void)zenit_error_string(ZENIT_ERROR_SIZE);
    (void)zenit_error_string((zenit_error_t)999);
}

int main(void) {
    zenit_bench_result_t r;

    r = zenit_bench_run("error_string", bench_error_string, NULL, 1000000);
    zenit_bench_print(&r);

    return 0;
}
