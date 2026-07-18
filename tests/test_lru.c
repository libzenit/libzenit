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

#include <libzenit/lru.h>
#include <libzenit/allocator.h>
#include "test_runner.h"
#include <string.h>
#include <stdlib.h>

/* ─── Helpers ─── */

static int evict_count = 0;
static int evict_last_key = -1;
static int evict_last_value = -1;

static void track_evict(const void *key, void *value, void *ctx) {
    (void)ctx;
    evict_count++;
    memcpy(&evict_last_key, key, sizeof(int));
    memcpy(&evict_last_value, value, sizeof(int));
}

static void reset_evict(void) {
    evict_count = 0;
    evict_last_key = -1;
    evict_last_value = -1;
}

/* ─── Tests ─── */

static int test_create_destroy(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    ASSERT(c != NULL, "create should succeed");
    ASSERT(zenit_lru_count(c) == 0, "new cache count 0");
    ASSERT(zenit_lru_capacity(c) == 4, "capacity is 4");
    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_put_get_hit(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    ASSERT(c != NULL, "create");

    int k = 42;
    int v = 100;
    zenit_result_t r = zenit_lru_put(c, &k, &v);
    ASSERT(r.error == ZENIT_OK, "put ok");

    ASSERT(zenit_lru_count(c) == 1, "count 1 after put");

    int out = 0;
    int found = zenit_lru_get(c, &k, &out);
    ASSERT(found == 1, "get hit");
    ASSERT(out == 100, "get correct value");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_get_miss(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    int k = 99;
    int out = 0;
    int found = zenit_lru_get(c, &k, &out);
    ASSERT(found == 0, "get miss returns 0");
    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_put_get_miss_after_evict(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 2);
    int k;

    /* Insert two entries */
    k = 0; zenit_lru_put(c, &k, &k);
    k = 1; zenit_lru_put(c, &k, &k);

    /* Third insert evicts 0 (which is LRU) */
    k = 2; zenit_lru_put(c, &k, &k);

    /* Key 0 should be gone */
    int out = -1;
    ASSERT(zenit_lru_get(c, &(int){0}, &out) == 0, "evicted key missing");

    /* Keys 1 and 2 should be present */
    ASSERT(zenit_lru_get(c, &(int){1}, &out) == 1 && out == 1, "key 1 present");
    ASSERT(zenit_lru_get(c, &(int){2}, &out) == 1 && out == 2, "key 2 present");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_peek_does_not_change_order(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 2);
    int k;
    int v;

    /* Insert 0 and 1.  Cache: [1 (MRU), 0 (LRU)] */
    k = 0; v = 10; zenit_lru_put(c, &k, &v);
    k = 1; v = 20; zenit_lru_put(c, &k, &v);

    /* Peek at 0 — should NOT promote it */
    int out = 0;
    int found = zenit_lru_peek(c, &(int){0}, &out);
    ASSERT(found == 1 && out == 10, "peek finds 0");

    /* Now insert 2: should evict 0 (LRU), NOT 1 */
    k = 2; v = 30; zenit_lru_put(c, &k, &v);

    ASSERT(zenit_lru_get(c, &(int){0}, &out) == 0, "0 evicted (still LRU after peek)");
    ASSERT(zenit_lru_get(c, &(int){1}, &out) == 1 && out == 20, "1 still present");
    ASSERT(zenit_lru_get(c, &(int){2}, &out) == 1 && out == 30, "2 present");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_update_existing_key(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 3);
    int k = 5;
    int v;

    v = 10; zenit_lru_put(c, &k, &v);
    v = 99; zenit_lru_put(c, &k, &v);

    int out = 0;
    ASSERT(zenit_lru_get(c, &k, &out) == 1, "get after update");
    ASSERT(out == 99, "updated value");
    ASSERT(zenit_lru_count(c) == 1, "count still 1 after update");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_remove(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    int k = 7;
    int v = 42;
    zenit_lru_put(c, &k, &v);

    zenit_result_t r = zenit_lru_remove(c, &k);
    ASSERT(r.error == ZENIT_OK, "remove ok");
    ASSERT(zenit_lru_count(c) == 0, "count 0 after remove");
    ASSERT(zenit_lru_get(c, &k, NULL) == 0, "get after remove miss");

    /* Remove non-existent key */
    r = zenit_lru_remove(c, &(int){99});
    ASSERT(r.error == ZENIT_ERROR_NOT_FOUND, "remove non-existent returns NOT_FOUND");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_contains(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    int k = 3;
    zenit_lru_put(c, &k, &k);

    ASSERT(zenit_lru_contains(c, &k) == 1, "contains existing");
    ASSERT(zenit_lru_contains(c, &(int){99}) == 0, "contains non-existing");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_clear(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    for (int i = 0; i < 4; i++) {
        zenit_lru_put(c, &i, &i);
    }
    ASSERT(zenit_lru_count(c) == 4, "count 4 before clear");

    zenit_lru_clear(c);
    ASSERT(zenit_lru_count(c) == 0, "count 0 after clear");
    for (int i = 0; i < 4; i++) {
        ASSERT(zenit_lru_get(c, &i, NULL) == 0, "key absent after clear");
    }

    /* Cache should still work after clear */
    int k = 10;
    int v = 20;
    zenit_lru_put(c, &k, &v);
    ASSERT(zenit_lru_count(c) == 1, "count 1 after clear+put");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_null_params(void) {
    /* Create/destroy with NULL */
    zenit_lru_destroy(NULL);

    /* Put with NULL */
    ASSERT(zenit_lru_put(NULL, NULL, NULL).error == ZENIT_ERROR_NULL, "put NULL handle");
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    ASSERT(c != NULL, "create");
    ASSERT(zenit_lru_put(c, NULL, NULL).error == ZENIT_ERROR_NULL, "put NULL key");
    ASSERT(zenit_lru_put(c, &(int){0}, NULL).error == ZENIT_ERROR_NULL, "put NULL value");

    /* Get with NULL */
    ASSERT(zenit_lru_get(NULL, NULL, NULL) == 0, "get NULL handle");
    ASSERT(zenit_lru_get(c, NULL, NULL) == 0, "get NULL key");

    /* Peek with NULL */
    ASSERT(zenit_lru_peek(NULL, NULL, NULL) == 0, "peek NULL handle");
    ASSERT(zenit_lru_peek(c, NULL, NULL) == 0, "peek NULL key");

    /* Contains with NULL */
    ASSERT(zenit_lru_contains(NULL, NULL) == 0, "contains NULL handle");
    ASSERT(zenit_lru_contains(c, NULL) == 0, "contains NULL key");

    /* Remove with NULL */
    ASSERT(zenit_lru_remove(NULL, NULL).error == ZENIT_ERROR_NULL, "remove NULL handle");
    ASSERT(zenit_lru_remove(c, NULL).error == ZENIT_ERROR_NULL, "remove NULL key");

    /* Clear with NULL */
    zenit_lru_clear(NULL);

    /* Count/capacity with NULL */
    ASSERT(zenit_lru_count(NULL) == 0, "count NULL");
    ASSERT(zenit_lru_capacity(NULL) == 0, "capacity NULL");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_create_invalid_params(void) {
    ASSERT(zenit_lru_create(0, sizeof(int), 4) == NULL, "zero key_size");
    ASSERT(zenit_lru_create(sizeof(int), 0, 4) == NULL, "zero value_size");
    ASSERT(zenit_lru_create(sizeof(int), sizeof(int), 0) == NULL, "zero capacity");
    PASS();
    return 0;
}

static int test_many_operations(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 64);
    ASSERT(c != NULL, "create");

    int N = 1000;

    /* Insert N entries */
    for (int i = 0; i < N; i++) {
        zenit_lru_put(c, &i, &i);
    }

    /* Cache should have 64 entries (capacity) */
    ASSERT(zenit_lru_count(c) == 64, "count = capacity after many puts");

    /* The most recent 64 keys should be present */
    for (int i = N - 64; i < N; i++) {
        int out = -1;
        int found = zenit_lru_get(c, &i, &out);
        ASSERT(found == 1, "recent key found");
        ASSERT(out == i, "recent key correct value");
    }

    /* The oldest keys should be evicted */
    for (int i = 0; i < N - 64; i++) {
        ASSERT(zenit_lru_get(c, &i, NULL) == 0, "old key evicted");
    }

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_allocator_variant(void) {
    zenit_allocator_t a = ZENIT_ALLOCATOR_DEFAULT;
    zenit_lru_t *c = zenit_lru_create_with_allocator(sizeof(int), sizeof(int), 4, a);
    ASSERT(c != NULL, "create with allocator");

    int k = 1;
    int v = 2;
    ASSERT(zenit_lru_put(c, &k, &v).error == ZENIT_OK, "put with allocator cache");

    int out = 0;
    ASSERT(zenit_lru_get(c, &k, &out) == 1 && out == 2, "get with allocator cache");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_evict_with_evict_and_allocator(void) {
    zenit_allocator_t a = ZENIT_ALLOCATOR_DEFAULT;
    reset_evict();

    zenit_lru_t *c = zenit_lru_create_with_evict_and_allocator(
        sizeof(int), sizeof(int), 3, track_evict, NULL, a
    );
    ASSERT(c != NULL, "create with evict and allocator");

    int k;
    int v;
    k = 0; v = 10; zenit_lru_put(c, &k, &v);
    k = 1; v = 20; zenit_lru_put(c, &k, &v);
    k = 2; v = 30; zenit_lru_put(c, &k, &v);
    ASSERT(evict_count == 0, "no eviction before full");

    /* Insert 4th entry — should evict 0 */
    k = 3; v = 40; zenit_lru_put(c, &k, &v);
    ASSERT(evict_count == 1, "one eviction");
    ASSERT(evict_last_key == 0, "evicted key 0");
    ASSERT(evict_last_value == 10, "evicted value 10");

    /* Insert 5th — evict 1 */
    k = 4; v = 50; zenit_lru_put(c, &k, &v);
    ASSERT(evict_count == 2, "two evictions");
    ASSERT(evict_last_key == 1, "evicted key 1");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_evict_callback_on_destroy(void) {
    reset_evict();
    zenit_lru_t *c = zenit_lru_create_with_evict(
        sizeof(int), sizeof(int), 3, track_evict, NULL
    );
    ASSERT(c != NULL, "create");

    for (int i = 0; i < 3; i++) {
        zenit_lru_put(c, &i, &i);
    }

    evict_count = 0; /* reset before destroy */
    zenit_lru_destroy(c);
    ASSERT(evict_count == 3, "evict callback called on destroy");
    PASS();
    return 0;
}

static int test_remove_does_not_trigger_evict(void) {
    reset_evict();
    zenit_lru_t *c = zenit_lru_create_with_evict(
        sizeof(int), sizeof(int), 3, track_evict, NULL
    );
    ASSERT(c != NULL, "create");

    int k = 0;
    zenit_lru_put(c, &k, &k);
    zenit_lru_remove(c, &k);
    ASSERT(evict_count == 0, "remove does not call evict callback");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_get_promotes_to_mru(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 3);
    int k;
    int v;

    /* Insert 0, 1, 2.  LRU order: 2 (MRU), 1, 0 (LRU) */
    k = 0; v = 0; zenit_lru_put(c, &k, &v);
    k = 1; v = 1; zenit_lru_put(c, &k, &v);
    k = 2; v = 2; zenit_lru_put(c, &k, &v);

    /* Get 0 — promotes.  Order: 0 (MRU), 2, 1 (LRU) */
    int out = -1;
    zenit_lru_get(c, &(int){0}, &out);

    /* Insert 3 — should evict 1 (LRU), not 0 */
    k = 3; v = 3; zenit_lru_put(c, &k, &v);

    ASSERT(zenit_lru_get(c, &(int){0}, NULL) == 1, "0 present after promotion");
    ASSERT(zenit_lru_get(c, &(int){2}, NULL) == 1, "2 present");
    /* 1 was not promoted and is the LRU — should be evicted */
    ASSERT(zenit_lru_get(c, &(int){1}, NULL) == 0, "1 evicted after promotion");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_large_entries(void) {
    typedef struct {
        char data[128];
        int id;
    } large_t;

    zenit_lru_t *c = zenit_lru_create(sizeof(large_t), sizeof(large_t), 8);
    ASSERT(c != NULL, "create large");

    for (int i = 0; i < 8; i++) {
        large_t key;
        large_t value;
        key.id = i;
        memset(key.data, 'A' + i, 128);
        value.id = i * 10;
        memset(value.data, 'a' + i, 128);
        zenit_lru_put(c, &key, &value);
    }

    /* Check all entries */
    for (int i = 0; i < 8; i++) {
        large_t key;
        key.id = i;
        memset(key.data, 'A' + i, 128);

        large_t out;
        int found = zenit_lru_get(c, &key, &out);
        ASSERT(found == 1, "large entry found");
        ASSERT(out.id == i * 10, "large entry value correct");
    }

    /* Eviction should work with large entries */
    large_t k9;
    large_t v9;
    k9.id = 9; memset(k9.data, 'J', 128);
    v9.id = 90; memset(v9.data, 'j', 128);
    zenit_lru_put(c, &k9, &v9);

    large_t k0;
    k0.id = 0; memset(k0.data, 'A', 128);
    ASSERT(zenit_lru_get(c, &k0, NULL) == 0, "old entry evicted for large");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_put_get_out_value_null(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    int k = 42;
    int v = 100;

    /* put, then get with NULL out_value to check hit/miss only */
    zenit_lru_put(c, &k, &v);
    ASSERT(zenit_lru_get(c, &k, NULL) == 1, "get with NULL out_value hit");
    ASSERT(zenit_lru_get(c, &(int){99}, NULL) == 0, "get with NULL out_value miss");

    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_peek_null_cache(void) {
    ASSERT(zenit_lru_peek(NULL, NULL, NULL) == 0, "peek NULL cache");
    ASSERT(zenit_lru_peek(NULL, &(int){0}, NULL) == 0, "peek NULL cache with key");
    PASS();
    return 0;
}

static int test_clear_with_evict(void) {
    reset_evict();
    zenit_lru_t *c = zenit_lru_create_with_evict(sizeof(int), sizeof(int), 4, track_evict, NULL);
    ASSERT(c != NULL, "create with evict");
    for (int i = 0; i < 3; i++) {
        zenit_lru_put(c, &i, &i);
    }
    ASSERT(evict_count == 0, "no evictions on put before full");
    evict_count = 0;
    zenit_lru_clear(c);
    ASSERT(evict_count == 3, "evict callback called 3 times on clear");
    ASSERT(zenit_lru_count(c) == 0, "count 0 after clear");
    int k = 42;
    zenit_lru_put(c, &k, &k);
    ASSERT(zenit_lru_count(c) == 1, "count 1 after clear+put");
    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_remove_null_key(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    ASSERT(c != NULL, "create");
    ASSERT(zenit_lru_remove(c, NULL).error == ZENIT_ERROR_NULL, "remove NULL key");
    ASSERT(zenit_lru_remove(NULL, &(int){0}).error == ZENIT_ERROR_NULL, "remove NULL cache");
    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_contains_null_key(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    ASSERT(c != NULL, "create");
    ASSERT(zenit_lru_contains(c, NULL) == 0, "contains NULL key");
    ASSERT(zenit_lru_contains(NULL, &(int){0}) == 0, "contains NULL cache");
    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_put_null_params(void) {
    zenit_lru_t *c = zenit_lru_create(sizeof(int), sizeof(int), 4);
    ASSERT(c != NULL, "create");
    ASSERT(zenit_lru_put(NULL, NULL, NULL).error == ZENIT_ERROR_NULL, "put NULL all");
    ASSERT(zenit_lru_put(c, NULL, &(int){0}).error == ZENIT_ERROR_NULL, "put NULL key");
    ASSERT(zenit_lru_put(c, &(int){0}, NULL).error == ZENIT_ERROR_NULL, "put NULL value");
    zenit_lru_destroy(c);
    PASS();
    return 0;
}

static int test_alloc_array_zero(void) {
    ASSERT(zenit_lru_create(0, sizeof(int), 4) == NULL, "alloc_array zero key_size");
    ASSERT(zenit_lru_create(sizeof(int), 0, 4) == NULL, "alloc_array zero value_size");
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        &test_create_destroy,
        &test_put_get_hit,
        &test_get_miss,
        &test_put_get_miss_after_evict,
        &test_peek_does_not_change_order,
        &test_update_existing_key,
        &test_remove,
        &test_contains,
        &test_clear,
        &test_null_params,
        &test_create_invalid_params,
        &test_many_operations,
        &test_allocator_variant,
        &test_evict_with_evict_and_allocator,
        &test_evict_callback_on_destroy,
        &test_remove_does_not_trigger_evict,
        &test_get_promotes_to_mru,
        &test_large_entries,
        &test_put_get_out_value_null,
        &test_peek_null_cache,
        &test_clear_with_evict,
        &test_remove_null_key,
        &test_contains_null_key,
        &test_put_null_params,
        &test_alloc_array_zero,
    };
    const char *names[] = {
        "create_destroy",
        "put_get_hit",
        "get_miss",
        "put_get_miss_after_evict",
        "peek_does_not_change_order",
        "update_existing_key",
        "remove",
        "contains",
        "clear",
        "null_params",
        "create_invalid_params",
        "many_operations",
        "allocator_variant",
        "evict_with_evict_and_allocator",
        "evict_callback_on_destroy",
        "remove_does_not_trigger_evict",
        "get_promotes_to_mru",
        "large_entries",
        "put_get_out_value_null",
        "peek_null_cache",
        "clear_with_evict",
        "remove_null_key",
        "contains_null_key",
        "put_null_params",
        "alloc_array_zero",
    };
    ZENIT_RUN_TESTS("lru", tests, names);
}
