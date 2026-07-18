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
#include <libzenit/glob.h>

static void bench_glob_exact(void *ctx) {
    (void)ctx;
    zenit_glob_match("hello_world.txt", "hello_world.txt");
}

static void bench_glob_star(void *ctx) {
    (void)ctx;
    zenit_glob_match("*.txt", "readme.txt");
}

static void bench_glob_star_miss(void *ctx) {
    (void)ctx;
    zenit_glob_match("*.txt", "readme.md");
}

static void bench_glob_question(void *ctx) {
    (void)ctx;
    zenit_glob_match("hello.???", "hello.c");
}

static void bench_glob_class(void *ctx) {
    (void)ctx;
    zenit_glob_match("file_[a-z].txt", "file_x.txt");
}

int main(void) {
    zenit_bench_result_t r;

    r = zenit_bench_run("glob_exact", bench_glob_exact, NULL, 10000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("glob_star", bench_glob_star, NULL, 10000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("glob_star_miss", bench_glob_star_miss, NULL, 10000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("glob_question", bench_glob_question, NULL, 10000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("glob_class", bench_glob_class, NULL, 10000000);
    zenit_bench_print(&r);

    return 0;
}
