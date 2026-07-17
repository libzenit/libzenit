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
#include <libzenit/string.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * append_cstr: 100K small (8-byte) strings
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_string_t *str;
    char buf[8];
} append_cstr_ctx_t;

static append_cstr_ctx_t* append_cstr_setup(void) {
    append_cstr_ctx_t *ctx = malloc(sizeof(append_cstr_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->str = zenit_string_create();
    if (ctx->str == NULL) {
        free(ctx);
        return NULL;
    }

    memset(ctx->buf, 'x', 7);
    ctx->buf[7] = '\0';
    return ctx;
}

static void append_cstr_teardown(append_cstr_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_string_destroy(ctx->str);
    free(ctx);
}

static void bench_append_cstr_fn(void *ctx) {
    append_cstr_ctx_t *c = (append_cstr_ctx_t *)ctx;
    /* Reset string each iteration so we don't grow unboundedly */
    /* Actually, the benchmark runs iterations sequentially on one ctx,
       so we want to measure steady-state throughput.  We grow the
       string by appending each time. */
    zenit_string_append_cstr(c->str, c->buf);
}

/* ---------------------------------------------------------------------------
 * append (raw): 100K medium (64-byte) strings
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_string_t *str;
    char buf[64];
} append_ctx_t;

static append_ctx_t* append_setup(void) {
    append_ctx_t *ctx = malloc(sizeof(append_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->str = zenit_string_create();
    if (ctx->str == NULL) {
        free(ctx);
        return NULL;
    }

    memset(ctx->buf, 'A', 64);
    return ctx;
}

static void append_teardown(append_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_string_destroy(ctx->str);
    free(ctx);
}

static void bench_append_fn(void *ctx) {
    append_ctx_t *c = (append_ctx_t *)ctx;
    zenit_string_append(c->str, c->buf, 64);
}

/* ---------------------------------------------------------------------------
 * Main — run all string benchmarks
 * -------------------------------------------------------------------------*/

int main(void) {
    /* append_cstr — 100K small (8-byte) strings */
    {
        append_cstr_ctx_t *ctx = append_cstr_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "string_append_cstr_8B", bench_append_cstr_fn, ctx, 100000
            );
            zenit_bench_print(&r);
            append_cstr_teardown(ctx);
        }
    }

    /* append (raw) — 100K medium (64-byte) strings */
    {
        append_ctx_t *ctx = append_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "string_append_64B", bench_append_fn, ctx, 100000
            );
            zenit_bench_print(&r);
            append_teardown(ctx);
        }
    }

    return 0;
}
