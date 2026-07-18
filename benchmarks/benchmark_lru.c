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
#include <libzenit/lru.h>
#include <stdlib.h>
#include <string.h>

/* Deterministic PRNG (xorshift32) — not crypto-secure, adequate for benchmarking */
static unsigned xorshift32(unsigned *state) {
    unsigned x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

#define N 100000
#define SMALL_CAPACITY 128

/* ─── Context ─── */

typedef struct {
    zenit_lru_t *cache;
    int *keys;
    int *values;
} lru_ctx_t;

/* ─── Shared setup / teardown ─── */

static lru_ctx_t *setup_base(int capacity) {
    lru_ctx_t *ctx = malloc(sizeof(lru_ctx_t));
    if (ctx == NULL) return NULL;
    ctx->cache = zenit_lru_create(sizeof(int), sizeof(int), (size_t)capacity);
    if (ctx->cache == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->keys = malloc(N * sizeof(int));
    ctx->values = malloc(N * sizeof(int));
    if (ctx->keys == NULL || ctx->values == NULL) {
        free(ctx->keys);
        free(ctx->values);
        zenit_lru_destroy(ctx->cache);
        free(ctx);
        return NULL;
    }
    for (int i = 0; i < N; i++) {
        ctx->keys[i] = i;
        ctx->values[i] = i;
    }
    return ctx;
}

static void teardown(lru_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_lru_destroy(ctx->cache);
    free(ctx->keys);
    free(ctx->values);
    free(ctx);
}

/* ─── Benchmark: sequential puts (fill warm, then overwrite) ─── */

static lru_ctx_t *setup_put(void) {
    lru_ctx_t *ctx = setup_base(N);
    if (ctx == NULL) return NULL;
    for (int i = 0; i < N; i++) {
        zenit_lru_put(ctx->cache, &ctx->keys[i], &ctx->values[i]);
    }
    return ctx;
}

static unsigned put_rng = 42;

static void bench_put_fn(void *vctx) {
    lru_ctx_t *ctx = (lru_ctx_t *)vctx;
    int k = (int)(xorshift32(&put_rng) % N);
    int v = k;
    zenit_lru_put(ctx->cache, &k, &v);
}

/* ─── Benchmark: get hits (warm cache, random access) ─── */

static lru_ctx_t *setup_get(void) {
    lru_ctx_t *ctx = setup_base(N);
    if (ctx == NULL) return NULL;
    for (int i = 0; i < N; i++) {
        zenit_lru_put(ctx->cache, &ctx->keys[i], &ctx->values[i]);
    }
    return ctx;
}

static unsigned get_rng = 137;

static void bench_get_hit_fn(void *vctx) {
    lru_ctx_t *ctx = (lru_ctx_t *)vctx;
    int k = (int)(xorshift32(&get_rng) % N);
    int out = 0;
    zenit_lru_get(ctx->cache, &k, &out);
}

/* ─── Benchmark: put with eviction (small capacity, many puts) ─── */

static lru_ctx_t *setup_put_evict(void) {
    lru_ctx_t *ctx = setup_base(SMALL_CAPACITY);
    if (ctx == NULL) return NULL;
    for (int i = 0; i < SMALL_CAPACITY; i++) {
        zenit_lru_put(ctx->cache, &ctx->keys[i], &ctx->values[i]);
    }
    return ctx;
}

static void bench_put_evict_fn(void *vctx) {
    lru_ctx_t *ctx = (lru_ctx_t *)vctx;
    static int counter = SMALL_CAPACITY;
    int k = counter++;
    int v = k;
    zenit_lru_put(ctx->cache, &k, &v);
}

/* ─── Main ─── */

int main(void) {
    lru_ctx_t *ctx;

    ctx = setup_put();
    if (ctx == NULL) return 1;
    zenit_bench_result_t r1 = zenit_bench_run("lru_put_100K", bench_put_fn, ctx, N);
    zenit_bench_print(&r1);
    teardown(ctx);

    ctx = setup_get();
    if (ctx == NULL) return 1;
    zenit_bench_result_t r2 = zenit_bench_run("lru_get_hit_100K", bench_get_hit_fn, ctx, N);
    zenit_bench_print(&r2);
    teardown(ctx);

    ctx = setup_put_evict();
    if (ctx == NULL) return 1;
    zenit_bench_result_t r3 = zenit_bench_run("lru_put_evict_100K", bench_put_evict_fn, ctx, N);
    zenit_bench_print(&r3);
    teardown(ctx);

    return 0;
}
