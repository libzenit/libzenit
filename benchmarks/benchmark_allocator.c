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
#include <libzenit/allocator.h>
#include <stdlib.h>

typedef struct {
    size_t size;
    zenit_allocator_t a;
} bench_ctx_t;

static void bench_malloc_free(void *ctx) {
    bench_ctx_t *c = (bench_ctx_t *)ctx;
    void *p = malloc(c->size);
    if (p) free(p);
}

static void bench_zenit_alloc_free(void *ctx) {
    bench_ctx_t *c = (bench_ctx_t *)ctx;
    void *p = c->a.alloc_fn(c->size, c->a.ctx);
    if (p) c->a.free_fn(p, c->a.ctx);
}

static void bench_realloc_fallback(void *ctx) {
    bench_ctx_t *c = (bench_ctx_t *)ctx;
    void *p = c->a.alloc_fn(c->size, c->a.ctx);
    if (p) {
        void *q = zenit_allocator_realloc(c->a, p, c->size, c->size * 2);
        if (q) {
            c->a.free_fn(q, c->a.ctx);
        }
    }
}

int main(void) {
    zenit_allocator_t fallback_a = {
        .alloc_fn = zenit_default_alloc,
        .realloc_fn = NULL,
        .free_fn = zenit_default_free,
        .ctx = NULL
    };

    bench_ctx_t ac = { .size = 64, .a = ZENIT_ALLOCATOR_DEFAULT };
    bench_ctx_t fc = { .size = 64, .a = fallback_a };

    zenit_bench_result_t r;

    r = zenit_bench_run("malloc_free_64", bench_malloc_free, &ac, 1000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("zenit_alloc_free_64", bench_zenit_alloc_free, &ac, 1000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("realloc_fallback_64", bench_realloc_fallback, &fc, 100000);
    zenit_bench_print(&r);

    return 0;
}
