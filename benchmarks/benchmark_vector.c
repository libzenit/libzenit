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
#include <libzenit/vector.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * Sequential push: push N ints into a vector.
 * Measures raw append throughput.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_vector_t *vector;
    int value;
} push_ctx_t;

static push_ctx_t* push_setup(void) {
    push_ctx_t *ctx = malloc(sizeof(push_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->vector = zenit_vector_create(sizeof(int));
    if (ctx->vector == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->value = 42;
    return ctx;
}

static void push_teardown(push_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_vector_destroy(ctx->vector);
    free(ctx);
}

static void bench_push_fn(void *ctx) {
    zenit_vector_push(((push_ctx_t *)ctx)->vector, &((push_ctx_t *)ctx)->value);
}

/* ---------------------------------------------------------------------------
 * Sequential push/pop: push then pop, back and forth.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_vector_t *vector;
    int value;
    int toggle;
} push_pop_ctx_t;

static push_pop_ctx_t* push_pop_setup(void) {
    push_pop_ctx_t *ctx = malloc(sizeof(push_pop_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->vector = zenit_vector_create(sizeof(int));
    if (ctx->vector == NULL) {
        free(ctx);
        return NULL;
    }

    ctx->value = 42;
    ctx->toggle = 0;
    return ctx;
}

static void push_pop_teardown(push_pop_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_vector_destroy(ctx->vector);
    free(ctx);
}

static void bench_push_pop_fn(void *ctx) {
    push_pop_ctx_t *c = (push_pop_ctx_t *)ctx;
    c->toggle = !c->toggle;
    if (c->toggle) {
        zenit_vector_push(c->vector, &c->value);
    } else {
        zenit_vector_pop(c->vector, &c->value);
    }
}

/* ---------------------------------------------------------------------------
 * Insert front (worst case O(n) shift).
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_vector_t *vector;
    int value;
} insert_front_ctx_t;

static insert_front_ctx_t* insert_front_setup(void) {
    insert_front_ctx_t *ctx = malloc(sizeof(insert_front_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->vector = zenit_vector_create_with_capacity(sizeof(int), 64);
    if (ctx->vector == NULL) {
        free(ctx);
        return NULL;
    }

    /* Pre-fill with 32 elements so shifts are meaningful */
    for (int i = 0; i < 32; i++) {
        zenit_vector_push(ctx->vector, &i);
    }

    ctx->value = 99;
    return ctx;
}

static void insert_front_teardown(insert_front_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_vector_destroy(ctx->vector);
    free(ctx);
}

static void bench_insert_front_fn(void *ctx) {
    zenit_vector_insert(((insert_front_ctx_t *)ctx)->vector, 0,
                        &((insert_front_ctx_t *)ctx)->value);
}

/* ---------------------------------------------------------------------------
 * Reserve then push: pre-allocate then fill.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_vector_t *vector;
    int value;
} reserve_push_ctx_t;

static reserve_push_ctx_t* reserve_push_setup(void) {
    reserve_push_ctx_t *ctx = malloc(sizeof(reserve_push_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->vector = zenit_vector_create(sizeof(int));
    if (ctx->vector == NULL) {
        free(ctx);
        return NULL;
    }

    /* Pre-allocate 10000 slots */
    zenit_vector_reserve(ctx->vector, 10000);

    ctx->value = 42;
    return ctx;
}

static void reserve_push_teardown(reserve_push_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_vector_destroy(ctx->vector);
    free(ctx);
}

static void bench_reserve_push_fn(void *ctx) {
    zenit_vector_push(((reserve_push_ctx_t *)ctx)->vector,
                      &((reserve_push_ctx_t *)ctx)->value);
}

/* ---------------------------------------------------------------------------
 * Main — run all vector benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Sequential push — 1M elements */
    {
        push_ctx_t *ctx = push_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "vector_push_seq", bench_push_fn, ctx, 1000000
            );
            zenit_bench_print(&r);
            push_teardown(ctx);
        }
    }

    /* Sequential push/pop — 500K pairs */
    {
        push_pop_ctx_t *ctx = push_pop_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "vector_push_pop", bench_push_pop_fn, ctx, 1000000
            );
            zenit_bench_print(&r);
            push_pop_teardown(ctx);
        }
    }

    /* Insert front — 1000 inserts (O(n) each) */
    {
        insert_front_ctx_t *ctx = insert_front_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "vector_insert_front", bench_insert_front_fn, ctx, 10000
            );
            zenit_bench_print(&r);
            insert_front_teardown(ctx);
        }
    }

    /* Reserve then push — 1M elements, no growth overhead */
    {
        reserve_push_ctx_t *ctx = reserve_push_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "vector_reserve_push", bench_reserve_push_fn, ctx, 1000000
            );
            zenit_bench_print(&r);
            reserve_push_teardown(ctx);
        }
    }

    return 0;
}
