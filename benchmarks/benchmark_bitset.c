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
#include <libzenit/bitset.h>
#include <stdint.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * Set 100K bits sequentially.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_bitset_t *bs;
    size_t pos;
} bitset_set_ctx_t;

static bitset_set_ctx_t* bitset_set_setup(void) {
    bitset_set_ctx_t *ctx = malloc(sizeof(bitset_set_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->bs = zenit_bitset_create(100000);
    if (ctx->bs == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->pos = 0;
    return ctx;
}

static void bitset_set_teardown(bitset_set_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_bitset_destroy(ctx->bs);
    free(ctx);
}

static void bench_set_fn(void *ctx) {
    bitset_set_ctx_t *c = (bitset_set_ctx_t *)ctx;
    zenit_bitset_set(c->bs, c->pos);
    c->pos = (c->pos + 1) % 100000;
}

/* Escape hatch to prevent dead-code elimination — the compiler cannot see
 * across translation units, so writing to this pointer forces the result
 * to be materialised without volatile or atomic types. */
static void *bench_sink = NULL;

/* ---------------------------------------------------------------------------
 * Test 100K bits (hit — all bits are set first).
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_bitset_t *bs;
    size_t pos;
} bitset_test_ctx_t;

static bitset_test_ctx_t* bitset_test_setup(void) {
    bitset_test_ctx_t *ctx = malloc(sizeof(bitset_test_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->bs = zenit_bitset_create(100000);
    if (ctx->bs == NULL) {
        free(ctx);
        return NULL;
    }

    /* Pre-set all bits */
    zenit_bitset_set_all(ctx->bs);

    ctx->pos = 0;
    return ctx;
}

static void bitset_test_teardown(bitset_test_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_bitset_destroy(ctx->bs);
    free(ctx);
}

static void bench_test_fn(void *ctx) {
    bitset_test_ctx_t *c = (bitset_test_ctx_t *)ctx;
    int v = zenit_bitset_test(c->bs, c->pos);
    /* Escape the result so the compiler cannot optimise the call away */
    bench_sink = (void*)(uintptr_t)v;
    c->pos = (c->pos + 1) % 100000;
}

/* ---------------------------------------------------------------------------
 * Count 100K bits.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_bitset_t *bs;
} bitset_count_ctx_t;

static bitset_count_ctx_t* bitset_count_setup(void) {
    bitset_count_ctx_t *ctx = malloc(sizeof(bitset_count_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->bs = zenit_bitset_create(100000);
    if (ctx->bs == NULL) {
        free(ctx);
        return NULL;
    }

    /* Set every other bit so count is 50000 */
    for (size_t i = 0; i < 100000; i += 2) {
        zenit_bitset_set(ctx->bs, i);
    }

    return ctx;
}

static void bitset_count_teardown(bitset_count_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_bitset_destroy(ctx->bs);
    free(ctx);
}

static void bench_count_fn(void *ctx) {
    size_t c = zenit_bitset_count(((bitset_count_ctx_t *)ctx)->bs);
    /* Escape the result so the compiler cannot optimise the call away */
    bench_sink = (void*)c;
}

/* ---------------------------------------------------------------------------
 * Main — run all bitset benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Set 100K bits sequentially */
    {
        bitset_set_ctx_t *ctx = bitset_set_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "bitset_set", bench_set_fn, ctx, 100000
            );
            zenit_bench_print(&r);
            bitset_set_teardown(ctx);
        }
    }

    /* Test 100K bits (hit) */
    {
        bitset_test_ctx_t *ctx = bitset_test_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "bitset_test", bench_test_fn, ctx, 100000
            );
            zenit_bench_print(&r);
            bitset_test_teardown(ctx);
        }
    }

    /* Count 100K bits */
    {
        bitset_count_ctx_t *ctx = bitset_count_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "bitset_count", bench_count_fn, ctx, 100000
            );
            zenit_bench_print(&r);
            bitset_count_teardown(ctx);
        }
    }

    return 0;
}
