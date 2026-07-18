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
#include <libzenit/spsc.h>

static void bench_spsc_push(void *ctx) {
    zenit_spsc_t *s = (zenit_spsc_t *)ctx;
    int v = 42;
    zenit_spsc_push(s, &v);
    zenit_spsc_pop(s, &v);
}

static void bench_spsc_pop(void *ctx) {
    zenit_spsc_t *s = (zenit_spsc_t *)ctx;
    int v;
    zenit_spsc_pop(s, &v);
    /* Re-push for the next iteration */
    v = 42;
    zenit_spsc_push(s, &v);
}

int main(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 1024);
    /* Pre-fill with one element for pop benchmark */
    int v = 42;
    zenit_spsc_push(s, &v);

    zenit_bench_result_t r;

    r = zenit_bench_run("spsc_push_pop", bench_spsc_push, s, 1000000);
    zenit_bench_print(&r);

    r = zenit_bench_run("spsc_pop_push", bench_spsc_pop, s, 1000000);
    zenit_bench_print(&r);

    zenit_spsc_destroy(s);
    return 0;
}
