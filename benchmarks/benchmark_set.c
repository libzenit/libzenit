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
#include <libzenit/set.h>
#include <stdlib.h>

/* ─── Context: a set pre-populated with N entries ─── */
typedef struct {
    zenit_set_t *set;
    int *keys;
    size_t count;
} bench_ctx_t;

/* ─── Helper: create a set with N entries ─── */
static bench_ctx_t *make_set(size_t n) {
    bench_ctx_t *ctx = malloc(sizeof(bench_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->set = zenit_set_create(sizeof(int));
    if (ctx->set == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->keys = malloc(n * sizeof(int));
    if (n > 0 && ctx->keys == NULL) {
        zenit_set_destroy(ctx->set);
        free(ctx);
        return NULL;
    }

    ctx->count = n;

    for (size_t i = 0; i < n; i++) {
        ctx->keys[i] = (int)i;
        zenit_set_insert(ctx->set, &ctx->keys[i]);
    }

    return ctx;
}

static void destroy_ctx(bench_ctx_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    zenit_set_destroy(ctx->set);
    free(ctx->keys);
    free(ctx);
}

/* ─── Bench: sequential insert of N new keys ─── */
static int insert_counter = 0;

static void bench_insert(void *ptr) {
    bench_ctx_t *ctx = (bench_ctx_t *)ptr;
    int key = insert_counter++;
    zenit_set_insert(ctx->set, &key);
}

/* ─── Bench: contains (hit) ─── */
static size_t contains_hit_idx = 0;

static void bench_contains_hit(void *ptr) {
    const bench_ctx_t *ctx = (const bench_ctx_t *)ptr;
    size_t i = contains_hit_idx++ % ctx->count;
    int key = ctx->keys[i];
    zenit_set_contains(ctx->set, &key);
}

/* ─── Bench: contains (miss — key not in set) ─── */
static int contains_miss_counter = 1000000;

static void bench_contains_miss(void *ptr) {
    const bench_ctx_t *ctx = (const bench_ctx_t *)ptr;
    int key = contains_miss_counter++;
    zenit_set_contains(ctx->set, &key);
}

/* ─── Bench: insert (triggering rehash) ─── */
static int rehash_counter = 0;

static void bench_insert_rehash(void *ptr) {
    bench_ctx_t *ctx = (bench_ctx_t *)ptr;
    int key = rehash_counter++;
    zenit_set_insert(ctx->set, &key);
}

/* ─── Bench: foreach iteration ─── */
static void bench_visit(const void *key, void *ctx) {
    (void)key;
    (void)ctx;
}

static void bench_foreach(void *ptr) {
    const bench_ctx_t *ctx = (const bench_ctx_t *)ptr;
    zenit_set_foreach(ctx->set, bench_visit, NULL);
}

/* ─── Main ─── */
int main(void) {
    bench_ctx_t *full = make_set(100000);
    if (full == NULL) {
        return 1;
    }

    bench_ctx_t *empty = make_set(0);
    if (empty == NULL) {
        destroy_ctx(full);
        return 1;
    }

    bench_ctx_t *growing = make_set(0);
    if (growing == NULL) {
        destroy_ctx(full);
        destroy_ctx(empty);
        return 1;
    }

    zenit_bench_result_t r;

    r = zenit_bench_run("set insert (100K)", bench_insert, empty, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("set contains (hit 100K)", bench_contains_hit, full, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("set contains (miss 100K)", bench_contains_miss, full, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("set insert rehash (100K)", bench_insert_rehash, growing, 100000);
    zenit_bench_print(&r);

    r = zenit_bench_run("set foreach (100K)", bench_foreach, full, 1000);
    zenit_bench_print(&r);

    destroy_ctx(full);
    destroy_ctx(empty);
    destroy_ctx(growing);

    return 0;
}
