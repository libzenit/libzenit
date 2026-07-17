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

#include <libzenit/deque.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Helpers ─── */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while (0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); tests_failed++; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return 1; } } while (0)

/* ─── Test: create and destroy ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    ASSERT(d != NULL, "expected non-NULL");
    ASSERT(zenit_deque_count(d) == 0, "count should be 0");
    ASSERT(zenit_deque_empty(d), "should be empty");
    ASSERT(zenit_deque_front(d) == NULL, "front should be NULL");
    ASSERT(zenit_deque_back(d) == NULL, "back should be NULL");
    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: create with invalid params returns NULL ─── */
static int test_create_invalid(void) {
    TEST("create invalid params");
    ASSERT(zenit_deque_create(0) == NULL, "zero elem_size");
    ASSERT(zenit_deque_create_with_capacity(sizeof(int), 0) == NULL, "zero capacity");
    ASSERT(zenit_deque_create_with_capacity(0, 8) == NULL, "zero elem_size w/ cap");
    PASS();
    return 0;
}

/* ─── Test: destroy NULL is safe ─── */
static int test_destroy_null(void) {
    TEST("destroy NULL is safe");
    zenit_deque_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: push_back / pop_front (FIFO) ─── */
static int test_push_back_pop_front(void) {
    TEST("push_back/pop_front");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    ASSERT(d != NULL, "expected non-NULL");

    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        zenit_deque_push_back(d, &vals[i]);
    }
    ASSERT(zenit_deque_count(d) == 5, "count should be 5");

    for (int i = 0; i < 5; i++) {
        int out;
        ASSERT(zenit_deque_pop_front(d, &out).error == ZENIT_OK, "pop_front failed");
        ASSERT(out == vals[i], "FIFO order broken");
    }
    ASSERT(zenit_deque_count(d) == 0, "count should be 0");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: push_front / pop_back (reverse FIFO) ─── */
static int test_push_front_pop_back(void) {
    TEST("push_front/pop_back");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        zenit_deque_push_front(d, &vals[i]);
    }

    int out;
    zenit_deque_pop_back(d, &out);
    ASSERT(out == 10, "expected 10 (first pushed)");
    zenit_deque_pop_back(d, &out);
    ASSERT(out == 20, "expected 20");
    zenit_deque_pop_back(d, &out);
    ASSERT(out == 30, "expected 30");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: push_front / pop_front (LIFO) ─── */
static int test_push_front_pop_front(void) {
    TEST("push_front/pop_front");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        zenit_deque_push_front(d, &vals[i]);
    }

    int out;
    zenit_deque_pop_front(d, &out);
    ASSERT(out == 30, "expected 30 (last pushed front)");
    zenit_deque_pop_front(d, &out);
    ASSERT(out == 20, "expected 20");
    zenit_deque_pop_front(d, &out);
    ASSERT(out == 10, "expected 10");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: push_back / pop_back (stack) ─── */
static int test_push_back_pop_back(void) {
    TEST("push_back/pop_back");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        zenit_deque_push_back(d, &vals[i]);
    }

    int out;
    zenit_deque_pop_back(d, &out);
    ASSERT(out == 30, "expected 30 (last pushed back)");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: pop on empty ─── */
static int test_pop_empty(void) {
    TEST("pop on empty returns error");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    int out;
    ASSERT(zenit_deque_pop_front(d, &out).error == ZENIT_ERROR_EMPTY, "expected EMPTY front");
    ASSERT(zenit_deque_pop_back(d, &out).error == ZENIT_ERROR_EMPTY, "expected EMPTY back");
    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: push/pop NULL params ─── */
static int test_null_params(void) {
    TEST("NULL params return error");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    int v = 0;
    int out;

    ASSERT(zenit_deque_push_front(NULL, &v).error == ZENIT_ERROR_NULL, "push_front NULL list");
    ASSERT(zenit_deque_push_front(d, NULL).error == ZENIT_ERROR_NULL, "push_front NULL elem");
    ASSERT(zenit_deque_push_back(NULL, &v).error == ZENIT_ERROR_NULL, "push_back NULL list");
    ASSERT(zenit_deque_push_back(d, NULL).error == ZENIT_ERROR_NULL, "push_back NULL elem");
    ASSERT(zenit_deque_pop_front(NULL, &out).error == ZENIT_ERROR_NULL, "pop_front NULL list");
    ASSERT(zenit_deque_pop_front(d, NULL).error == ZENIT_ERROR_NULL, "pop_front NULL out");
    ASSERT(zenit_deque_pop_back(NULL, &out).error == ZENIT_ERROR_NULL, "pop_back NULL list");
    ASSERT(zenit_deque_pop_back(d, NULL).error == ZENIT_ERROR_NULL, "pop_back NULL out");
    ASSERT(zenit_deque_reserve(NULL, 10).error == ZENIT_ERROR_NULL, "reserve NULL");
    ASSERT(zenit_deque_shrink_to_fit(NULL).error == ZENIT_ERROR_NULL, "shrink NULL");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: get by index ─── */
static int test_get(void) {
    TEST("get by index");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));

    int vals[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        zenit_deque_push_back(d, &vals[i]);
    }

    ASSERT(*(int *)zenit_deque_get(d, 0) == 10, "get(0)");
    ASSERT(*(int *)zenit_deque_get(d, 1) == 20, "get(1)");
    ASSERT(*(int *)zenit_deque_get(d, 2) == 30, "get(2)");
    ASSERT(*(int *)zenit_deque_get(d, 3) == 40, "get(3)");

    ASSERT(zenit_deque_get(d, 4) == NULL, "get OOB");
    ASSERT(zenit_deque_get(NULL, 0) == NULL, "get on NULL");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: get with wrap-around ─── */
static int test_get_wrap(void) {
    TEST("get with wrap-around");
    /* Create a deque with small capacity to force wrap */
    zenit_deque_t *d = zenit_deque_create_with_capacity(sizeof(int), 4);
    ASSERT(d != NULL, "expected non-NULL");

    int vals[] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        zenit_deque_push_back(d, &vals[i]);
    }

    /* Pop two front, push two back — forces head to wrap */
    int out;
    zenit_deque_pop_front(d, &out); ASSERT(out == 1, "pop 1");
    zenit_deque_pop_front(d, &out); ASSERT(out == 2, "pop 2");

    int v = 5; zenit_deque_push_back(d, &v);
    v = 6; zenit_deque_push_back(d, &v);

    /* Now buffer has [3, 4, 5, 6] with head at index 2 */
    ASSERT(zenit_deque_count(d) == 4, "count 4");
    ASSERT(*(int *)zenit_deque_get(d, 0) == 3, "get(0) after wrap");
    ASSERT(*(int *)zenit_deque_get(d, 1) == 4, "get(1) after wrap");
    ASSERT(*(int *)zenit_deque_get(d, 2) == 5, "get(2) after wrap");
    ASSERT(*(int *)zenit_deque_get(d, 3) == 6, "get(3) after wrap");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: front and back access ─── */
static int test_front_back(void) {
    TEST("front/back");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    ASSERT(zenit_deque_front(d) == NULL, "front empty");
    ASSERT(zenit_deque_back(d) == NULL, "back empty");
    ASSERT(zenit_deque_front(NULL) == NULL, "front NULL");
    ASSERT(zenit_deque_back(NULL) == NULL, "back NULL");

    int a = 10;
    int b = 20;
    zenit_deque_push_back(d, &a);
    zenit_deque_push_back(d, &b);
    ASSERT(*(int *)zenit_deque_front(d) == 10, "front is 10");
    ASSERT(*(int *)zenit_deque_back(d) == 20, "back is 20");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: reserve ─── */
static int test_reserve(void) {
    TEST("reserve");
    zenit_deque_t *d = zenit_deque_create_with_capacity(sizeof(int), 4);
    ASSERT(zenit_deque_capacity(d) == 4, "initial capacity 4");

    /* Push some elements to test that reserve preserves order */
    int vals[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        zenit_deque_push_back(d, &vals[i]);
    }

    ASSERT(zenit_deque_reserve(d, 10).error == ZENIT_OK, "reserve to 10");
    ASSERT(zenit_deque_capacity(d) >= 10, "capacity >= 10");
    ASSERT(zenit_deque_count(d) == 3, "count preserved");

    /* Verify order is preserved after linearisation */
    for (int i = 0; i < 3; i++) {
        ASSERT(*(int *)zenit_deque_get(d, (size_t)i) == vals[i], "order preserved");
    }

    /* No-op reserve */
    ASSERT(zenit_deque_reserve(d, 5).error == ZENIT_OK, "reserve smaller no-op");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: shrink_to_fit ─── */
static int test_shrink(void) {
    TEST("shrink_to_fit");
    zenit_deque_t *d = zenit_deque_create_with_capacity(sizeof(int), 16);
    ASSERT(d != NULL, "expected non-NULL");

    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        zenit_deque_push_back(d, &vals[i]);
    }

    ASSERT(zenit_deque_shrink_to_fit(d).error == ZENIT_OK, "shrink");
    ASSERT(zenit_deque_capacity(d) == 5, "capacity shrunk to 5");
    ASSERT(zenit_deque_count(d) == 5, "count preserved");

    for (int i = 0; i < 5; i++) {
        ASSERT(*(int *)zenit_deque_get(d, (size_t)i) == vals[i], "order preserved");
    }

    /* Shrink to already-fit is no-op */
    ASSERT(zenit_deque_shrink_to_fit(d).error == ZENIT_OK, "shrink already fit");

    /* Shrink empty to 0 */
    zenit_deque_clear(d);
    ASSERT(zenit_deque_shrink_to_fit(d).error == ZENIT_OK, "shrink empty");
    ASSERT(zenit_deque_capacity(d) == 0, "capacity 0 after shrink empty");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    TEST("clear");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    for (int i = 0; i < 10; i++) {
        zenit_deque_push_back(d, &i);
    }
    ASSERT(zenit_deque_count(d) == 10, "count before clear");

    zenit_deque_clear(d);
    ASSERT(zenit_deque_count(d) == 0, "count after clear");
    ASSERT(zenit_deque_empty(d), "empty after clear");

    zenit_deque_clear(NULL);
    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: many elements ─── */
static int test_many_elements(void) {
    TEST("many elements");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));
    int n = 10000;

    for (int i = 0; i < n; i++) {
        zenit_deque_push_back(d, &i);
    }
    ASSERT(zenit_deque_count(d) == (size_t)n, "count matches");

    for (int i = 0; i < n; i++) {
        ASSERT(*(int *)zenit_deque_get(d, (size_t)i) == i, "value at index");
    }

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: mixed push/pop from both ends ─── */
static int test_mixed(void) {
    TEST("mixed push/pop both ends");
    zenit_deque_t *d = zenit_deque_create(sizeof(int));

    /* Build: front <- [20, 30, 40, 50] <- back */
    int v;
    v = 30; zenit_deque_push_back(d, &v);
    v = 20; zenit_deque_push_front(d, &v);
    v = 40; zenit_deque_push_back(d, &v);
    v = 10; zenit_deque_push_front(d, &v);
    v = 50; zenit_deque_push_back(d, &v);

    ASSERT(zenit_deque_count(d) == 5, "count 5");

    int out;
    zenit_deque_pop_front(d, &out); ASSERT(out == 10, "pop front 10");
    zenit_deque_pop_back(d, &out);  ASSERT(out == 50, "pop back 50");
    zenit_deque_pop_front(d, &out); ASSERT(out == 20, "pop front 20");
    zenit_deque_pop_back(d, &out);  ASSERT(out == 40, "pop back 40");
    zenit_deque_pop_front(d, &out); ASSERT(out == 30, "pop front 30");
    ASSERT(zenit_deque_empty(d), "empty after all pops");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: struct elements ─── */
typedef struct {
    int id;
    double val;
} point_t;

static int test_struct(void) {
    TEST("struct elements");
    zenit_deque_t *d = zenit_deque_create(sizeof(point_t));
    ASSERT(d != NULL, "expected non-NULL");

    point_t p1 = {1, 1.5};
    point_t p2 = {2, 2.5};
    zenit_deque_push_back(d, &p1);
    zenit_deque_push_front(d, &p2);

    const point_t *fp = (const point_t *)zenit_deque_front(d);
    ASSERT(fp->id == 2 && fp->val == 2.5, "front check");

    const point_t *bp = (const point_t *)zenit_deque_back(d);
    ASSERT(bp->id == 1 && bp->val == 1.5, "back check");

    zenit_deque_destroy(d);
    PASS();
    return 0;
}

/* ─── Test: count/capacity/empty on NULL ─── */
static int test_query_null(void) {
    TEST("query functions on NULL");
    ASSERT(zenit_deque_count(NULL) == 0, "count on NULL");
    ASSERT(zenit_deque_capacity(NULL) == 0, "capacity on NULL");
    ASSERT(zenit_deque_empty(NULL) == 1, "empty on NULL");
    PASS();
    return 0;
}

int main(void) {
    struct { int (*fn)(void); const char *name; } tests[] = {
        { test_create_destroy,     "create_destroy" },
        { test_create_invalid,     "create_invalid" },
        { test_destroy_null,       "destroy_null" },
        { test_push_back_pop_front, "push_back_pop_front" },
        { test_push_front_pop_back, "push_front_pop_back" },
        { test_push_front_pop_front, "push_front_pop_front" },
        { test_push_back_pop_back, "push_back_pop_back" },
        { test_pop_empty,          "pop_empty" },
        { test_null_params,        "null_params" },
        { test_get,                "get" },
        { test_get_wrap,           "get_wrap" },
        { test_front_back,         "front_back" },
        { test_reserve,            "reserve" },
        { test_shrink,             "shrink" },
        { test_clear,              "clear" },
        { test_many_elements,      "many_elements" },
        { test_mixed,              "mixed" },
        { test_struct,             "struct" },
        { test_query_null,         "query_null" },
        { 0, 0 }
    };

    printf("=== test_deque ===\n");
    for (int i = 0; tests[i].fn != NULL; i++) {
        tests[i].fn();
    }

    printf("\n%d passed, %d failed out of %d\n",
           tests_passed, tests_failed, tests_passed + tests_failed);

    if (tests_failed != 0) return 1;

    size_t total = tests_passed + tests_failed;
    size_t expected = 0;
    while (tests[expected].fn != NULL) expected++;
    if (total != expected) {
        fprintf(stderr, "Mismatch: %zu tests defined, %zu executed\n", expected, total);
        return 1;
    }

    return 0;
}
