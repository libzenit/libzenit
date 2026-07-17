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
#include <libzenit/uri.h>
#include <stdlib.h>
#include <string.h>

/* 200-byte string with mix of unreserved and reserved characters */
static const char *test_input =
    "https://example.com/path/to/resource?query=hello world&foo=bar+baz#fragment";
static char *encoded = NULL;

static void bench_encode_fn(void *ctx) {
    (void)ctx;
    char *e = zenit_uri_encode(test_input);
    if (e) free(e);
}

static void bench_decode_fn(void *ctx) {
    (void)ctx;
    char *d = zenit_uri_decode(encoded);
    if (d) free(d);
}

int main(void) {
    encoded = zenit_uri_encode(test_input);

    /* Encode */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "uri_encode", bench_encode_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Decode */
    if (encoded != NULL) {
        zenit_bench_result_t r = zenit_bench_run(
            "uri_decode", bench_decode_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    free(encoded);
    return 0;
}
