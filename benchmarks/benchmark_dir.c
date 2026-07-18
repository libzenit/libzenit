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
#include <libzenit/dir.h>
#include <libzenit/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BENCH_DIR "benchmark_dir_tmp"
#define BENCH_FILE_COUNT 10
#define BENCH_ITERATIONS 100

typedef struct {
    char dir[64];
} dir_list_ctx_t;

static dir_list_ctx_t* dir_list_setup(void) {
    dir_list_ctx_t *ctx = malloc(sizeof(dir_list_ctx_t));
    if (ctx == NULL) return NULL;

    snprintf(ctx->dir, sizeof(ctx->dir), "%s", BENCH_DIR);

    /* Create the benchmark directory */
    zenit_dir_create(ctx->dir);

    /* Populate with a few files */
    for (int i = 0; i < BENCH_FILE_COUNT; i++) {
        char path[128];
        snprintf(path, sizeof(path), "%s/file_%d.txt", ctx->dir, i);
        zenit_file_write(path, "data", 4);
    }

    return ctx;
}

static void dir_list_teardown(dir_list_ctx_t *ctx) {
    if (ctx == NULL) return;

    /* Remove files */
    for (int i = 0; i < BENCH_FILE_COUNT; i++) {
        char path[128];
        snprintf(path, sizeof(path), "%s/file_%d.txt", ctx->dir, i);
        zenit_file_delete(path);
    }

    /* Remove the directory */
    zenit_dir_remove(ctx->dir);
    free(ctx);
}

static void bench_dir_list_fn(void *ctx) {
    char **names = NULL;
    size_t count = 0;
    zenit_dir_list(((dir_list_ctx_t *)ctx)->dir, &names, &count);
    for (size_t i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}

int main(void) {
    dir_list_ctx_t *ctx = dir_list_setup();
    if (ctx == NULL) {
        fprintf(stderr, "setup failed\n");
        return 1;
    }

    zenit_bench_result_t r = zenit_bench_run("dir_list", bench_dir_list_fn, ctx, BENCH_ITERATIONS);
    zenit_bench_print(&r);

    dir_list_teardown(ctx);
    return 0;
}
