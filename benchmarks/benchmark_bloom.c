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
#include <libzenit/bloom.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Context shared by insert and contains-hit benchmarks.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_bloom_t *bf;
    unsigned char **items;
    size_t count;
    size_t item_len;
    size_t index; /* used to cycle through items sequentially */
} bench_ctx_t;

static bench_ctx_t* ctx_setup(size_t num_items) {
    bench_ctx_t *ctx = malloc(sizeof(bench_ctx_t));
    if (ctx == NULL) return NULL;

    /* Create a Bloom filter sized for num_items with 1 % FPR */
    ctx->bf = zenit_bloom_create(num_items, 0.01);
    if (ctx->bf == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->items = malloc(num_items * sizeof(unsigned char *));
    if (ctx->items == NULL) {
        zenit_bloom_destroy(ctx->bf);
        free(ctx);
        return NULL;
    }

    ctx->count = num_items;
    ctx->item_len = 16;
    ctx->index = 0;

    /* Generate distinct items — each is a 16-byte array with a unique prefix */
    for (size_t i = 0; i < num_items; i++) {
        ctx->items[i] = malloc(16);
        if (ctx->items[i] == NULL) {
            /* Free previously allocated items */
            for (size_t j = 0; j < i; j++) {
                free(ctx->items[j]);
            }
            free(ctx->items);
            zenit_bloom_destroy(ctx->bf);
            free(ctx);
            return NULL;
        }
        memset(ctx->items[i], (int)(i & 0xFF), 16);
        /* Break the repeat pattern with a unique suffix */
        ctx->items[i][8] = (unsigned char)(i >> 8);
        ctx->items[i][9] = (unsigned char)(i >> 16);
        ctx->items[i][10] = (unsigned char)(i >> 24);
    }

    return ctx;
}

static void ctx_teardown(bench_ctx_t *ctx) {
    if (ctx == NULL) return;
    for (size_t i = 0; i < ctx->count; i++) {
        free(ctx->items[i]);
    }
    free(ctx->items);
    zenit_bloom_destroy(ctx->bf);
    free(ctx);
}

/* Iterate index to cycle through items sequentially */
static void bench_insert_fn(void *vctx) {
    bench_ctx_t *ctx = (bench_ctx_t *)vctx;
    size_t i = ctx->index;
    ctx->index = (i + 1) % ctx->count;
    zenit_bloom_insert(ctx->bf, ctx->items[i], ctx->item_len);
}

/* For the contains-miss benchmark we need items that were never inserted */
typedef struct {
    zenit_bloom_t *bf;
    unsigned char **inserted;
    unsigned char **miss_items;
    size_t count;
    size_t item_len;
    size_t index;
} miss_ctx_t;

static miss_ctx_t* miss_setup(size_t num_items) {
    miss_ctx_t *ctx = malloc(sizeof(miss_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->bf = zenit_bloom_create(num_items, 0.01);
    if (ctx->bf == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->inserted = malloc(num_items * sizeof(unsigned char *));
    ctx->miss_items = malloc(num_items * sizeof(unsigned char *));
    if (ctx->inserted == NULL || ctx->miss_items == NULL) {
        free(ctx->inserted);
        free(ctx->miss_items);
        zenit_bloom_destroy(ctx->bf);
        free(ctx);
        return NULL;
    }

    ctx->count = num_items;
    ctx->item_len = 16;
    ctx->index = 0;

    /* Generate inserted items (0..num_items-1) and miss items (num_items..2*num_items-1) */
    for (size_t i = 0; i < num_items; i++) {
        ctx->inserted[i] = malloc(16);
        ctx->miss_items[i] = malloc(16);
        if (ctx->inserted[i] == NULL || ctx->miss_items[i] == NULL) {
            for (size_t j = 0; j < i; j++) {
                free(ctx->inserted[j]);
                free(ctx->miss_items[j]);
            }
            free(ctx->inserted);
            free(ctx->miss_items);
            zenit_bloom_destroy(ctx->bf);
            free(ctx);
            return NULL;
        }
        memset(ctx->inserted[i], (int)(i & 0xFF), 16);
        ctx->inserted[i][8] = (unsigned char)(i >> 8);
        ctx->inserted[i][9] = (unsigned char)(i >> 16);
        ctx->inserted[i][10] = (unsigned char)(i >> 24);

        /* Miss items use a different base offset */
        size_t miss_val = i + num_items;
        memset(ctx->miss_items[i], (int)(miss_val & 0xFF), 16);
        ctx->miss_items[i][8] = (unsigned char)(miss_val >> 8);
        ctx->miss_items[i][9] = (unsigned char)(miss_val >> 16);
        ctx->miss_items[i][10] = (unsigned char)(miss_val >> 24);

        /* Insert the item so we can benchmark contains-hit later */
        zenit_bloom_insert(ctx->bf, ctx->inserted[i], 16);
    }

    return ctx;
}

static void miss_teardown(miss_ctx_t *ctx) {
    if (ctx == NULL) return;
    for (size_t i = 0; i < ctx->count; i++) {
        free(ctx->inserted[i]);
        free(ctx->miss_items[i]);
    }
    free(ctx->inserted);
    free(ctx->miss_items);
    zenit_bloom_destroy(ctx->bf);
    free(ctx);
}

static void bench_contains_miss_fn(void *vctx) {
    miss_ctx_t *ctx = (miss_ctx_t *)vctx;
    size_t i = ctx->index;
    ctx->index = (i + 1) % ctx->count;
    (void)zenit_bloom_contains(ctx->bf, ctx->miss_items[i], ctx->item_len);
}

/* For contains-hit we query the inserted items (guaranteed hits) */
static void bench_contains_hit_from_miss_ctx_fn(void *vctx) {
    miss_ctx_t *ctx = (miss_ctx_t *)vctx;
    size_t i = ctx->index;
    ctx->index = (i + 1) % ctx->count;
    (void)zenit_bloom_contains(ctx->bf, ctx->inserted[i], ctx->item_len);
}

/* ---------------------------------------------------------------------------
 * Main — run all bloom-filter benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    size_t const N = 100000;

    /* Insert 100K items */
    {
        bench_ctx_t *ctx = ctx_setup(N);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "bloom_insert_100k", bench_insert_fn, ctx, N
            );
            zenit_bench_print(&r);
            ctx_teardown(ctx);
        }
    }

    /* Contains hit on 100K items (using a fresh pre-filled filter) */
    {
        miss_ctx_t *ctx = miss_setup(N);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "bloom_contains_hit_100k", bench_contains_hit_from_miss_ctx_fn, ctx, N
            );
            zenit_bench_print(&r);
            miss_teardown(ctx);
        }
    }

    /* Contains miss on 100K items */
    {
        miss_ctx_t *ctx = miss_setup(N);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "bloom_contains_miss_100k", bench_contains_miss_fn, ctx, N
            );
            zenit_bench_print(&r);
            miss_teardown(ctx);
        }
    }

    return 0;
}
