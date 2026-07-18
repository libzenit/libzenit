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
#include <libzenit/base64.h>
#include <stdlib.h>
#include <string.h>

/* 256 bytes of test data */
static unsigned char test_data[256];
static char *encoded = NULL;

static void bench_encode_fn(void *ctx) {
    (void)ctx;
    char *e = zenit_base64_encode(test_data, 256);
    if (e) free(e);
}

static void bench_decode_fn(void *ctx) {
    (void)ctx;
    size_t len;
    unsigned char *d = zenit_base64_decode(encoded, &len);
    if (d) free(d);
}

int main(void) {
    for (int i = 0; i < 256; i++) test_data[i] = (unsigned char)i;
    encoded = zenit_base64_encode(test_data, 256);

    /* Encode */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "base64_encode_256B", bench_encode_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Decode */
    if (encoded != NULL) {
        zenit_bench_result_t r = zenit_bench_run(
            "base64_decode_256B", bench_decode_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    free(encoded);
    return 0;
}
