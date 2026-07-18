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
#include <libzenit/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IO_WRITE_COUNT 100000
#define IO_CHUNK_SIZE 1024

typedef struct {
    unsigned char data[IO_CHUNK_SIZE];
    int count;
} write_ctx_t;

static write_ctx_t* write_setup(void) {
    write_ctx_t *ctx = malloc(sizeof(write_ctx_t));
    if (ctx == NULL) return NULL;

    memset(ctx->data, 0xAB, IO_CHUNK_SIZE);
    ctx->count = 0;
    return ctx;
}

static void write_teardown(write_ctx_t *ctx) {
    if (ctx == NULL) return;
    free(ctx);
}

static void bench_write_fn(void *ctx) {
    write_ctx_t *c = (write_ctx_t *)ctx;
    char path[64];
    snprintf(path, sizeof(path), "bench_io_write_%d.tmp", c->count);
    zenit_file_write(path, c->data, IO_CHUNK_SIZE);
    c->count++;
}

typedef struct {
    char path[64];
} read_ctx_t;

static read_ctx_t* read_setup(void) {
    read_ctx_t *ctx = malloc(sizeof(read_ctx_t));
    if (ctx == NULL) return NULL;

    snprintf(ctx->path, sizeof(ctx->path), "bench_io_read.tmp");
    unsigned char data[IO_CHUNK_SIZE];
    memset(data, 0xAB, IO_CHUNK_SIZE);
    zenit_file_write(ctx->path, data, IO_CHUNK_SIZE);
    return ctx;
}

static void read_teardown(read_ctx_t *ctx) {
    if (ctx == NULL) return;
    zenit_file_delete(ctx->path);
    free(ctx);
}

static void bench_read_fn(void *ctx) {
    void *data = NULL;
    size_t len = 0;
    zenit_file_read(((read_ctx_t *)ctx)->path, &data, &len);
    free(data);
}

static void cleanup_write_files(void) {
    for (int i = 0; i < IO_WRITE_COUNT; i++) {
        char path[64];
        snprintf(path, sizeof(path), "bench_io_write_%d.tmp", i);
        zenit_file_delete(path);
    }
}

int main(void) {
    {
        write_ctx_t *ctx = write_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("io_write_1k", bench_write_fn, ctx, IO_WRITE_COUNT);
            zenit_bench_print(&r);
            write_teardown(ctx);
        }
    }

    cleanup_write_files();

    {
        read_ctx_t *ctx = read_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run("io_read_1k", bench_read_fn, ctx, IO_WRITE_COUNT);
            zenit_bench_print(&r);
            read_teardown(ctx);
        }
    }
    return 0;
}
