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
#include <libzenit/queue.h>
#include <stdlib.h>

typedef struct {
    zenit_queue_t *queue;
    int value;
} enqueue_ctx_t;

static enqueue_ctx_t* enqueue_setup(void) {
    enqueue_ctx_t *ctx = malloc(sizeof(enqueue_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->queue = zenit_queue_create(sizeof(int));
    if (ctx->queue == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->value = 42;
    return ctx;
}

static void enqueue_teardown(enqueue_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_queue_destroy(ctx->queue);
    free(ctx);
}

static void bench_enqueue_fn(void *ctx) {
    zenit_queue_enqueue(((enqueue_ctx_t *)ctx)->queue, &((enqueue_ctx_t *)ctx)->value);
}

typedef struct {
    zenit_queue_t *queue;
    int value;
} enq_deq_ctx_t;

static enq_deq_ctx_t* enq_deq_setup(void) {
    enq_deq_ctx_t *ctx = malloc(sizeof(enq_deq_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->queue = zenit_queue_create(sizeof(int));
    if (ctx->queue == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->value = 42;
    return ctx;
}

static void enq_deq_teardown(enq_deq_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_queue_destroy(ctx->queue);
    free(ctx);
}

static void bench_enq_deq_fn(void *ctx) {
    enq_deq_ctx_t *c = (enq_deq_ctx_t *)ctx;
    zenit_queue_enqueue(c->queue, &c->value);
    zenit_queue_dequeue(c->queue, &c->value);
}

typedef struct {
    zenit_queue_t *queue;
    int value;
} peek_ctx_t;

static peek_ctx_t* peek_setup(void) {
    peek_ctx_t *ctx = malloc(sizeof(peek_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->queue = zenit_queue_create(sizeof(int));
    if (ctx->queue == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->value = 42;
    zenit_queue_enqueue(ctx->queue, &ctx->value);
    return ctx;
}

static void peek_teardown(peek_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_queue_destroy(ctx->queue);
    free(ctx);
}

static void bench_peek_fn(void *ctx) {
    zenit_queue_peek(((peek_ctx_t *)ctx)->queue);
}

int main(void) {
    {
        enqueue_ctx_t *ctx = enqueue_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("queue_enqueue", bench_enqueue_fn, ctx, 1000000);
            zenit_bench_print(&r);
            enqueue_teardown(ctx);
        }
    }
    {
        enq_deq_ctx_t *ctx = enq_deq_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("queue_enqueue_dequeue", bench_enq_deq_fn, ctx, 1000000);
            zenit_bench_print(&r);
            enq_deq_teardown(ctx);
        }
    }
    {
        peek_ctx_t *ctx = peek_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("queue_peek", bench_peek_fn, ctx, 1000000);
            zenit_bench_print(&r);
            peek_teardown(ctx);
        }
    }
    return 0;
}
