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

#include <libzenit/heap.h>
#include <string.h>

#include "test_runner.h"

/* ─── Max-heap comparator (standard: a > b → positive) ─── */
static int cmp_int_max(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (ia > ib) return 1;
    if (ia < ib) return -1;
    return 0;
}

/* ─── Min-heap comparator ─── */
static int cmp_int_min(const void *lhs, const void *rhs) {
    return cmp_int_max(rhs, lhs);
}

/* ─── Test: create and destroy ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    ASSERT(heap != NULL, "expected non-NULL");
    ASSERT(zenit_heap_count(heap) == 0, "count should be 0");
    ASSERT(zenit_heap_empty(heap), "should be empty");
    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: create with invalid params returns NULL ─── */
static int test_create_invalid(void) {
    TEST("create invalid params");
    ASSERT(zenit_heap_create(0, cmp_int_max) == NULL, "zero elem_size");
    ASSERT(zenit_heap_create(sizeof(int), NULL) == NULL, "NULL compare");
    ASSERT(zenit_heap_create_with_capacity(sizeof(int), NULL, 8) == NULL, "NULL compare w/ cap");
    ASSERT(zenit_heap_create_with_capacity(sizeof(int), cmp_int_max, 0) == NULL, "zero capacity");
    PASS();
    return 0;
}

/* ─── Test: destroy NULL is safe ─── */
static int test_destroy_null(void) {
    TEST("destroy NULL is safe");
    zenit_heap_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: push and pop in max-heap order ─── */
static int test_push_pop_max(void) {
    TEST("push/pop max-heap");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    ASSERT(heap != NULL, "expected non-NULL");

    int vals[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int sorted[] = {9, 6, 5, 5, 4, 3, 3, 2, 1, 1};
    size_t n = sizeof(vals) / sizeof(vals[0]);

    for (size_t i = 0; i < n; i++) {
        ASSERT(zenit_heap_push(heap, &vals[i]).error == ZENIT_OK, "push failed");
    }
    ASSERT(zenit_heap_count(heap) == n, "count mismatch");

    for (size_t i = 0; i < n; i++) {
        int out;
        ASSERT(zenit_heap_pop(heap, &out).error == ZENIT_OK, "pop failed");
        ASSERT(out == sorted[i], "unexpected pop order");
    }
    ASSERT(zenit_heap_count(heap) == 0, "should be empty after pop");

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: push and pop in min-heap order ─── */
static int test_push_pop_min(void) {
    TEST("push/pop min-heap");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_min);
    ASSERT(heap != NULL, "expected non-NULL");

    int vals[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int sorted[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};
    size_t n = sizeof(vals) / sizeof(vals[0]);

    for (size_t i = 0; i < n; i++) {
        zenit_heap_push(heap, &vals[i]);
    }

    for (size_t i = 0; i < n; i++) {
        int out;
        zenit_heap_pop(heap, &out);
        ASSERT(out == sorted[i], "unexpected min-pop order");
    }

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: peek returns root without removing ─── */
static int test_peek(void) {
    TEST("peek");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    ASSERT(zenit_heap_peek(heap) == NULL, "peek on empty");

    int v = 42;
    zenit_heap_push(heap, &v);
    ASSERT(*(int *)zenit_heap_peek(heap) == 42, "peek after push");
    ASSERT(zenit_heap_count(heap) == 1, "peek should not remove");

    ASSERT(zenit_heap_peek(NULL) == NULL, "peek on NULL");

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: pop on empty returns error ─── */
static int test_pop_empty(void) {
    TEST("pop on empty returns error");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    int out;
    ASSERT(zenit_heap_pop(heap, &out).error == ZENIT_ERROR_EMPTY, "expected EMPTY");
    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: push/pop with NULL params ─── */
static int test_null_params(void) {
    TEST("NULL params return error");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    int v = 0;
    int out;

    ASSERT(zenit_heap_push(NULL, &v).error == ZENIT_ERROR_NULL, "push NULL list");
    ASSERT(zenit_heap_push(heap, NULL).error == ZENIT_ERROR_NULL, "push NULL elem");
    ASSERT(zenit_heap_pop(NULL, &out).error == ZENIT_ERROR_NULL, "pop NULL list");
    ASSERT(zenit_heap_pop(heap, NULL).error == ZENIT_ERROR_NULL, "pop NULL out");
    ASSERT(zenit_heap_reserve(NULL, 10).error == ZENIT_ERROR_NULL, "reserve NULL");

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: reserve ─── */
static int test_reserve(void) {
    TEST("reserve");
    zenit_heap_t *heap = zenit_heap_create_with_capacity(sizeof(int), cmp_int_max, 4);
    ASSERT(heap != NULL, "expected non-NULL");
    ASSERT(zenit_heap_capacity(heap) == 4, "initial capacity 4");

    ASSERT(zenit_heap_reserve(heap, 10).error == ZENIT_OK, "reserve to 10");
    ASSERT(zenit_heap_capacity(heap) >= 10, "capacity >= 10");

    /* No-op reserve */
    ASSERT(zenit_heap_reserve(heap, 5).error == ZENIT_OK, "reserve smaller no-op");
    ASSERT(zenit_heap_capacity(heap) >= 10, "capacity unchanged");

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    TEST("clear");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    int vals[] = {5, 3, 8, 1};
    for (size_t i = 0; i < 4; i++) {
        zenit_heap_push(heap, &vals[i]);
    }
    ASSERT(zenit_heap_count(heap) == 4, "count before clear");

    zenit_heap_clear(heap);
    ASSERT(zenit_heap_count(heap) == 0, "count after clear");
    ASSERT(zenit_heap_empty(heap), "empty after clear");

    zenit_heap_clear(NULL);
    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: many elements ─── */
static int test_many_elements(void) {
    TEST("many elements");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    int n = 10000;

    for (int i = 0; i < n; i++) {
        zenit_heap_push(heap, &i);
    }
    ASSERT(zenit_heap_count(heap) == (size_t)n, "count matches");

    /* Verify descending order */
    int prev = n;
    for (int i = 0; i < n; i++) {
        int out;
        zenit_heap_pop(heap, &out);
        ASSERT(out <= prev, "should be descending");
        prev = out;
    }

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: struct elements ─── */
typedef struct {
    int priority;
    char data[16];
} heap_item_t;

static int cmp_item(const void *a, const void *b) {
    int pa = ((const heap_item_t *)a)->priority;
    int pb = ((const heap_item_t *)b)->priority;
    if (pa > pb) return 1;
    if (pa < pb) return -1;
    return 0;
}

static int test_struct(void) {
    TEST("struct elements");
    zenit_heap_t *heap = zenit_heap_create(sizeof(heap_item_t), cmp_item);
    ASSERT(heap != NULL, "expected non-NULL");

    heap_item_t items[] = {
        {3, "three"},
        {1, "one"},
        {2, "two"},
    };
    for (size_t i = 0; i < 3; i++) {
        zenit_heap_push(heap, &items[i]);
    }

    heap_item_t out;
    zenit_heap_pop(heap, &out);
    ASSERT(out.priority == 3, "expected priority 3");
    ASSERT(strcmp(out.data, "three") == 0, "expected 'three'");

    zenit_heap_pop(heap, &out);
    ASSERT(out.priority == 2, "expected priority 2");

    zenit_heap_pop(heap, &out);
    ASSERT(out.priority == 1, "expected priority 1");

    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

/* ─── Test: count/capacity/empty on NULL ─── */
static int test_query_null(void) {
    TEST("query functions on NULL");
    ASSERT(zenit_heap_count(NULL) == 0, "count on NULL");
    ASSERT(zenit_heap_capacity(NULL) == 0, "capacity on NULL");
    ASSERT(zenit_heap_empty(NULL) == 1, "empty on NULL");
    PASS();
    return 0;
}

/* ─── Test: heap build ─── */
static int test_heap_build(void) {
    TEST("heap_build");
    zenit_heap_t *heap = zenit_heap_create(sizeof(int), cmp_int_max);
    ASSERT(heap != NULL, "create");
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
    zenit_result_t r = zenit_heap_build(heap, arr, 8);
    ASSERT(r.error == ZENIT_OK, "build ok");
    ASSERT(zenit_heap_count(heap) == 8, "8 elements");
    int root = *(int*)zenit_heap_peek(heap);
    ASSERT(root == 9, "root is max");
    /* NULL params */
    ASSERT(zenit_heap_build(NULL, arr, 8).error == ZENIT_ERROR_NULL, "NULL heap");
    ASSERT(zenit_heap_build(heap, NULL, 8).error == ZENIT_ERROR_NULL, "NULL array");
    /* Build empty */
    ASSERT(zenit_heap_build(heap, arr, 0).error == ZENIT_OK, "empty build");
    ASSERT(zenit_heap_count(heap) == 0, "0 elements after empty build");
    zenit_heap_destroy(heap);
    PASS();
    return 0;
}

int main(void) {
    TEST_ENTRY tests[] = {
        { test_create_destroy,     "create_destroy" },
        { test_create_invalid,     "create_invalid" },
        { test_destroy_null,       "destroy_null" },
        { test_push_pop_max,       "push_pop_max" },
        { test_push_pop_min,       "push_pop_min" },
        { test_peek,               "peek" },
        { test_pop_empty,          "pop_empty" },
        { test_null_params,        "null_params" },
        { test_reserve,            "reserve" },
        { test_clear,              "clear" },
        { test_many_elements,      "many_elements" },
        { test_struct,             "struct" },
        { test_query_null,         "query_null" },
        { test_heap_build,         "heap_build" },
        { 0, 0 }
    };
    return test_run_all("test_heap", tests);
}
