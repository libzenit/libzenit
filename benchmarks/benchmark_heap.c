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

#include <libzenit/heap.h>
#include <libzenit/benchmark.h>
#include <stdint.h>
#include <stdlib.h>

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (ia > ib) return 1;
    if (ia < ib) return -1;
    return 0;
}

/* ─── Bench: push N elements ─── */
static void bench_push(void *ctx) {
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int);
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_heap_push(heap, &i);
    }
    zenit_heap_destroy(heap);
}

/* ─── Bench: push then pop all N ─── */
static void bench_push_pop(void *ctx) {
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int);
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_heap_push(heap, &i);
    }
    int out;
    for (int i = 0; i < n; i++) {
        zenit_heap_pop(heap, &out);
    }
    zenit_heap_destroy(heap);
}

/* ─── Bench: peek N times on a heap ─── */
static void bench_peek(void *ctx) {
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int);
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_heap_push(heap, &i);
    }
    for (int i = 0; i < n; i++) {
        zenit_heap_peek(heap);
    }
    zenit_heap_destroy(heap);
}

int main(void) {
    int n = 100000;

    zenit_bench_result_t r;

    r = zenit_bench_run("heap_push_100K", bench_push, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    r = zenit_bench_run("heap_push_pop_100K", bench_push_pop, (void *)(intptr_t)n, 20);
    zenit_bench_print(&r);

    r = zenit_bench_run("heap_peek_100K", bench_peek, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    return 0;
}
