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
#include <libzenit/pool.h>
#include <stdlib.h>

typedef struct {
    zenit_pool_t *pool;
} acquire_ctx_t;

static acquire_ctx_t* acquire_setup(void) {
    acquire_ctx_t *ctx = malloc(sizeof(acquire_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->pool = zenit_pool_create(sizeof(int), 1000000UL);
    if (ctx->pool == NULL) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

static void acquire_teardown(acquire_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_pool_destroy(ctx->pool);
    free(ctx);
}

static void bench_acquire_fn(void *ctx) {
    zenit_pool_acquire(((acquire_ctx_t *)ctx)->pool);
}

typedef struct {
    zenit_pool_t *pool;
} acquire_release_ctx_t;

static acquire_release_ctx_t* acquire_release_setup(void) {
    acquire_release_ctx_t *ctx = malloc(sizeof(acquire_release_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->pool = zenit_pool_create(sizeof(int), 1000000UL);
    if (ctx->pool == NULL) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

static void acquire_release_teardown(acquire_release_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_pool_destroy(ctx->pool);
    free(ctx);
}

static void bench_acquire_release_fn(void *ctx) {
    acquire_release_ctx_t *c = (acquire_release_ctx_t *)ctx;
    void *obj = zenit_pool_acquire(c->pool);
    if (obj != NULL) {
        zenit_pool_release(c->pool, obj);
    }
}

typedef struct {
    zenit_pool_t *pool;
} small_ctx_t;

static small_ctx_t* small_setup(void) {
    small_ctx_t *ctx = malloc(sizeof(small_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->pool = zenit_pool_create(sizeof(int), 100);
    if (ctx->pool == NULL) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

static void small_teardown(small_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_pool_destroy(ctx->pool);
    free(ctx);
}

static void bench_small_fn(void *ctx) {
    small_ctx_t *c = (small_ctx_t *)ctx;
    void *obj = zenit_pool_acquire(c->pool);
    if (obj != NULL) {
        zenit_pool_release(c->pool, obj);
    }
}

int main(void) {
    {
        acquire_ctx_t *ctx = acquire_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("pool_acquire", bench_acquire_fn, ctx, 1000000);
            zenit_bench_print(&r);
            acquire_teardown(ctx);
        }
    }
    {
        acquire_release_ctx_t *ctx = acquire_release_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("pool_acquire_release", bench_acquire_release_fn, ctx, 1000000);
            zenit_bench_print(&r);
            acquire_release_teardown(ctx);
        }
    }
    {
        small_ctx_t *ctx = small_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("pool_small_acquire_release", bench_small_fn, ctx, 1000000);
            zenit_bench_print(&r);
            small_teardown(ctx);
        }
    }
    return 0;
}
