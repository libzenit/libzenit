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
#include <string.h>

#include "test_runner.h"

/* ─── Visitor context for foreach test ─── */
typedef struct {
    int values[256];
    size_t count;
} visit_ctx_t;

static void visit_int(const void *elem, void *ctx) {
    visit_ctx_t *c = (visit_ctx_t *)ctx;
    c->values[c->count] = *(const int *)elem;
    c->count++;
}

/* ─── Test: create and destroy ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    ASSERT(list != NULL, "expected non-NULL list");
    ASSERT(zenit_list_count(list) == 0, "count should be 0");
    ASSERT(zenit_list_empty(list), "list should be empty");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: create with zero elem_size returns NULL ─── */
static int test_create_zero_elem(void) {
    TEST("create with zero elem_size returns NULL");
    ASSERT(zenit_list_create(0) == NULL, "expected NULL");
    PASS();
    return 0;
}

/* ─── Test: destroy NULL is safe ─── */
static int test_destroy_null(void) {
    TEST("destroy NULL is safe");
    zenit_list_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: push_front and pop_front ─── */
static int test_push_pop_front(void) {
    TEST("push_front/pop_front");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    ASSERT(list != NULL, "expected non-NULL");

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        ASSERT(zenit_list_push_front(list, &vals[i]).error == ZENIT_OK, "push_front failed");
    }
    ASSERT(zenit_list_count(list) == 3, "count should be 3");

    int out;
    ASSERT(zenit_list_pop_front(list, &out).error == ZENIT_OK, "pop_front failed");
    ASSERT(out == 30, "expected 30 (last pushed)");
    ASSERT(zenit_list_pop_front(list, &out).error == ZENIT_OK, "pop_front failed");
    ASSERT(out == 20, "expected 20");
    ASSERT(zenit_list_pop_front(list, &out).error == ZENIT_OK, "pop_front failed");
    ASSERT(out == 10, "expected 10");
    ASSERT(zenit_list_count(list) == 0, "count should be 0");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: push_back and pop_back ─── */
static int test_push_pop_back(void) {
    TEST("push_back/pop_back");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    ASSERT(list != NULL, "expected non-NULL");

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        ASSERT(zenit_list_push_back(list, &vals[i]).error == ZENIT_OK, "push_back failed");
    }
    ASSERT(zenit_list_count(list) == 3, "count should be 3");

    int out;
    ASSERT(zenit_list_pop_back(list, &out).error == ZENIT_OK, "pop_back failed");
    ASSERT(out == 30, "expected 30 (last pushed)");
    ASSERT(zenit_list_pop_back(list, &out).error == ZENIT_OK, "pop_back failed");
    ASSERT(out == 20, "expected 20");
    ASSERT(zenit_list_pop_back(list, &out).error == ZENIT_OK, "pop_back failed");
    ASSERT(out == 10, "expected 10");
    ASSERT(zenit_list_count(list) == 0, "count should be 0");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: pop_front on empty returns error ─── */
static int test_pop_front_empty(void) {
    TEST("pop_front on empty returns error");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int out;
    ASSERT(zenit_list_pop_front(list, &out).error == ZENIT_ERROR_EMPTY, "expected EMPTY");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: pop_back on empty returns error ─── */
static int test_pop_back_empty(void) {
    TEST("pop_back on empty returns error");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int out;
    ASSERT(zenit_list_pop_back(list, &out).error == ZENIT_ERROR_EMPTY, "expected EMPTY");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: push_front NULL returns error ─── */
static int test_push_front_null(void) {
    TEST("push_front NULL returns error");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int v = 0;
    ASSERT(zenit_list_push_front(NULL, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_push_front(list, NULL).error == ZENIT_ERROR_NULL, "expected NULL on elem");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: push_back NULL returns error ─── */
static int test_push_back_null(void) {
    TEST("push_back NULL returns error");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int v = 0;
    ASSERT(zenit_list_push_back(NULL, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_push_back(list, NULL).error == ZENIT_ERROR_NULL, "expected NULL on elem");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: pop_front/back NULL returns error ─── */
static int test_pop_null(void) {
    TEST("pop NULL returns error");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int v = 0;
    ASSERT(zenit_list_pop_front(NULL, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_pop_front(list, NULL).error == ZENIT_ERROR_NULL, "expected NULL on out");
    ASSERT(zenit_list_pop_back(NULL, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_pop_back(list, NULL).error == ZENIT_ERROR_NULL, "expected NULL on out");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: push_front + pop_back (LIFO check) ─── */
static int test_push_front_pop_back(void) {
    TEST("push_front/pop_back");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        ASSERT(zenit_list_push_front(list, &vals[i]).error == ZENIT_OK, "push_front failed");
    }
    int out;
    ASSERT(zenit_list_pop_back(list, &out).error == ZENIT_OK, "pop_back failed");
    ASSERT(out == 10, "expected 10 (first pushed)");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: insert at various positions ─── */
static int test_insert(void) {
    TEST("insert");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int v;
    v = 10; zenit_list_insert(list, 0, &v);
    v = 30; zenit_list_insert(list, 1, &v);
    v = 20; zenit_list_insert(list, 1, &v);

    ASSERT(zenit_list_count(list) == 3, "count should be 3");
    ASSERT(*(int *)zenit_list_get(list, 0) == 10, "expected 10 at 0");
    ASSERT(*(int *)zenit_list_get(list, 1) == 20, "expected 20 at 1");
    ASSERT(*(int *)zenit_list_get(list, 2) == 30, "expected 30 at 2");

    /* Insert out of bounds */
    v = 99;
    ASSERT(zenit_list_insert(list, 4, &v).error == ZENIT_ERROR_PARAM, "expected PARAM");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: insert NULL params ─── */
static int test_insert_null(void) {
    TEST("insert NULL params");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int v = 0;
    ASSERT(zenit_list_insert(NULL, 0, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_insert(list, 0, NULL).error == ZENIT_ERROR_NULL, "expected NULL on elem");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: remove at various positions ─── */
static int test_remove(void) {
    TEST("remove");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int vals[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        zenit_list_push_back(list, &vals[i]);
    }

    int out;
    ASSERT(zenit_list_remove(list, 1, &out).error == ZENIT_OK, "remove at 1");
    ASSERT(out == 20, "expected 20");
    ASSERT(zenit_list_count(list) == 3, "count should be 3");

    ASSERT(*(int *)zenit_list_get(list, 0) == 10, "expected 10 at 0");
    ASSERT(*(int *)zenit_list_get(list, 1) == 30, "expected 30 at 1");
    ASSERT(*(int *)zenit_list_get(list, 2) == 40, "expected 40 at 2");

    /* Remove out of bounds */
    ASSERT(zenit_list_remove(list, 5, &out).error == ZENIT_ERROR_PARAM, "expected PARAM");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: remove at front and back ─── */
static int test_remove_front_back(void) {
    TEST("remove at front and back");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int vals[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        zenit_list_push_back(list, &vals[i]);
    }

    int out;
    /* Remove at front (index 0) */
    ASSERT(zenit_list_remove(list, 0, &out).error == ZENIT_OK, "remove at front");
    ASSERT(out == 10, "expected 10");
    ASSERT(zenit_list_count(list) == 3, "count should be 3");

    /* Remove at back (index count-1 = 2) */
    ASSERT(zenit_list_remove(list, 2, &out).error == ZENIT_OK, "remove at back");
    ASSERT(out == 40, "expected 40");
    ASSERT(zenit_list_count(list) == 2, "count should be 2");

    ASSERT(*(int *)zenit_list_get(list, 0) == 20, "expected 20 at 0");
    ASSERT(*(int *)zenit_list_get(list, 1) == 30, "expected 30 at 1");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: remove NULL params ─── */
static int test_remove_null(void) {
    TEST("remove NULL params");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int v = 0;
    ASSERT(zenit_list_remove(NULL, 0, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_remove(list, 0, NULL).error == ZENIT_ERROR_NULL, "expected NULL on out");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: get out of bounds ─── */
static int test_get_oob(void) {
    TEST("get out of bounds returns NULL");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    ASSERT(zenit_list_get(list, 0) == NULL, "expected NULL on empty");
    ASSERT(zenit_list_get(NULL, 0) == NULL, "expected NULL on NULL list");

    int v = 42;
    zenit_list_push_back(list, &v);
    ASSERT(zenit_list_get(list, 1) == NULL, "expected NULL OOB");
    ASSERT(zenit_list_get(list, 0) != NULL, "expected non-NULL in bounds");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: set at index ─── */
static int test_set(void) {
    TEST("set");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        zenit_list_push_back(list, &vals[i]);
    }

    int v = 99;
    ASSERT(zenit_list_set(list, 1, &v).error == ZENIT_OK, "set at 1");
    ASSERT(*(int *)zenit_list_get(list, 1) == 99, "expected 99 at 1");

    ASSERT(zenit_list_set(list, 5, &v).error == ZENIT_ERROR_PARAM, "expected PARAM OOB");
    ASSERT(zenit_list_set(NULL, 0, &v).error == ZENIT_ERROR_NULL, "expected NULL on list");
    ASSERT(zenit_list_set(list, 0, NULL).error == ZENIT_ERROR_NULL, "expected NULL on elem");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    TEST("clear");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    for (int i = 0; i < 10; i++) {
        zenit_list_push_back(list, &i);
    }
    ASSERT(zenit_list_count(list) == 10, "count should be 10");

    zenit_list_clear(list);
    ASSERT(zenit_list_count(list) == 0, "count should be 0 after clear");
    ASSERT(zenit_list_empty(list), "should be empty after clear");
    ASSERT(zenit_list_front(list) == NULL, "front should be NULL after clear");
    ASSERT(zenit_list_back(list) == NULL, "back should be NULL after clear");

    zenit_list_clear(NULL);
    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: foreach iteration ─── */
static int test_foreach(void) {
    TEST("foreach");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        zenit_list_push_back(list, &vals[i]);
    }

    visit_ctx_t ctx = { .count = 0 };
    zenit_list_foreach(list, visit_int, &ctx);
    ASSERT(ctx.count == 5, "should visit 5 elements");
    for (size_t i = 0; i < 5; i++) {
        ASSERT(ctx.values[i] == vals[i], "unexpected value in foreach");
    }

    zenit_list_foreach(NULL, visit_int, &ctx);
    zenit_list_foreach(list, NULL, &ctx);

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: front and back access ─── */
static int test_front_back(void) {
    TEST("front/back");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    ASSERT(zenit_list_front(list) == NULL, "front should be NULL on empty");
    ASSERT(zenit_list_back(list) == NULL, "back should be NULL on empty");
    ASSERT(zenit_list_front(NULL) == NULL, "front should be NULL on NULL");
    ASSERT(zenit_list_back(NULL) == NULL, "back should be NULL on NULL");

    int a = 10;
    int b = 20;
    zenit_list_push_back(list, &a);
    zenit_list_push_back(list, &b);
    ASSERT(*(int *)zenit_list_front(list) == 10, "front should be 10");
    ASSERT(*(int *)zenit_list_back(list) == 20, "back should be 20");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: large number of elements ─── */
static int test_many_elements(void) {
    TEST("many elements");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    int n = 1000;
    for (int i = 0; i < n; i++) {
        zenit_list_push_back(list, &i);
    }
    ASSERT(zenit_list_count(list) == (size_t)n, "count should be n");

    for (int i = 0; i < n; i++) {
        const int *v = (const int *)zenit_list_get(list, (size_t)i);
        ASSERT(*v == i, "unexpected value at index");
    }

    /* Pop all from front */
    for (int i = 0; i < n; i++) {
        int out;
        zenit_list_pop_front(list, &out);
        ASSERT(out == i, "unexpected pop value");
    }
    ASSERT(zenit_list_count(list) == 0, "count should be 0");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: struct elements ─── */
typedef struct {
    int x;
    double y;
    char label[16];
} test_struct_t;

static int test_struct(void) {
    TEST("struct elements");
    zenit_list_t *list = zenit_list_create(sizeof(test_struct_t));
    ASSERT(list != NULL, "expected non-NULL");

    test_struct_t s1 = {1, 1.5, "first"};
    test_struct_t s2 = {2, 2.5, "second"};
    zenit_list_push_back(list, &s1);
    zenit_list_push_front(list, &s2);

    const test_struct_t *g2 = (const test_struct_t *)zenit_list_get(list, 0);
    ASSERT(g2->x == 2 && g2->y == 2.5 && strcmp(g2->label, "second") == 0, "expected second at 0");

    const test_struct_t *g1 = (const test_struct_t *)zenit_list_get(list, 1);
    ASSERT(g1->x == 1 && g1->y == 1.5 && strcmp(g1->label, "first") == 0, "expected first at 1");

    zenit_list_destroy(list);
    PASS();
    return 0;
}

/* ─── Test: count/empty on NULL ─── */
static int test_count_empty_null(void) {
    TEST("count/empty on NULL");
    ASSERT(zenit_list_count(NULL) == 0, "count should be 0 on NULL");
    ASSERT(zenit_list_empty(NULL) == 1, "empty should be 1 on NULL");
    PASS();
    return 0;
}

/* ─── Test: clear after destroy is safe ─── */
static int test_clear_empty(void) {
    TEST("clear empty list is safe");
    zenit_list_t *list = zenit_list_create(sizeof(int));
    zenit_list_clear(list);
    ASSERT(zenit_list_count(list) == 0, "count should be 0");
    zenit_list_destroy(list);
    PASS();
    return 0;
}

int main(void) {
    TEST_ENTRY tests[] = {
        { test_create_destroy,          "create_destroy" },
        { test_create_zero_elem,        "create_zero_elem" },
        { test_destroy_null,            "destroy_null" },
        { test_push_pop_front,          "push_pop_front" },
        { test_push_pop_back,           "push_pop_back" },
        { test_pop_front_empty,         "pop_front_empty" },
        { test_pop_back_empty,          "pop_back_empty" },
        { test_push_front_null,         "push_front_null" },
        { test_push_back_null,          "push_back_null" },
        { test_pop_null,                "pop_null" },
        { test_push_front_pop_back,     "push_front_pop_back" },
        { test_insert,                  "insert" },
        { test_insert_null,             "insert_null" },
        { test_remove,                  "remove" },
        { test_remove_front_back,       "remove_front_back" },
        { test_remove_null,             "remove_null" },
        { test_get_oob,                 "get_oob" },
        { test_set,                     "set" },
        { test_clear,                   "clear" },
        { test_foreach,                 "foreach" },
        { test_front_back,              "front_back" },
        { test_many_elements,           "many_elements" },
        { test_struct,                  "struct" },
        { test_count_empty_null,        "count_empty_null" },
        { test_clear_empty,             "clear_empty" },
        { 0, 0 }
    };

    return test_run_all("test_list", tests);
}
