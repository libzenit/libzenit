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
#include <libzenit/stack.h>
#include <stdlib.h>

typedef struct {
    zenit_stack_t *stack;
    int value;
} push_ctx_t;

static push_ctx_t* push_setup(void) {
    push_ctx_t *ctx = malloc(sizeof(push_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->stack = zenit_stack_create(sizeof(int));
    if (ctx->stack == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->value = 42;
    return ctx;
}

static void push_teardown(push_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_stack_destroy(ctx->stack);
    free(ctx);
}

static void bench_push_fn(void *ctx) {
    zenit_stack_push(((push_ctx_t *)ctx)->stack, &((push_ctx_t *)ctx)->value);
}

typedef struct {
    zenit_stack_t *stack;
    int value;
} push_pop_ctx_t;

static push_pop_ctx_t* push_pop_setup(void) {
    push_pop_ctx_t *ctx = malloc(sizeof(push_pop_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->stack = zenit_stack_create(sizeof(int));
    if (ctx->stack == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->value = 42;
    return ctx;
}

static void push_pop_teardown(push_pop_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_stack_destroy(ctx->stack);
    free(ctx);
}

static void bench_push_pop_fn(void *ctx) {
    push_pop_ctx_t *c = (push_pop_ctx_t *)ctx;
    zenit_stack_push(c->stack, &c->value);
    zenit_stack_pop(c->stack, &c->value);
}

typedef struct {
    zenit_stack_t *stack;
    int value;
} peek_ctx_t;

static peek_ctx_t* peek_setup(void) {
    peek_ctx_t *ctx = malloc(sizeof(peek_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->stack = zenit_stack_create(sizeof(int));
    if (ctx->stack == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->value = 42;
    zenit_stack_push(ctx->stack, &ctx->value);
    return ctx;
}

static void peek_teardown(peek_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_stack_destroy(ctx->stack);
    free(ctx);
}

static void bench_peek_fn(void *ctx) {
    zenit_stack_peek(((peek_ctx_t *)ctx)->stack);
}

int main(void) {
    {
        push_ctx_t *ctx = push_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("stack_push", bench_push_fn, ctx, 1000000);
            zenit_bench_print(&r);
            push_teardown(ctx);
        }
    }
    {
        push_pop_ctx_t *ctx = push_pop_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("stack_push_pop", bench_push_pop_fn, ctx, 1000000);
            zenit_bench_print(&r);
            push_pop_teardown(ctx);
        }
    }
    {
        peek_ctx_t *ctx = peek_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("stack_peek", bench_peek_fn, ctx, 1000000);
            zenit_bench_print(&r);
            peek_teardown(ctx);
        }
    }
    return 0;
}
