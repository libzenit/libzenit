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
#include <libzenit/sort.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

/* Pre-allocated array that we re-fill before each call */
#define SORT_SIZE 10000

typedef struct {
    int *data;
    int *ref;
} sort_ctx_t;

static void bench_sort_random_fn(void *ctx) {
    sort_ctx_t *c = (sort_ctx_t *)ctx;
    /* Copy from reference (already randomised) so each call sorts the same */
    memcpy(c->data, c->ref, SORT_SIZE * sizeof(int));
    zenit_sort_quick(c->data, SORT_SIZE, sizeof(int), cmp_int);
}

static void bench_sort_sorted_fn(void *ctx) {
    sort_ctx_t *c = (sort_ctx_t *)ctx;
    memcpy(c->data, c->ref, SORT_SIZE * sizeof(int));
    zenit_sort_quick(c->data, SORT_SIZE, sizeof(int), cmp_int);
}

static void bench_binary_search_hit_fn(void *ctx) {
    sort_ctx_t *c = (sort_ctx_t *)ctx;
    int key = c->data[SORT_SIZE / 2];
    zenit_binary_search(&key, c->data, SORT_SIZE, sizeof(int), cmp_int);
}

static void bench_binary_search_miss_fn(void *ctx) {
    sort_ctx_t *c = (sort_ctx_t *)ctx;
    int key = -999999;
    zenit_binary_search(&key, c->data, SORT_SIZE, sizeof(int), cmp_int);
}

int main(void) {
    sort_ctx_t ctx;

    ctx.data = malloc(SORT_SIZE * sizeof(int));
    ctx.ref  = malloc(SORT_SIZE * sizeof(int));
    if (ctx.data == NULL || ctx.ref == NULL) {
        free(ctx.data);
        free(ctx.ref);
        return 1;
    }

    /* Seed and fill reference with random data */
    srand(42);
    for (int i = 0; i < SORT_SIZE; i++) {
        ctx.ref[i] = rand();
    }

    /* Sort random data */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "sort_random_10K", bench_sort_random_fn, &ctx, 1000
        );
        zenit_bench_print(&r);
    }

    /* Sort already-sorted data (best case) */
    /* ctx.ref is now sorted from the previous call */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "sort_sorted_10K", bench_sort_sorted_fn, &ctx, 1000
        );
        zenit_bench_print(&r);
    }

    /* Binary search hit */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "binary_search_hit", bench_binary_search_hit_fn, &ctx, 1000000
        );
        zenit_bench_print(&r);
    }

    /* Binary search miss */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "binary_search_miss", bench_binary_search_miss_fn, &ctx, 1000000
        );
        zenit_bench_print(&r);
    }

    free(ctx.data);
    free(ctx.ref);
    return 0;
}
