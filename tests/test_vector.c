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

#include <libzenit/vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
#define FAIL(msg) do { \
    fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    failures++; \
} while(0)

/* ─── Test: create / destroy ─── */
static int test_create_destroy(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create returned NULL"); return 1; }

    if (zenit_vector_count(v) != 0) { FAIL("new vector not empty"); return 1; }
    if (zenit_vector_capacity(v) != 0) { FAIL("new vector capacity not 0"); return 1; }
    if (!zenit_vector_empty(v)) { FAIL("new vector should be empty"); return 1; }

    zenit_vector_destroy(v);
    zenit_vector_destroy(NULL);
    return 0;
}

/* ─── Test: create with zero elem_size ─── */
static int test_create_zero_elem(void) {
    zenit_vector_t *v = zenit_vector_create(0);
    if (v != NULL) {
        FAIL("expected NULL for elem_size=0");
        zenit_vector_destroy(v);
        return 1;
    }
    return 0;
}

/* ─── Test: create_with_capacity ─── */
static int test_create_with_capacity(void) {
    zenit_vector_t *v = zenit_vector_create_with_capacity(4, 16);
    if (v == NULL) { FAIL("create_with_capacity returned NULL"); return 1; }

    if (zenit_vector_capacity(v) != 16) { FAIL("capacity should be 16"); return 1; }
    if (zenit_vector_count(v) != 0) { FAIL("count should be 0"); return 1; }
    if (!zenit_vector_empty(v)) { FAIL("should be empty"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: create_with_capacity bad params ─── */
static int test_create_with_capacity_bad(void) {
    zenit_vector_t *v;

    v = zenit_vector_create_with_capacity(0, 16);
    if (v != NULL) { FAIL("expected NULL for elem_size=0"); return 1; }

    v = zenit_vector_create_with_capacity(4, 0);
    if (v != NULL) { FAIL("expected NULL for capacity=0"); return 1; }

    return 0;
}

/* ─── Test: push and get basic ─── */
static int test_push_get(void) {
    zenit_vector_t *v = zenit_vector_create(sizeof(int));
    if (v == NULL) { FAIL("create"); return 1; }

    for (int i = 0; i < 100; i++) {
        if (zenit_vector_push(v, &i).error != ZENIT_OK) {
            FAIL("push failed"); return 1;
        }
    }

    if (zenit_vector_count(v) != 100) { FAIL("count should be 100"); return 1; }

    for (int i = 0; i < 100; i++) {
        const int *p = (const int *)zenit_vector_get(v, (size_t)i);
        if (p == NULL) { FAIL("get returned NULL"); return 1; }
        if (*p != i) { FAIL("data mismatch"); return 1; }
    }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: pop ─── */
static int test_pop(void) {
    zenit_vector_t *v = zenit_vector_create(sizeof(int));
    if (v == NULL) { FAIL("create"); return 1; }

    for (int i = 0; i < 10; i++) {
        if (zenit_vector_push(v, &i).error != ZENIT_OK) {
            FAIL("push"); return 1;
        }
    }

    for (int i = 9; i >= 0; i--) {
        int val = -1;
        zenit_result_t r = zenit_vector_pop(v, &val);
        if (r.error != ZENIT_OK) { FAIL("pop"); return 1; }
        if (val != i) { FAIL("pop data mismatch"); return 1; }
    }

    if (zenit_vector_count(v) != 0) { FAIL("should be empty after pop"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: pop from empty ─── */
static int test_pop_empty(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    int val;
    zenit_result_t r = zenit_vector_pop(v, &val);
    if (r.error != ZENIT_ERROR_EMPTY) {
        FAIL("expected EMPTY on pop from empty"); return 1;
    }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: set ─── */
static int test_set(void) {
    zenit_vector_t *v = zenit_vector_create(sizeof(int));
    if (v == NULL) { FAIL("create"); return 1; }

    int a = 42;
    if (zenit_vector_push(v, &a).error != ZENIT_OK) { FAIL("push"); return 1; }

    int b = 99;
    if (zenit_vector_set(v, 0, &b).error != ZENIT_OK) { FAIL("set"); return 1; }

    const int *p = (const int *)zenit_vector_get(v, 0);
    if (p == NULL) { FAIL("get after set"); return 1; }
    if (*p != 99) { FAIL("set data mismatch"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: set out of bounds ─── */
static int test_set_oob(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    int x = 42;
    zenit_result_t r = zenit_vector_set(v, 0, &x);
    if (r.error != ZENIT_ERROR_PARAM) {
        FAIL("expected PARAM on set out of bounds"); return 1;
    }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: get out of bounds returns NULL ─── */
static int test_get_oob(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    const void *p = zenit_vector_get(v, 0);
    if (p != NULL) { FAIL("expected NULL for OOB get"); return 1; }

    p = zenit_vector_get(NULL, 0);
    if (p != NULL) { FAIL("expected NULL for NULL vector get"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: insert ─── */
static int test_insert(void) {
    zenit_vector_t *v = zenit_vector_create(sizeof(int));
    if (v == NULL) { FAIL("create"); return 1; }

    /* Insert at end (same as push) */
    int a = 10;
    if (zenit_vector_insert(v, 0, &a).error != ZENIT_OK) { FAIL("insert first"); return 1; }

    int b = 30;
    if (zenit_vector_insert(v, 1, &b).error != ZENIT_OK) { FAIL("insert last"); return 1; }

    /* Insert in middle */
    int c = 20;
    if (zenit_vector_insert(v, 1, &c).error != ZENIT_OK) { FAIL("insert middle"); return 1; }

    /* Insert at front */
    int d = 0;
    if (zenit_vector_insert(v, 0, &d).error != ZENIT_OK) { FAIL("insert front"); return 1; }

    /* Expected: [0, 10, 20, 30] */
    int expected[] = {0, 10, 20, 30};
    for (size_t i = 0; i < 4; i++) {
        const int *p = (const int *)zenit_vector_get(v, i);
        if (p == NULL || *p != expected[i]) {
            FAIL("insert data mismatch"); return 1;
        }
    }

    /* Insert at index == count (append) */
    int e = 40;
    if (zenit_vector_insert(v, 4, &e).error != ZENIT_OK) { FAIL("insert at count"); return 1; }
    if (zenit_vector_count(v) != 5) { FAIL("count should be 5"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: insert out of bounds ─── */
static int test_insert_oob(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    int x = 42;
    zenit_result_t r = zenit_vector_insert(v, 1, &x);
    if (r.error != ZENIT_ERROR_PARAM) {
        FAIL("expected PARAM on insert OOB"); return 1;
    }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: remove ─── */
static int test_remove(void) {
    zenit_vector_t *v = zenit_vector_create(sizeof(int));
    if (v == NULL) { FAIL("create"); return 1; }

    int data[] = {0, 10, 20, 30, 40};
    for (size_t i = 0; i < 5; i++) {
        if (zenit_vector_push(v, &data[i]).error != ZENIT_OK) { FAIL("push"); return 1; }
    }

    /* Remove middle (index 2 → 20) */
    int out = -1;
    if (zenit_vector_remove(v, 2, &out).error != ZENIT_OK) { FAIL("remove middle"); return 1; }
    if (out != 20) { FAIL("remove data mismatch"); return 1; }

    /* Expected: [0, 10, 30, 40] */
    int expected1[] = {0, 10, 30, 40};
    for (size_t i = 0; i < 4; i++) {
        const int *p = (const int *)zenit_vector_get(v, i);
        if (p == NULL || *p != expected1[i]) {
            FAIL("remove data order wrong"); return 1;
        }
    }

    /* Remove last */
    if (zenit_vector_remove(v, 3, &out).error != ZENIT_OK) { FAIL("remove last"); return 1; }
    if (out != 40) { FAIL("remove last data"); return 1; }

    /* Remove first */
    if (zenit_vector_remove(v, 0, &out).error != ZENIT_OK) { FAIL("remove first"); return 1; }
    if (out != 0) { FAIL("remove first data"); return 1; }

    if (zenit_vector_count(v) != 2) { FAIL("count should be 2"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: remove out of bounds ─── */
static int test_remove_oob(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    int out;
    zenit_result_t r = zenit_vector_remove(v, 0, &out);
    if (r.error != ZENIT_ERROR_PARAM) {
        FAIL("expected PARAM on remove from empty"); return 1;
    }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: reserve ─── */
static int test_reserve(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    if (zenit_vector_reserve(v, 100).error != ZENIT_OK) { FAIL("reserve"); return 1; }
    if (zenit_vector_capacity(v) < 100) { FAIL("capacity < 100 after reserve"); return 1; }

    /* Reserve smaller than current — should be no-op */
    if (zenit_vector_reserve(v, 10).error != ZENIT_OK) { FAIL("reserve smaller"); return 1; }
    if (zenit_vector_capacity(v) < 100) { FAIL("capacity should not shrink"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: shrink_to_fit ─── */
static int test_shrink_to_fit(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    /* Push some elements, then shrink */
    for (int i = 0; i < 10; i++) {
        if (zenit_vector_push(v, &i).error != ZENIT_OK) { FAIL("push"); return 1; }
    }

    size_t cap_before = zenit_vector_capacity(v);
    if (cap_before < 10) { FAIL("capacity should be >= 10"); return 1; }

    if (zenit_vector_shrink_to_fit(v).error != ZENIT_OK) { FAIL("shrink"); return 1; }
    if (zenit_vector_capacity(v) != 10) { FAIL("capacity should be 10 after shrink"); return 1; }

    /* Verify data still intact */
    for (int i = 0; i < 10; i++) {
        const int *p = (const int *)zenit_vector_get(v, (size_t)i);
        if (p == NULL || *p != i) { FAIL("data after shrink"); return 1; }
    }

    /* Shrink empty vector to zero */
    zenit_vector_clear(v);
    if (zenit_vector_shrink_to_fit(v).error != ZENIT_OK) { FAIL("shrink empty"); return 1; }
    if (zenit_vector_capacity(v) != 0) { FAIL("capacity should be 0"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    for (int i = 0; i < 10; i++) {
        if (zenit_vector_push(v, &i).error != ZENIT_OK) { FAIL("push"); return 1; }
    }

    zenit_vector_clear(v);
    if (zenit_vector_count(v) != 0) { FAIL("count should be 0 after clear"); return 1; }
    if (!zenit_vector_empty(v)) { FAIL("should be empty after clear"); return 1; }
    /* Capacity unchanged */
    if (zenit_vector_capacity(v) == 0) { FAIL("capacity should not shrink on clear"); return 1; }

    /* Re-push after clear */
    int x = 99;
    if (zenit_vector_push(v, &x).error != ZENIT_OK) { FAIL("push after clear"); return 1; }
    if (zenit_vector_count(v) != 1) { FAIL("count should be 1"); return 1; }

    zenit_vector_clear(NULL);
    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: NULL / edge cases ─── */
static int test_edge_cases(void) {
    zenit_vector_t *v = zenit_vector_create(4);
    if (v == NULL) { FAIL("create"); return 1; }

    int x = 42;

    /* push with NULL vector */
    if (zenit_vector_push(NULL, &x).error != ZENIT_ERROR_NULL) {
        FAIL("push(NULL vector)"); return 1;
    }
    /* push with NULL elem */
    if (zenit_vector_push(v, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("push(NULL elem)"); return 1;
    }

    /* pop with NULL vector */
    if (zenit_vector_pop(NULL, &x).error != ZENIT_ERROR_NULL) {
        FAIL("pop(NULL vector)"); return 1;
    }
    /* pop with NULL out */
    if (zenit_vector_pop(v, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("pop(NULL out)"); return 1;
    }

    /* insert with NULL vector */
    if (zenit_vector_insert(NULL, 0, &x).error != ZENIT_ERROR_NULL) {
        FAIL("insert(NULL vector)"); return 1;
    }
    /* insert with NULL elem */
    if (zenit_vector_insert(v, 0, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("insert(NULL elem)"); return 1;
    }

    /* remove with NULL vector */
    if (zenit_vector_remove(NULL, 0, &x).error != ZENIT_ERROR_NULL) {
        FAIL("remove(NULL vector)"); return 1;
    }
    /* remove with NULL out */
    if (zenit_vector_remove(v, 0, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("remove(NULL out)"); return 1;
    }

    /* set with NULL vector */
    if (zenit_vector_set(NULL, 0, &x).error != ZENIT_ERROR_NULL) {
        FAIL("set(NULL vector)"); return 1;
    }
    /* set with NULL elem */
    if (zenit_vector_set(v, 0, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("set(NULL elem)"); return 1;
    }

    /* count/capacity with NULL */
    if (zenit_vector_count(NULL) != 0) { FAIL("count(NULL)"); return 1; }
    if (zenit_vector_capacity(NULL) != 0) { FAIL("capacity(NULL)"); return 1; }

    /* empty with NULL */
    if (!zenit_vector_empty(NULL)) { FAIL("empty(NULL) should be 1"); return 1; }

    /* reserve with NULL */
    if (zenit_vector_reserve(NULL, 10).error != ZENIT_ERROR_NULL) {
        FAIL("reserve(NULL)"); return 1;
    }

    /* shrink_to_fit with NULL */
    if (zenit_vector_shrink_to_fit(NULL).error != ZENIT_ERROR_NULL) {
        FAIL("shrink_to_fit(NULL)"); return 1;
    }

    /* shrink_to_fit when count == capacity — no-op path */
    zenit_vector_t *v2 = zenit_vector_create_with_capacity(4, 8);
    if (v2 == NULL) { FAIL("create_with_capacity"); return 1; }
    for (int i = 0; i < 8; i++) {
        if (zenit_vector_push(v2, &i).error != ZENIT_OK) { FAIL("push"); return 1; }
    }
    if (zenit_vector_capacity(v2) != 8) { FAIL("capacity should be 8 before shrink"); return 1; }
    if (zenit_vector_shrink_to_fit(v2).error != ZENIT_OK) { FAIL("shrink no-op"); return 1; }
    if (zenit_vector_capacity(v2) != 8) { FAIL("capacity should stay 8 on no-op shrink"); return 1; }
    zenit_vector_destroy(v2);

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: push with struct ─── */
static int test_struct(void) {
    typedef struct { int x; double y; } point_t;
    zenit_vector_t *v = zenit_vector_create(sizeof(point_t));
    if (v == NULL) { FAIL("create"); return 1; }

    point_t p1 = { 1, 2.5 };
    point_t p2 = { 3, 4.5 };

    if (zenit_vector_push(v, &p1).error != ZENIT_OK) { FAIL("push p1"); return 1; }
    if (zenit_vector_push(v, &p2).error != ZENIT_OK) { FAIL("push p2"); return 1; }

    const point_t *g1 = (const point_t *)zenit_vector_get(v, 0);
    const point_t *g2 = (const point_t *)zenit_vector_get(v, 1);
    if (g1 == NULL || g2 == NULL) { FAIL("get struct"); return 1; }
    if (g1->x != 1 || g1->y != 2.5) { FAIL("struct 1 mismatch"); return 1; }
    if (g2->x != 3 || g2->y != 4.5) { FAIL("struct 2 mismatch"); return 1; }

    zenit_vector_destroy(v);
    return 0;
}

/* ─── Test: large number of elements (forcing multiple grows) ─── */
static int test_many_elements(void) {
    zenit_vector_t *v = zenit_vector_create(sizeof(int));
    if (v == NULL) { FAIL("create"); return 1; }

    int n = 10000;
    for (int i = 0; i < n; i++) {
        if (zenit_vector_push(v, &i).error != ZENIT_OK) {
            FAIL("push many"); return 1;
        }
    }

    if (zenit_vector_count(v) != (size_t)n) { FAIL("count should be 10000"); return 1; }

    for (int i = 0; i < n; i++) {
        const int *p = (const int *)zenit_vector_get(v, (size_t)i);
        if (p == NULL || *p != i) { FAIL("many data mismatch"); return 1; }
    }

    zenit_vector_destroy(v);
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_destroy();
    ret |= test_create_zero_elem();
    ret |= test_create_with_capacity();
    ret |= test_create_with_capacity_bad();
    ret |= test_push_get();
    ret |= test_pop();
    ret |= test_pop_empty();
    ret |= test_set();
    ret |= test_set_oob();
    ret |= test_get_oob();
    ret |= test_insert();
    ret |= test_insert_oob();
    ret |= test_remove();
    ret |= test_remove_oob();
    ret |= test_reserve();
    ret |= test_shrink_to_fit();
    ret |= test_clear();
    ret |= test_edge_cases();
    ret |= test_struct();
    ret |= test_many_elements();

    if (failures > 0 || ret != 0) {
        fprintf(stderr, "FAIL: %d test(s) had errors\n", failures);
        return 1;
    }

    printf("PASS: vector tests\n");
    return 0;
}
