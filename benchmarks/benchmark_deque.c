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

#include <libzenit/deque.h>
#include <libzenit/benchmark.h>
#include <stdint.h>
#include <stdlib.h>

/* ─── Bench: push_back N elements ─── */
static void bench_push_back(void *ctx) {
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_deque_push_back(d, &i);
    }
    zenit_deque_destroy(d);
}

/* ─── Bench: push_front N elements ─── */
static void bench_push_front(void *ctx) {
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_deque_push_front(d, &i);
    }
    zenit_deque_destroy(d);
}

/* ─── Bench: push_back then pop_front N ─── */
static void bench_push_pop(void *ctx) {
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_deque_push_back(d, &i);
    }
    int out;
    for (int i = 0; i < n; i++) {
        zenit_deque_pop_front(d, &out);
    }
    zenit_deque_destroy(d);
}

int main(void) {
    int n = 1000000;

    zenit_bench_result_t r;

    r = zenit_bench_run("deque_push_back_1M", bench_push_back, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    r = zenit_bench_run("deque_push_front_1M", bench_push_front, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    r = zenit_bench_run("deque_push_pop_1M", bench_push_pop, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    return 0;
}
