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

#include <libzenit/bloom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
#define FAIL(msg) do { \
    fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    failures++; \
} while(0)

/* ─── Test: create / destroy (default allocator) ─── */
static int test_create_destroy(void) {
    zenit_bloom_t *bf = zenit_bloom_create(10000, 0.01);
    if (bf == NULL) { FAIL("create returned NULL"); return 1; }

    if (zenit_bloom_num_bits(bf) == 0) { FAIL("num_bits should be > 0"); return 1; }
    if (zenit_bloom_num_hashes(bf) == 0) { FAIL("num_hashes should be > 0"); return 1; }
    if (zenit_bloom_count(bf) != 0) { FAIL("new filter count should be 0"); return 1; }

    zenit_bloom_destroy(bf);
    zenit_bloom_destroy(NULL); /* must be safe */
    return 0;
}

/* ─── Test: create / destroy with allocator ─── */
static int test_create_destroy_with_allocator(void) {
    zenit_bloom_t *bf = zenit_bloom_create_with_allocator(10000, 0.01, ZENIT_ALLOCATOR_DEFAULT);
    if (bf == NULL) { FAIL("create_with_allocator returned NULL"); return 1; }

    if (zenit_bloom_num_bits(bf) == 0) { FAIL("num_bits should be > 0"); return 1; }
    if (zenit_bloom_count(bf) != 0) { FAIL("count should be 0"); return 1; }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: create_explicit / destroy ─── */
static int test_create_explicit(void) {
    zenit_bloom_t *bf = zenit_bloom_create_explicit(1024, 7);
    if (bf == NULL) { FAIL("create_explicit returned NULL"); return 1; }

    /* 1024 bits rounds up to 1024 (already multiple of 64) */
    if (zenit_bloom_num_bits(bf) < 1024) { FAIL("num_bits wrong"); return 1; }
    if (zenit_bloom_num_hashes(bf) != 7) { FAIL("num_hashes wrong"); return 1; }
    if (zenit_bloom_count(bf) != 0) { FAIL("count should be 0"); return 1; }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: create_explicit with allocator ─── */
static int test_create_explicit_with_allocator(void) {
    zenit_bloom_t *bf = zenit_bloom_create_explicit_with_allocator(1024, 7, ZENIT_ALLOCATOR_DEFAULT);
    if (bf == NULL) { FAIL("create_explicit_with_allocator returned NULL"); return 1; }

    if (zenit_bloom_num_hashes(bf) != 7) { FAIL("num_hashes wrong"); return 1; }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: insert / contains (exact match) ─── */
static int test_insert_contains(void) {
    zenit_bloom_t *bf = zenit_bloom_create(1000, 0.01);
    if (bf == NULL) { FAIL("create"); return 1; }

    const char *item = "hello";
    zenit_bloom_insert(bf, item, 5);

    if (!zenit_bloom_contains(bf, item, 5)) {
        FAIL("inserted item not found");
        return 1;
    }
    if (zenit_bloom_count(bf) != 1) {
        FAIL("count should be 1");
        return 1;
    }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: contains returns 0 for non-inserted item ─── */
static int test_not_contains(void) {
    zenit_bloom_t *bf = zenit_bloom_create(1000, 0.001);
    if (bf == NULL) { FAIL("create"); return 1; }

    const char *a = "alpha";
    const char *b = "beta";
    zenit_bloom_insert(bf, a, 5);

    /* "beta" was not inserted; with 0.001 FPR, nearly always returns 0 */
    if (zenit_bloom_contains(bf, b, 4)) {
        FAIL("non-inserted item reported as present (false positive)");
        return 1;
    }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: clear resets all bits ─── */
static int test_clear(void) {
    zenit_bloom_t *bf = zenit_bloom_create(1000, 0.01);
    if (bf == NULL) { FAIL("create"); return 1; }

    const char *item = "hello";
    zenit_bloom_insert(bf, item, 5);
    if (zenit_bloom_count(bf) != 1) { FAIL("count should be 1 before clear"); return 1; }

    zenit_bloom_clear(bf);
    if (zenit_bloom_count(bf) != 0) { FAIL("count should be 0 after clear"); return 1; }
    /* After clear, no item should be found */
    if (zenit_bloom_contains(bf, item, 5)) {
        FAIL("item still found after clear");
        return 1;
    }

    /* Should be able to insert again after clear */
    zenit_bloom_insert(bf, item, 5);
    if (!zenit_bloom_contains(bf, item, 5)) {
        FAIL("item not found after reinsert");
        return 1;
    }
    if (zenit_bloom_count(bf) != 1) { FAIL("count should be 1 after reinsert"); return 1; }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: false_positive_rate returns expected rate ─── */
static int test_false_positive_rate(void) {
    zenit_bloom_t *bf = zenit_bloom_create(10000, 0.05);
    if (bf == NULL) { FAIL("create"); return 1; }

    double fpr = zenit_bloom_false_positive_rate(bf);
    /* The returned value should be close to 0.05 (may differ slightly due
     * to rounding of num_bits/num_hashes to integers and 64-bit alignment) */
    if (fpr <= 0.0 || fpr > 0.06) {
        FAIL("false positive rate out of expected range");
        return 1;
    }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: explicit false positive rate ─── */
static int test_explicit_fpr(void) {
    /* For 7 hash functions, FPR estimate = 0.5^7 ≈ 0.0078125 */
    zenit_bloom_t *bf = zenit_bloom_create_explicit(1024, 7);
    if (bf == NULL) { FAIL("create_explicit"); return 1; }

    double fpr = zenit_bloom_false_positive_rate(bf);
    double expected = 0.0078125;
    if (fpr < expected * 0.9 || fpr > expected * 1.1) {
        FAIL("explicit FPR out of range");
        return 1;
    }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: NULL params safe ─── */
static int test_null_params(void) {
    /* All NULL calls must not crash */
    zenit_bloom_destroy(NULL);
    zenit_bloom_clear(NULL);
    zenit_bloom_insert(NULL, "x", 1);

    zenit_bloom_t *bf = zenit_bloom_create(100, 0.01);
    if (bf == NULL) { FAIL("create"); return 1; }

    /* insert/contains with NULL data */
    zenit_bloom_insert(bf, NULL, 0);
    if (zenit_bloom_contains(bf, NULL, 0) != 0) {
        FAIL("contains(NULL) should return 0");
        return 1;
    }

    /* getters on NULL */
    if (zenit_bloom_false_positive_rate(NULL) != 0.0) { FAIL("fpr(NULL) should be 0.0"); return 1; }
    if (zenit_bloom_count(NULL) != 0) { FAIL("count(NULL) should be 0"); return 1; }
    if (zenit_bloom_num_bits(NULL) != 0) { FAIL("num_bits(NULL) should be 0"); return 1; }
    if (zenit_bloom_num_hashes(NULL) != 0) { FAIL("num_hashes(NULL) should be 0"); return 1; }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: many insertions ─── */
static int test_many_insertions(void) {
    zenit_bloom_t *bf = zenit_bloom_create(10000, 0.01);
    if (bf == NULL) { FAIL("create"); return 1; }

    /* Insert 5000 items (well under capacity) */
    for (size_t i = 0; i < 5000; i++) {
        unsigned char item[8];
        memset(item, (int)i, 8);
        zenit_bloom_insert(bf, item, 8);
    }

    if (zenit_bloom_count(bf) != 5000) {
        FAIL("count should be 5000");
        return 1;
    }

    /* Verify all inserted items are found */
    for (size_t i = 0; i < 5000; i++) {
        unsigned char item[8];
        memset(item, (int)i, 8);
        if (!zenit_bloom_contains(bf, item, 8)) {
            FAIL("inserted item not found in large test");
            return 1;
        }
    }

    zenit_bloom_destroy(bf);
    return 0;
}

/* ─── Test: create with zero capacity returns NULL ─── */
static int test_zero_capacity(void) {
    zenit_bloom_t *bf = zenit_bloom_create(0, 0.01);
    if (bf != NULL) {
        FAIL("expected NULL for capacity=0");
        zenit_bloom_destroy(bf);
        return 1;
    }
    return 0;
}

/* ─── Test: create with invalid rate returns NULL ─── */
static int test_invalid_rate(void) {
    zenit_bloom_t *bf1 = zenit_bloom_create(100, 0.0);
    zenit_bloom_t *bf2 = zenit_bloom_create(100, 1.0);
    zenit_bloom_t *bf3 = zenit_bloom_create(100, -0.1);
    if (bf1 != NULL || bf2 != NULL || bf3 != NULL) {
        FAIL("expected NULL for invalid rate");
        zenit_bloom_destroy(bf1);
        zenit_bloom_destroy(bf2);
        zenit_bloom_destroy(bf3);
        return 1;
    }
    return 0;
}

/* ─── Test: create_explicit with zero params returns NULL ─── */
static int test_explicit_zero(void) {
    zenit_bloom_t *bf1 = zenit_bloom_create_explicit(0, 7);
    zenit_bloom_t *bf2 = zenit_bloom_create_explicit(1024, 0);
    if (bf1 != NULL || bf2 != NULL) {
        FAIL("expected NULL for zero params");
        zenit_bloom_destroy(bf1);
        zenit_bloom_destroy(bf2);
        return 1;
    }
    return 0;
}

/* ─── Test: create_explicit rounds bits up ─── */
static int test_explicit_rounding(void) {
    /* 100 bits should round up to 128 (2 * 64) */
    zenit_bloom_t *bf = zenit_bloom_create_explicit(100, 3);
    if (bf == NULL) { FAIL("create_explicit"); return 1; }

    if (zenit_bloom_num_bits(bf) != 128) {
        FAIL("num_bits should be 128 after rounding up from 100");
        return 1;
    }

    zenit_bloom_destroy(bf);
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_destroy();
    ret |= test_create_destroy_with_allocator();
    ret |= test_create_explicit();
    ret |= test_create_explicit_with_allocator();
    ret |= test_insert_contains();
    ret |= test_not_contains();
    ret |= test_clear();
    ret |= test_false_positive_rate();
    ret |= test_explicit_fpr();
    ret |= test_null_params();
    ret |= test_many_insertions();
    ret |= test_zero_capacity();
    ret |= test_invalid_rate();
    ret |= test_explicit_zero();
    ret |= test_explicit_rounding();

    if (failures > 0 || ret != 0) {
        fprintf(stderr, "FAIL: %d test(s) had errors\n", failures);
        return 1;
    }

    printf("PASS: bloom filter tests\n");
    return 0;
}
