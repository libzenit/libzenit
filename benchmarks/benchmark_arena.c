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
#include <libzenit/arena.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * Arena create / destroy — measures the overhead of pool construction
 * and teardown.  Each call both allocates and frees the entire arena.
 * -------------------------------------------------------------------------*/

static void bench_create_destroy_fn(void* ctx) {
    (void)ctx;
    zenit_arena_t* arena = zenit_arena_create(1024UL * 1024, 4096);
    if (arena != NULL) {
        zenit_arena_destroy(arena);
    }
}

/* ---------------------------------------------------------------------------
 * Acquire / release a usable arena from a pre-existing pool.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_arena_t* arena;
} acquire_ctx_t;

static acquire_ctx_t* acquire_setup(void) {
    acquire_ctx_t* ctx = malloc(sizeof(acquire_ctx_t));
    if (ctx == NULL) return NULL;
    ctx->arena = zenit_arena_create(1024UL * 1024, 4096);
    if (ctx->arena == NULL) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

static void acquire_teardown(acquire_ctx_t* ctx) {
    if (ctx == NULL) return;
    zenit_arena_destroy(ctx->arena);
    free(ctx);
}

static void bench_acquire_release_fn(void* ctx) {
    acquire_ctx_t* c = (acquire_ctx_t*)ctx;
    zenit_usable_arena_t* ua = zenit_arena_acquire(c->arena, 4096);
    if (ua != NULL) {
        zenit_arena_release(c->arena, ua);
    }
}

/* ---------------------------------------------------------------------------
 * Allocate / free a single buffer of a fixed size inside a usable arena.
 * The usable arena is acquired once during setup and released during teardown.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_arena_t* arena;
    zenit_usable_arena_t* ua;
    size_t alloc_size;
    int needs_reacquire; /* set to 1 after each free (always ready) */
} alloc_free_ctx_t;

/* Setup: create an arena, acquire a usable arena covering the whole pool. */
static alloc_free_ctx_t* alloc_free_setup(size_t arena_size, size_t block_size,
                                           size_t alloc_size) {
    alloc_free_ctx_t* ctx = malloc(sizeof(alloc_free_ctx_t));
    if (ctx == NULL) return NULL;

    ctx->arena = zenit_arena_create(arena_size, block_size);
    if (ctx->arena == NULL) {
        free(ctx);
        return NULL;
    }

    /* Acquire the entire pool so sub-allocations never run out of space */
    ctx->ua = zenit_arena_acquire(ctx->arena, arena_size);
    if (ctx->ua == NULL) {
        zenit_arena_destroy(ctx->arena);
        free(ctx);
        return NULL;
    }

    ctx->alloc_size = alloc_size;
    ctx->needs_reacquire = 0;
    return ctx;
}

static void alloc_free_teardown(alloc_free_ctx_t* ctx) {
    if (ctx == NULL) return;
    zenit_arena_release(ctx->arena, ctx->ua);
    zenit_arena_destroy(ctx->arena);
    free(ctx);
}

static void bench_alloc_free_fn(void* ctx) {
    alloc_free_ctx_t* c = (alloc_free_ctx_t*)ctx;
    /* Allocate a buffer of the configured size */
    zenit_usable_buffer_t buf = zenit_usable_arena_allocate(c->ua, c->alloc_size);
    /* Immediately free it — the next call will reuse the same block */
    zenit_usable_buffer_free(&buf);
}

/* ---------------------------------------------------------------------------
 * Malloc / free baseline for comparison (uses libc, not the arena).
 * -------------------------------------------------------------------------*/

static void bench_malloc_free_fn(void* ctx) {
    const size_t* size = (const size_t*)ctx;
    void* p = malloc(*size);
    if (p != NULL) {
        free(p);
    }
}

/* ---------------------------------------------------------------------------
 * Main — run all arena benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Create + destroy a 1 MB arena (4 KB blocks) */
    {
        zenit_bench_result_t r = zenit_bench_run(
            "arena_create_destroy", bench_create_destroy_fn, NULL, 500000
        );
        zenit_bench_print(&r);
    }

    /* Acquire + release a 4 KB usable region */
    {
        acquire_ctx_t* ctx = acquire_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "arena_acquire_release", bench_acquire_release_fn, ctx, 2000000
            );
            zenit_bench_print(&r);
            acquire_teardown(ctx);
        }
    }

    /* Allocate + free 8-byte buffers inside a usable arena */
    {
        alloc_free_ctx_t* ctx = alloc_free_setup(1024UL * 1024, 4096, 8);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "arena_alloc_free_8", bench_alloc_free_fn, ctx, 5000000
            );
            zenit_bench_print(&r);
            alloc_free_teardown(ctx);
        }
    }

    /* Allocate + free 64-byte buffers */
    {
        alloc_free_ctx_t* ctx = alloc_free_setup(1024UL * 1024, 4096, 64);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "arena_alloc_free_64", bench_alloc_free_fn, ctx, 5000000
            );
            zenit_bench_print(&r);
            alloc_free_teardown(ctx);
        }
    }

    /* Allocate + free 4096-byte buffers */
    {
        alloc_free_ctx_t* ctx = alloc_free_setup(1024UL * 1024, 4096, 4096);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "arena_alloc_free_4k", bench_alloc_free_fn, ctx, 500000
            );
            zenit_bench_print(&r);
            alloc_free_teardown(ctx);
        }
    }

    /* malloc + free baseline for 8-byte allocations */
    {
        size_t sz = 8;
        zenit_bench_result_t r = zenit_bench_run(
            "malloc_free_8", bench_malloc_free_fn, &sz, 5000000
        );
        zenit_bench_print(&r);
    }

    /* malloc + free baseline for 64-byte allocations */
    {
        size_t sz = 64;
        zenit_bench_result_t r = zenit_bench_run(
            "malloc_free_64", bench_malloc_free_fn, &sz, 5000000
        );
        zenit_bench_print(&r);
    }

    /* malloc + free baseline for 4096-byte allocations */
    {
        size_t sz = 4096;
        zenit_bench_result_t r = zenit_bench_run(
            "malloc_free_4k", bench_malloc_free_fn, &sz, 500000
        );
        zenit_bench_print(&r);
    }

    return 0;
}
