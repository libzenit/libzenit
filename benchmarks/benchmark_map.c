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
#include <libzenit/map.h>
#include <stdlib.h>

/* ─── Context: a map pre-populated with N entries ─── */
typedef struct {
    zenit_map_t *map;
    int *keys;
    int *values;
    size_t count;
} bench_ctx_t;

/* ─── Helper: create a map with N entries ─── */
static bench_ctx_t *make_map(size_t n) {
    bench_ctx_t *ctx = malloc(sizeof(bench_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->map = zenit_map_create(sizeof(int), sizeof(int));
    if (ctx->map == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->keys = malloc(n * sizeof(int));
    ctx->values = malloc(n * sizeof(int));
    if (ctx->keys == NULL || ctx->values == NULL) {
        free(ctx->keys);
        free(ctx->values);
        zenit_map_destroy(ctx->map);
        free(ctx);
        return NULL;
    }

    ctx->count = n;

    for (size_t i = 0; i < n; i++) {
        ctx->keys[i] = (int)i;
        ctx->values[i] = (int)(i * 2);
        zenit_map_insert(ctx->map, &ctx->keys[i], &ctx->values[i]);
    }

    return ctx;
}

static void destroy_ctx(bench_ctx_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    zenit_map_destroy(ctx->map);
    free(ctx->keys);
    free(ctx->values);
    free(ctx);
}

/* ─── Bench: sequential insert of N new keys ─── */
static int insert_counter = 0;

static void bench_insert(void *ptr) {
    bench_ctx_t *ctx = (bench_ctx_t *)ptr;
    int key = insert_counter++;
    int value = key * 3;
    zenit_map_insert(ctx->map, &key, &value);
}

/* ─── Bench: get (hit) ─── */
static size_t get_hit_idx = 0;

static void bench_get_hit(void *ptr) {
    const bench_ctx_t *ctx = (const bench_ctx_t *)ptr;
    size_t i = get_hit_idx++ % ctx->count;
    int key = ctx->keys[i];
    int out = 0;
    zenit_map_get(ctx->map, &key, &out);
}

/* ─── Bench: get (miss — key not in map) ─── */
static int get_miss_counter = 1000000;

static void bench_get_miss(void *ptr) {
    const bench_ctx_t *ctx = (const bench_ctx_t *)ptr;
    int key = get_miss_counter++;
    int out = 0;
    zenit_map_get(ctx->map, &key, &out);
}

/* ─── Bench: insert (triggering rehash) ─── */
static int rehash_counter = 0;

static void bench_insert_rehash(void *ptr) {
    bench_ctx_t *ctx = (bench_ctx_t *)ptr;
    int key = rehash_counter++;
    int value = key * 3;
    zenit_map_insert(ctx->map, &key, &value);
}

/* ─── Bench: foreach iteration ─── */
static void bench_visit(const void *key, const void *value, void *ctx) {
    (void)key;
    (void)value;
    (void)ctx;
}

static void bench_foreach(void *ptr) {
    const bench_ctx_t *ctx = (const bench_ctx_t *)ptr;
    zenit_map_foreach(ctx->map, bench_visit, NULL);
}

/* ─── Main ─── */
int main(void) {
    /* Pre-populated map for get/foreach benchmarks */
    bench_ctx_t *full = make_map(100000);
    if (full == NULL) {
        return 1;
    }

    /* For insert benchmark we need an empty map */
    bench_ctx_t *empty = make_map(0);
    if (empty == NULL) {
        destroy_ctx(full);
        return 1;
    }

    /* For rehash benchmark we need a map at capacity */
    bench_ctx_t *growing = make_map(0);
    if (growing == NULL) {
        destroy_ctx(full);
        destroy_ctx(empty);
        return 1;
    }

    zenit_bench_result_t r;

    r = zenit_bench_run("map_insert_100K", bench_insert, empty, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("map_get_hit_100K", bench_get_hit, full, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("map_get_miss_100K", bench_get_miss, full, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("map_insert_rehash_100K", bench_insert_rehash, growing, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("map_foreach_100K", bench_foreach, full, 1000);
    zenit_bench_print(&r);

    destroy_ctx(full);
    destroy_ctx(empty);
    destroy_ctx(growing);

    return 0;
}
