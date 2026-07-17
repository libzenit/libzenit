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

#include <libzenit/list.h>
#include <libzenit/benchmark.h>
#include <stdint.h>
#include <stdlib.h>

/* ─── Bench: push_back N elements ─── */
static void bench_push_back(void *ctx) {
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_list_push_back(list, &i);
    }
    zenit_list_destroy(list);
}

/* ─── Bench: push_front N elements ─── */
static void bench_push_front(void *ctx) {
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_list_push_front(list, &i);
    }
    zenit_list_destroy(list);
}

/* ─── Bench: push_back then pop_front N ─── */
static void bench_push_pop(void *ctx) {
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_list_push_back(list, &i);
    }
    int out;
    for (int i = 0; i < n; i++) {
        zenit_list_pop_front(list, &out);
    }
    zenit_list_destroy(list);
}

/* ─── Bench: foreach over N elements ─── */
static void foreach_noop(const void *elem, void *ctx) {
    (void)elem;
    (void)ctx;
}

static void bench_foreach(void *ctx) {
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int n = (int)(intptr_t)ctx;
    for (int i = 0; i < n; i++) {
        zenit_list_push_back(list, &i);
    }
    zenit_list_foreach(list, foreach_noop, NULL);
    zenit_list_destroy(list);
}

int main(void) {
    int n = 100000;

    zenit_bench_result_t r;

    r = zenit_bench_run("list_push_back_100K", bench_push_back, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    r = zenit_bench_run("list_push_front_100K", bench_push_front, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    r = zenit_bench_run("list_push_pop_100K", bench_push_pop, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    r = zenit_bench_run("list_foreach_100K", bench_foreach, (void *)(intptr_t)n, 100);
    zenit_bench_print(&r);

    return 0;
}
