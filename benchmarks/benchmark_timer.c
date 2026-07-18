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
#include <libzenit/timer.h>
#include <stdlib.h>

typedef struct {
    int dummy;
} now_ctx_t;

static now_ctx_t* now_setup(void) {
    now_ctx_t *ctx = malloc(sizeof(now_ctx_t));
    if (ctx == NULL) return NULL;
    ctx->dummy = 0;
    return ctx;
}

static void now_teardown(now_ctx_t *ctx) {
    if (ctx == NULL) return;
    free(ctx);
}

static void bench_now_fn(void *ctx) {
    (void)ctx;
    zenit_time_now();
}

typedef struct {
    zenit_time_t start;
    zenit_time_t end;
} elapsed_ctx_t;

static elapsed_ctx_t* elapsed_setup(void) {
    elapsed_ctx_t *ctx = malloc(sizeof(elapsed_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->start = zenit_time_now();
    ctx->end = zenit_time_now();
    return ctx;
}

static void elapsed_teardown(elapsed_ctx_t *ctx) {
    if (ctx == NULL) return;
    free(ctx);
}

static void bench_elapsed_fn(void *ctx) {
    elapsed_ctx_t *c = (elapsed_ctx_t *)ctx;
    zenit_time_elapsed_ns(c->start, c->end);
}

int main(void) {
    {
        now_ctx_t *ctx = now_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("timer_now", bench_now_fn, ctx, 10000000);
            zenit_bench_print(&r);
            now_teardown(ctx);
        }
    }
    {
        elapsed_ctx_t *ctx = elapsed_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("timer_elapsed_ns", bench_elapsed_fn, ctx, 10000000);
            zenit_bench_print(&r);
            elapsed_teardown(ctx);
        }
    }
    return 0;
}
