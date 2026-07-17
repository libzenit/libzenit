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

#include <libzenit/bitset.h>
#include <string.h>

#include "test_runner.h"

/* ─── Test: create / destroy ─── */
static int test_create_destroy(void) {
    TEST("create/destroy");
    zenit_bitset_t *bs = zenit_bitset_create(64);
    ASSERT(bs != NULL, "expected non-NULL");
    ASSERT(zenit_bitset_capacity(bs) == 64, "capacity should be 64");
    ASSERT(zenit_bitset_count(bs) == 0, "count should be 0");
    zenit_bitset_destroy(bs);
    zenit_bitset_destroy(NULL);
    PASS();
    return 0;
}

/* ─── Test: create with zero bits ─── */
static int test_create_zero(void) {
    TEST("create with zero bits");
    zenit_bitset_t *bs = zenit_bitset_create(0);
    ASSERT(bs != NULL, "expected non-NULL");
    ASSERT(zenit_bitset_capacity(bs) == 0, "capacity should be 0");
    ASSERT(zenit_bitset_count(bs) == 0, "count should be 0");
    ASSERT(zenit_bitset_test(bs, 0) == 0, "test(0) should be 0");
    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: set and test single bit ─── */
static int test_set_test(void) {
    TEST("set/test single bit");
    zenit_bitset_t *bs = zenit_bitset_create(64);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_test(bs, 0) == 0, "bit 0 should be 0 initially");
    ASSERT(zenit_bitset_set(bs, 0).error == ZENIT_OK, "set bit 0");
    ASSERT(zenit_bitset_test(bs, 0) == 1, "bit 0 should be 1");

    ASSERT(zenit_bitset_test(bs, 63) == 0, "bit 63 should be 0 initially");
    ASSERT(zenit_bitset_set(bs, 63).error == ZENIT_OK, "set bit 63");
    ASSERT(zenit_bitset_test(bs, 63) == 1, "bit 63 should be 1");

    /* Verify bit 1 is still 0 */
    ASSERT(zenit_bitset_test(bs, 1) == 0, "bit 1 should be 0");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: set/clear/test ─── */
static int test_set_clear(void) {
    TEST("set/clear/test");
    zenit_bitset_t *bs = zenit_bitset_create(8);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_set(bs, 4).error == ZENIT_OK, "set bit 4");
    ASSERT(zenit_bitset_test(bs, 4) == 1, "bit 4 should be 1");
    ASSERT(zenit_bitset_clear(bs, 4).error == ZENIT_OK, "clear bit 4");
    ASSERT(zenit_bitset_test(bs, 4) == 0, "bit 4 should be 0 after clear");

    /* Clear an already-clear bit should be safe */
    ASSERT(zenit_bitset_clear(bs, 4).error == ZENIT_OK, "clear already clear bit");
    ASSERT(zenit_bitset_test(bs, 4) == 0, "bit 4 should still be 0");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: toggle ─── */
static int test_toggle(void) {
    TEST("toggle");
    zenit_bitset_t *bs = zenit_bitset_create(16);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_test(bs, 7) == 0, "bit 7 should be 0 initially");
    ASSERT(zenit_bitset_toggle(bs, 7).error == ZENIT_OK, "toggle bit 7");
    ASSERT(zenit_bitset_test(bs, 7) == 1, "bit 7 should be 1 after toggle");
    ASSERT(zenit_bitset_toggle(bs, 7).error == ZENIT_OK, "toggle bit 7 again");
    ASSERT(zenit_bitset_test(bs, 7) == 0, "bit 7 should be 0 after second toggle");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: multiple bits ─── */
static int test_multiple_bits(void) {
    TEST("multiple bits");
    zenit_bitset_t *bs = zenit_bitset_create(128);
    ASSERT(bs != NULL, "expected non-NULL");

    /* Set every 10th bit */
    for (size_t i = 0; i < 128; i += 10) {
        ASSERT(zenit_bitset_set(bs, i).error == ZENIT_OK, "set multiple");
    }

    /* Verify every 10th bit and check neighbours */
    for (size_t i = 0; i < 128; i++) {
        int expected = (i % 10 == 0) ? 1 : 0;
        ASSERT(zenit_bitset_test(bs, i) == expected, "bit check");
    }

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: set_all and verify ─── */
static int test_set_all(void) {
    TEST("set_all");
    zenit_bitset_t *bs = zenit_bitset_create(64);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_set_all(bs).error == ZENIT_OK, "set_all");
    ASSERT(zenit_bitset_count(bs) == 64, "count should be 64 after set_all");

    for (size_t i = 0; i < 64; i++) {
        ASSERT(zenit_bitset_test(bs, i) == 1, "all bits should be 1");
    }

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: clear_all ─── */
static int test_clear_all(void) {
    TEST("clear_all");
    zenit_bitset_t *bs = zenit_bitset_create(32);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_set_all(bs).error == ZENIT_OK, "set_all");
    ASSERT(zenit_bitset_count(bs) == 32, "count should be 32");

    ASSERT(zenit_bitset_clear_all(bs).error == ZENIT_OK, "clear_all");
    ASSERT(zenit_bitset_count(bs) == 0, "count should be 0 after clear_all");

    for (size_t i = 0; i < 32; i++) {
        ASSERT(zenit_bitset_test(bs, i) == 0, "all bits should be 0");
    }

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: count ─── */
static int test_count(void) {
    TEST("count");
    zenit_bitset_t *bs = zenit_bitset_create(100);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_count(bs) == 0, "count should be 0 initially");

    ASSERT(zenit_bitset_set(bs, 0).error == ZENIT_OK, "set bit 0");
    ASSERT(zenit_bitset_set(bs, 1).error == ZENIT_OK, "set bit 1");
    ASSERT(zenit_bitset_set(bs, 2).error == ZENIT_OK, "set bit 2");
    ASSERT(zenit_bitset_count(bs) == 3, "count should be 3");

    ASSERT(zenit_bitset_clear(bs, 1).error == ZENIT_OK, "clear bit 1");
    ASSERT(zenit_bitset_count(bs) == 2, "count should be 2");

    /* Toggle a 1 to 0 and a 0 to 1 */
    ASSERT(zenit_bitset_toggle(bs, 0).error == ZENIT_OK, "toggle bit 0 (1→0)");
    ASSERT(zenit_bitset_toggle(bs, 99).error == ZENIT_OK, "toggle bit 99 (0→1)");
    ASSERT(zenit_bitset_count(bs) == 2, "count should still be 2 after toggle pair");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: resize (grow) ─── */
static int test_resize_grow(void) {
    TEST("resize grow");
    zenit_bitset_t *bs = zenit_bitset_create(8);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_capacity(bs) == 8, "initial capacity should be 8");

    ASSERT(zenit_bitset_resize(bs, 64).error == ZENIT_OK, "resize to 64");
    ASSERT(zenit_bitset_capacity(bs) == 64, "capacity should be 64");

    /* Set a bit beyond original size */
    ASSERT(zenit_bitset_set(bs, 63).error == ZENIT_OK, "set bit 63 after resize");
    ASSERT(zenit_bitset_test(bs, 63) == 1, "bit 63 should be 1");
    ASSERT(zenit_bitset_test(bs, 7) == 0, "bit 7 should still be 0");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: resize (shrink) ─── */
static int test_resize_shrink(void) {
    TEST("resize shrink");
    zenit_bitset_t *bs = zenit_bitset_create(128);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_set(bs, 100).error == ZENIT_OK, "set bit 100");
    ASSERT(zenit_bitset_resize(bs, 32).error == ZENIT_OK, "shrink to 32");
    ASSERT(zenit_bitset_capacity(bs) == 32, "capacity should be 32");
    /* Bit 100 is now out of range — test should return 0 */
    ASSERT(zenit_bitset_test(bs, 100) == 0, "bit 100 should be out of range");

    /* Resize back up — old data beyond 32 is gone */
    ASSERT(zenit_bitset_resize(bs, 128).error == ZENIT_OK, "grow back to 128");
    ASSERT(zenit_bitset_test(bs, 100) == 0, "bit 100 should still be 0 after re-grow");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: resize to zero ─── */
static int test_resize_zero(void) {
    TEST("resize to zero");
    zenit_bitset_t *bs = zenit_bitset_create(64);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_set(bs, 10).error == ZENIT_OK, "set bit 10");
    ASSERT(zenit_bitset_resize(bs, 0).error == ZENIT_OK, "resize to 0");
    ASSERT(zenit_bitset_capacity(bs) == 0, "capacity should be 0");
    ASSERT(zenit_bitset_count(bs) == 0, "count should be 0");

    /* Resize back and verify old data is gone */
    ASSERT(zenit_bitset_resize(bs, 64).error == ZENIT_OK, "resize back to 64");
    ASSERT(zenit_bitset_test(bs, 10) == 0, "bit 10 should be 0 after resize cycle");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: auto-grow on set beyond capacity ─── */
static int test_auto_grow_set(void) {
    TEST("auto-grow on set");
    zenit_bitset_t *bs = zenit_bitset_create(8);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_set(bs, 100).error == ZENIT_OK, "set bit 100 (auto-grow)");
    ASSERT(zenit_bitset_capacity(bs) > 100, "capacity should be > 100 after auto-grow");
    ASSERT(zenit_bitset_test(bs, 100) == 1, "bit 100 should be 1");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: auto-grow on clear beyond capacity ─── */
static int test_auto_grow_clear(void) {
    TEST("auto-grow on clear");
    zenit_bitset_t *bs = zenit_bitset_create(4);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_clear(bs, 100).error == ZENIT_OK, "clear bit 100 (auto-grow)");
    ASSERT(zenit_bitset_capacity(bs) > 100, "capacity should be > 100");
    ASSERT(zenit_bitset_test(bs, 100) == 0, "bit 100 should be 0");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: auto-grow on toggle beyond capacity ─── */
static int test_auto_grow_toggle(void) {
    TEST("auto-grow on toggle");
    zenit_bitset_t *bs = zenit_bitset_create(4);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_toggle(bs, 200).error == ZENIT_OK, "toggle bit 200 (auto-grow)");
    ASSERT(zenit_bitset_capacity(bs) > 200, "capacity should be > 200");
    ASSERT(zenit_bitset_test(bs, 200) == 1, "bit 200 should be 1 after toggle");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: edge case — position 0 ─── */
static int test_pos_zero(void) {
    TEST("position 0");
    zenit_bitset_t *bs = zenit_bitset_create(1);
    ASSERT(bs != NULL, "expected non-NULL");

    ASSERT(zenit_bitset_test(bs, 0) == 0, "bit 0 should be 0");
    ASSERT(zenit_bitset_set(bs, 0).error == ZENIT_OK, "set bit 0");
    ASSERT(zenit_bitset_test(bs, 0) == 1, "bit 0 should be 1");
    ASSERT(zenit_bitset_clear(bs, 0).error == ZENIT_OK, "clear bit 0");
    ASSERT(zenit_bitset_test(bs, 0) == 0, "bit 0 should be 0");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: NULL parameters ─── */
static int test_null_params(void) {
    TEST("NULL parameters");
    ASSERT(zenit_bitset_set(NULL, 0).error == ZENIT_ERROR_NULL, "set(NULL)");
    ASSERT(zenit_bitset_clear(NULL, 0).error == ZENIT_ERROR_NULL, "clear(NULL)");
    ASSERT(zenit_bitset_toggle(NULL, 0).error == ZENIT_ERROR_NULL, "toggle(NULL)");
    ASSERT(zenit_bitset_test(NULL, 0) == 0, "test(NULL) should be 0");
    ASSERT(zenit_bitset_set_all(NULL).error == ZENIT_ERROR_NULL, "set_all(NULL)");
    ASSERT(zenit_bitset_clear_all(NULL).error == ZENIT_ERROR_NULL, "clear_all(NULL)");
    ASSERT(zenit_bitset_count(NULL) == 0, "count(NULL) should be 0");
    ASSERT(zenit_bitset_capacity(NULL) == 0, "capacity(NULL) should be 0");
    ASSERT(zenit_bitset_resize(NULL, 64).error == ZENIT_ERROR_NULL, "resize(NULL)");
    PASS();
    return 0;
}

/* ─── Test: auto-grow from zero capacity ─── */
static int test_auto_grow_from_zero(void) {
    TEST("auto-grow from zero");
    zenit_bitset_t *bs = zenit_bitset_create(0);
    ASSERT(bs != NULL, "expected non-NULL");
    ASSERT(zenit_bitset_capacity(bs) == 0, "capacity should be 0");

    ASSERT(zenit_bitset_set(bs, 10).error == ZENIT_OK, "set bit 10 (auto-grow from zero)");
    ASSERT(zenit_bitset_test(bs, 10) == 1, "bit 10 should be 1");
    ASSERT(zenit_bitset_capacity(bs) > 10, "capacity should have grown");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

/* ─── Test: large position (stress auto-grow) ─── */
static int test_large_position(void) {
    TEST("large position auto-grow");
    zenit_bitset_t *bs = zenit_bitset_create(8);
    ASSERT(bs != NULL, "expected non-NULL");

    size_t big = 10000;
    ASSERT(zenit_bitset_set(bs, big).error == ZENIT_OK, "set large position");
    ASSERT(zenit_bitset_test(bs, big) == 1, "large bit should be 1");
    ASSERT(zenit_bitset_capacity(bs) > big, "capacity should exceed large pos");
    ASSERT(zenit_bitset_count(bs) == 1, "count should be 1");

    /* Verify bits before and after are unaffected */
    ASSERT(zenit_bitset_test(bs, big - 1) == 0, "bit big-1 should be 0");
    ASSERT(zenit_bitset_test(bs, big + 1) == 0, "bit big+1 should be 0");

    zenit_bitset_destroy(bs);
    PASS();
    return 0;
}

int main(void) {
    TEST_ENTRY tests[] = {
        { test_create_destroy,   "create/destroy" },
        { test_create_zero,      "create with zero bits" },
        { test_set_test,         "set/test single bit" },
        { test_set_clear,        "set/clear/test" },
        { test_toggle,           "toggle" },
        { test_multiple_bits,    "multiple bits" },
        { test_set_all,          "set_all" },
        { test_clear_all,        "clear_all" },
        { test_count,            "count" },
        { test_resize_grow,      "resize grow" },
        { test_resize_shrink,    "resize shrink" },
        { test_resize_zero,      "resize to zero" },
        { test_auto_grow_set,    "auto-grow on set" },
        { test_auto_grow_clear,  "auto-grow on clear" },
        { test_auto_grow_toggle, "auto-grow on toggle" },
        { test_pos_zero,         "position 0" },
        { test_null_params,      "NULL parameters" },
        { test_auto_grow_from_zero, "auto-grow from zero" },
        { test_large_position,   "large position auto-grow" },
        { 0, 0 }
    };
    return test_run_all("bitset", tests);
}
