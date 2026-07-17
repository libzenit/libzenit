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
#include <libzenit/str.h>
#include <stdlib.h>
#include <string.h>

static const char *trim_input = "  \t  hello world  \n  ";
static const char *split_input = "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p";
static const char *parts[] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p"};

static void bench_trim_fn(void *ctx) {
    (void)ctx;
    char *t = zenit_str_trim(trim_input);
    if (t) free(t);
}

static void bench_split_fn(void *ctx) {
    (void)ctx;
    size_t count;
    char **r = zenit_str_split(split_input, ",", &count);
    if (r) {
        for (size_t i = 0; i < count; i++) free(r[i]);
        free(r);
    }
}

static void bench_join_fn(void *ctx) {
    (void)ctx;
    char *j = zenit_str_join(parts, 16, ",");
    if (j) free(j);
}

int main(void) {
    /* Trim */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "str_trim", bench_trim_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Split */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "str_split_16", bench_split_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    /* Join */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "str_join_16", bench_join_fn, NULL, 100000
        );
        zenit_bench_print(&r);
    }

    return 0;
}
