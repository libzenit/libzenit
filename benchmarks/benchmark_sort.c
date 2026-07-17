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

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

#define SORT_SIZE 10000

typedef struct {
    int *data;
    int *ref;
} sort_ctx_t;

static void bench_sort_random_fn(void *ctx) {
    const sort_ctx_t *c = (const sort_ctx_t *)ctx;
    memcpy(c->data, c->ref, SORT_SIZE * sizeof(int));
    zenit_sort_quick(c->data, SORT_SIZE, sizeof(int), cmp_int);
}

static void bench_sort_sorted_fn(void *ctx) {
    const sort_ctx_t *c = (const sort_ctx_t *)ctx;
    memcpy(c->data, c->ref, SORT_SIZE * sizeof(int));
    zenit_sort_quick(c->data, SORT_SIZE, sizeof(int), cmp_int);
}

static void bench_binary_search_hit_fn(void *ctx) {
    const sort_ctx_t *c = (const sort_ctx_t *)ctx;
    int key = c->data[SORT_SIZE / 2];
    zenit_binary_search(&key, c->data, SORT_SIZE, sizeof(int), cmp_int);
}

static void bench_binary_search_miss_fn(void *ctx) {
    const sort_ctx_t *c = (const sort_ctx_t *)ctx;
    int key = -999999;
    zenit_binary_search(&key, c->data, SORT_SIZE, sizeof(int), cmp_int);
}

/* Simple deterministic PRNG (xorshift32) */
static unsigned xorshift32(unsigned *state) {
    unsigned x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
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

    unsigned rng = 42;
    for (int i = 0; i < SORT_SIZE; i++) {
        ctx.ref[i] = (int)xorshift32(&rng);
    }

    {
        zenit_bench_result_t r = zenit_bench_run(
            "sort_random_10K", bench_sort_random_fn, &ctx, 1000
        );
        zenit_bench_print(&r);
    }

    {
        zenit_bench_result_t r = zenit_bench_run(
            "sort_sorted_10K", bench_sort_sorted_fn, &ctx, 1000
        );
        zenit_bench_print(&r);
    }

    {
        zenit_bench_result_t r = zenit_bench_run(
            "binary_search_hit", bench_binary_search_hit_fn, &ctx, 1000000
        );
        zenit_bench_print(&r);
    }

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
