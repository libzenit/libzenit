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
#include <libzenit/thread_pool.h>

static volatile int bench_counter = 0;

static void bench_task(void *ctx) {
    (void)ctx;
    __sync_fetch_and_add(&bench_counter, 1);
}

static void bench_enqueue(void *ctx) {
    zenit_thread_pool_t *pool = (zenit_thread_pool_t *)ctx;
    zenit_thread_pool_enqueue(pool, bench_task, NULL);
}

int main(void) {
    zenit_thread_pool_t *pool = zenit_thread_pool_create(4);

    zenit_bench_result_t r;

    r = zenit_bench_run("thread_pool_enqueue", bench_enqueue, pool, 10000);
    zenit_bench_print(&r);

    zenit_thread_pool_wait(pool);
    zenit_thread_pool_destroy(pool);
    return 0;
}
