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

#include "test_runner.h"
#include <libzenit/spsc.h>

static int test_create_destroy(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 16);
    ASSERT(s != NULL, "spsc create");
    ASSERT(zenit_spsc_capacity(s) == 16, "spsc capacity");
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_push_pop(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
    int val = 42;
    zenit_result_t r = zenit_spsc_push(s, &val);
    ASSERT(r.error == ZENIT_OK, "spsc push");
    int out = 0;
    r = zenit_spsc_pop(s, &out);
    ASSERT(r.error == ZENIT_OK && out == 42, "spsc pop");
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_full(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 2);
    int v = 1; zenit_spsc_push(s, &v);
    v = 2; zenit_spsc_push(s, &v);
    v = 3;
    zenit_result_t r = zenit_spsc_push(s, &v);
    ASSERT(r.error == ZENIT_ERROR_FULL, "spsc full");
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_empty(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
    int out;
    zenit_result_t r = zenit_spsc_pop(s, &out);
    ASSERT(r.error == ZENIT_ERROR_EMPTY, "spsc empty");
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_count_empty_full(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
    ASSERT(zenit_spsc_empty(s), "spsc empty start");
    ASSERT(!zenit_spsc_full(s), "spsc not full start");
    int v = 1; zenit_spsc_push(s, &v);
    ASSERT(!zenit_spsc_empty(s), "spsc not empty after push");
    ASSERT(zenit_spsc_count(s) == 1, "spsc count");
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_wrap_around(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
    int v;
    for (int i = 0; i < 10; i++) {
        v = i; zenit_spsc_push(s, &v);
        zenit_spsc_pop(s, &v);
    }
    v = 99; zenit_spsc_push(s, &v);
    int out;
    zenit_spsc_pop(s, &out);
    ASSERT(out == 99, "spsc wrap");
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_multiple(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 8);
    for (int i = 0; i < 7; i++) {
        zenit_spsc_push(s, &i);
    }
    ASSERT(zenit_spsc_count(s) == 7, "spsc multi count");
    for (int i = 0; i < 7; i++) {
        int out;
        zenit_spsc_pop(s, &out);
        ASSERT(out == i, "spsc multi pop");
    }
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_null_params(void) {
    zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
    zenit_result_t r = zenit_spsc_push(NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "spsc push NULL");
    r = zenit_spsc_push(s, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "spsc push elem NULL");
    r = zenit_spsc_pop(NULL, NULL);
    ASSERT(r.error == ZENIT_ERROR_NULL, "spsc pop NULL");
    ASSERT(zenit_spsc_count(NULL) == 0, "spsc count NULL");
    ASSERT(zenit_spsc_capacity(NULL) == 0, "spsc capacity NULL");
    ASSERT(zenit_spsc_full(NULL) == 0, "spsc full NULL");
    ASSERT(zenit_spsc_empty(NULL) == 0, "spsc empty NULL");
    zenit_spsc_destroy(NULL);
    zenit_spsc_destroy(s);
    PASS();
    return 0;
}

static int test_invalid_create(void) {
    zenit_spsc_t *s = zenit_spsc_create(0, 16);
    ASSERT(s == NULL, "spsc create zero elem_size");
    s = zenit_spsc_create(sizeof(int), 0);
    ASSERT(s == NULL, "spsc create zero capacity");
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_create_destroy,
        test_push_pop,
        test_full,
        test_empty,
        test_count_empty_full,
        test_wrap_around,
        test_multiple,
        test_null_params,
        test_invalid_create,
    };
    const char *names[] = {
        "create_destroy",
        "push_pop",
        "full",
        "empty",
        "count_empty_full",
        "wrap_around",
        "multiple",
        "null_params",
        "invalid_create",
    };
    ZENIT_RUN_TESTS("spsc", tests, names);
    return 0;
}
