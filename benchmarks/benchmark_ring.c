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
#include <libzenit/ring.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Sequential push/pop: push N bytes, pop N bytes, repeat.
 * Measures raw throughput without wrap-around.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_ring_t *ring;
    unsigned char *data;
    size_t chunk_size;
} seq_ctx_t;

static seq_ctx_t* seq_setup(size_t capacity, size_t chunk_size) {
    seq_ctx_t *ctx = malloc(sizeof(seq_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->ring = zenit_ring_create(capacity);
    if (ctx->ring == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->data = malloc(chunk_size);
    if (ctx->data == NULL) {
        zenit_ring_destroy(ctx->ring);
        free(ctx);
        return NULL;
    }
    memset(ctx->data, 0xAB, chunk_size);

    ctx->chunk_size = chunk_size;
    return ctx;
}

static void seq_teardown(seq_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_ring_destroy(ctx->ring);
    free(ctx->data);
    free(ctx);
}

static void bench_seq_fn(void *ctx) {
    seq_ctx_t *c = (seq_ctx_t *)ctx;
    /* One unit of work: push + pop a chunk */
    size_t sz = c->chunk_size;
    zenit_ring_push(c->ring, c->data, c->chunk_size);
    zenit_ring_pop(c->ring, c->data, &sz);
}

/* ---------------------------------------------------------------------------
 * Miss: push to full then attempt another push (always fails).
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_ring_t *ring;
    unsigned char byte;
} full_ctx_t;

static full_ctx_t* full_setup(void) {
    full_ctx_t *ctx = malloc(sizeof(full_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->ring = zenit_ring_create(1);
    if (ctx->ring == NULL) {
        free(ctx);
        return NULL;
    }

    /* Fill the single slot */
    ctx->byte = 0x42;
    zenit_ring_push(ctx->ring, &ctx->byte, 1);

    return ctx;
}

static void full_teardown(full_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_ring_destroy(ctx->ring);
    free(ctx);
}

static void bench_full_fn(void *ctx) {
    unsigned char b = 0xFF;
    zenit_ring_push(((full_ctx_t *)ctx)->ring, &b, 1);
}

/* ---------------------------------------------------------------------------
 * Main — run all ring-buffer benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Sequential: 128-byte chunks in a 1 KB ring */
    {
        seq_ctx_t *ctx = seq_setup(1024, 128);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "ring_seq_128", bench_seq_fn, ctx, 500000
            );
            zenit_bench_print(&r);
            seq_teardown(ctx);
        }
    }

    /* Sequential: 1 KB chunks in a 1 MB ring */
    {
        seq_ctx_t *ctx = seq_setup(1024UL * 1024, 1024);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "ring_seq_1k", bench_seq_fn, ctx, 100000
            );
            zenit_bench_print(&r);
            seq_teardown(ctx);
        }
    }

    /* Full-miss: always-invalid push on a full buffer */
    {
        full_ctx_t *ctx = full_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "ring_full_miss", bench_full_fn, ctx, 10000000
            );
            zenit_bench_print(&r);
            full_teardown(ctx);
        }
    }

    return 0;
}
