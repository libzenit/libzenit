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

#include <libzenit/timer.h>
#include "test_runner.h"

static int test_now(void) {
    TEST("now returns reasonable sec");
    zenit_time_t t = zenit_time_now();
    ASSERT(t.sec >= 0, "sec should be >= 0");
    PASS();
    return 0;
}

static int test_elapsed_positive(void) {
    TEST("elapsed positive after busy-wait");
    zenit_time_t t1 = zenit_time_now();
    volatile long long i = 0;
    while (i < 5000000) i++;
    zenit_time_t t2 = zenit_time_now();
    ASSERT(zenit_time_elapsed_s(t1, t2) > 0.0, "elapsed_s > 0");
    ASSERT(zenit_time_elapsed_ms(t1, t2) > 0.0, "elapsed_ms > 0");
    ASSERT(zenit_time_elapsed_us(t1, t2) > 0.0, "elapsed_us > 0");
    ASSERT(zenit_time_elapsed_ns(t1, t2) > 0.0, "elapsed_ns > 0");
    PASS();
    return 0;
}

static int test_elapsed_reasonable(void) {
    TEST("elapsed within reasonable range (< 1s)");
    zenit_time_t t1 = zenit_time_now();
    volatile long long i = 0;
    while (i < 1000000) i++;
    zenit_time_t t2 = zenit_time_now();
    double s = zenit_time_elapsed_s(t1, t2);
    ASSERT(s >= 0.0 && s < 1.0, "elapsed_s in [0, 1)");
    PASS();
    return 0;
}

static int test_add_basic(void) {
    TEST("add 1s + 1s = 2s");
    zenit_time_t a = { .sec = 1, .nsec = 0 };
    zenit_time_t b = { .sec = 1, .nsec = 0 };
    zenit_time_t r = zenit_time_add(a, b);
    ASSERT(r.sec == 2, "sec == 2");
    ASSERT(r.nsec == 0, "nsec == 0");
    PASS();
    return 0;
}

static int test_add_nsec_overflow(void) {
    TEST("add 500ms + 600ms = 1s100ms");
    zenit_time_t a = { .sec = 0, .nsec = 500000000 };
    zenit_time_t b = { .sec = 0, .nsec = 600000000 };
    zenit_time_t r = zenit_time_add(a, b);
    ASSERT(r.sec == 1, "sec == 1");
    ASSERT(r.nsec == 100000000, "nsec == 100000000");
    PASS();
    return 0;
}

static int test_sub_basic(void) {
    TEST("sub 2s - 1s = 1s");
    zenit_time_t a = { .sec = 2, .nsec = 0 };
    zenit_time_t b = { .sec = 1, .nsec = 0 };
    zenit_time_t r = zenit_time_sub(a, b);
    ASSERT(r.sec == 1, "sec == 1");
    ASSERT(r.nsec == 0, "nsec == 0");
    PASS();
    return 0;
}

static int test_sub_nsec_borrow(void) {
    TEST("sub with nsec borrow");
    zenit_time_t a = { .sec = 2, .nsec = 0 };
    zenit_time_t b = { .sec = 0, .nsec = 500000000 };
    zenit_time_t r = zenit_time_sub(a, b);
    ASSERT(r.sec == 1, "sec == 1 after borrow");
    ASSERT(r.nsec == 500000000, "nsec == 500000000 after borrow");
    PASS();
    return 0;
}

static int test_cmp_equal(void) {
    TEST("cmp equal");
    zenit_time_t a = { .sec = 5, .nsec = 123 };
    zenit_time_t b = { .sec = 5, .nsec = 123 };
    ASSERT(zenit_time_cmp(a, b) == 0, "should be 0");
    PASS();
    return 0;
}

static int test_cmp_less(void) {
    TEST("cmp less");
    zenit_time_t a = { .sec = 1, .nsec = 0 };
    zenit_time_t b = { .sec = 2, .nsec = 0 };
    ASSERT(zenit_time_cmp(a, b) < 0, "a < b");
    PASS();
    return 0;
}

static int test_cmp_greater(void) {
    TEST("cmp greater");
    zenit_time_t a = { .sec = 5, .nsec = 100 };
    zenit_time_t b = { .sec = 5, .nsec = 50 };
    ASSERT(zenit_time_cmp(a, b) > 0, "a > b");
    PASS();
    return 0;
}

int main(void) {
    int (*tests[])(void) = {
        test_now,
        test_elapsed_positive,
        test_elapsed_reasonable,
        test_add_basic,
        test_add_nsec_overflow,
        test_sub_basic,
        test_sub_nsec_borrow,
        test_cmp_equal,
        test_cmp_less,
        test_cmp_greater,
    };
    const char *names[] = {
        "now returns reasonable sec",
        "elapsed positive after busy-wait",
        "elapsed within reasonable range",
        "add 1s + 1s = 2s",
        "add 500ms + 600ms = 1s100ms",
        "sub 2s - 1s = 1s",
        "sub with nsec borrow",
        "cmp equal",
        "cmp less",
        "cmp greater",
    };
    ZENIT_RUN_TESTS("timer", tests, names);
}
