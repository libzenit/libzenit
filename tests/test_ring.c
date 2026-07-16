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

#include <libzenit/ring.h>
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
    zenit_ring_t *r = zenit_ring_create(1024);
    if (r == NULL) { FAIL("create returned NULL"); return 1; }

    if (zenit_ring_capacity(r) != 1024) { FAIL("wrong capacity"); return 1; }
    if (zenit_ring_count(r) != 0) { FAIL("new ring not empty"); return 1; }

    zenit_ring_destroy(r);
    zenit_ring_destroy(NULL); /* must be safe */
    return 0;
}

/* ─── Test: create with zero capacity ─── */
static int test_create_zero(void) {
    zenit_ring_t *r = zenit_ring_create(0);
    if (r != NULL) {
        FAIL("expected NULL for capacity=0");
        zenit_ring_destroy(r);
        return 1;
    }
    return 0;
}

/* ─── Test: push / pop one byte ─── */
static int test_push_pop_one(void) {
    zenit_ring_t *r = zenit_ring_create(16);
    if (r == NULL) { FAIL("create"); return 1; }

    const char in = 'A';
    if (zenit_ring_push(r, &in, 1).error != ZENIT_OK) {
        FAIL("push returned error"); return 1;
    }
    if (zenit_ring_count(r) != 1) { FAIL("count should be 1"); return 1; }

    char out = 0;
    size_t sz = 1;
    if (zenit_ring_pop(r, &out, &sz).error != ZENIT_OK) {
        FAIL("pop returned error"); return 1;
    }
    if (sz != 1) { FAIL("pop size should be 1"); return 1; }
    if (out != 'A') { FAIL("data mismatch"); return 1; }
    if (zenit_ring_count(r) != 0) { FAIL("count should be 0 after pop"); return 1; }

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: push until full, then error ─── */
static int test_push_full(void) {
    zenit_ring_t *r = zenit_ring_create(8);
    if (r == NULL) { FAIL("create"); return 1; }

    /* Fill with 8 bytes */
    for (int i = 0; i < 8; i++) {
        char c = (char)i;
        if (zenit_ring_push(r, &c, 1).error != ZENIT_OK) {
            FAIL("unexpected full before 8 bytes"); return 1;
        }
    }
    if (zenit_ring_count(r) != 8) { FAIL("count should be 8"); return 1; }

    /* 9th byte should fail */
    char extra = 'X';
    if (zenit_ring_push(r, &extra, 1).error != ZENIT_ERROR_FULL) {
        FAIL("expected FULL at capacity"); return 1;
    }

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: pop from empty ─── */
static int test_pop_empty(void) {
    zenit_ring_t *r = zenit_ring_create(16);
    if (r == NULL) { FAIL("create"); return 1; }

    size_t sz = 1;
    char buf;
    zenit_result_t res = zenit_ring_pop(r, &buf, &sz);
    if (res.error != ZENIT_ERROR_EMPTY) {
        FAIL("expected EMPTY on pop from empty"); return 1;
    }

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: peek does not consume ─── */
static int test_peek(void) {
    zenit_ring_t *r = zenit_ring_create(16);
    if (r == NULL) { FAIL("create"); return 1; }

    const char *msg = "hello";
    if (zenit_ring_push(r, msg, 5).error != ZENIT_OK) {
        FAIL("push"); return 1;
    }

    char buf[8] = {0};
    size_t sz = 8;
    if (zenit_ring_peek(r, buf, &sz).error != ZENIT_OK) {
        FAIL("peek returned error"); return 1;
    }
    if (sz != 5) { FAIL("peek size wrong"); return 1; }
    if (memcmp(buf, "hello", 5) != 0) { FAIL("peek data wrong"); return 1; }

    /* Data should still be there — pop now */
    sz = 8;
    if (zenit_ring_pop(r, buf, &sz).error != ZENIT_OK) {
        FAIL("pop after peek"); return 1;
    }
    if (sz != 5) { FAIL("pop after peek size"); return 1; }
    if (zenit_ring_count(r) != 0) { FAIL("should be empty after pop"); return 1; }

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: wrap-around behaviour ─── */
static int test_wrap_around(void) {
    /* Create a 4-byte ring. Push 3 bytes, pop 2, push 2 (which wraps). */
    zenit_ring_t *r = zenit_ring_create(4);
    if (r == NULL) { FAIL("create"); return 1; }

    /* Push A, B, C → count=3, head=3 */
    if (zenit_ring_push(r, "ABC", 3).error != ZENIT_OK) { FAIL("push ABC"); return 1; }

    /* Pop 2 bytes → removes A, B. count=1, tail=2 */
    size_t sz = 2;
    char buf[4] = {0};
    if (zenit_ring_pop(r, buf, &sz).error != ZENIT_OK) { FAIL("pop 2"); return 1; }
    if (sz != 2 || buf[0] != 'A' || buf[1] != 'B') { FAIL("pop data"); return 1; }

    /* Push D, E → wraps: D goes to pos 3, E wraps to pos 0. count=3 */
    if (zenit_ring_push(r, "DE", 2).error != ZENIT_OK) { FAIL("push DE"); return 1; }

    /* Pop all 3 bytes → should get C, D, E */
    sz = 4;
    memset(buf, 0, 4);
    if (zenit_ring_pop(r, buf, &sz).error != ZENIT_OK) { FAIL("pop remaining"); return 1; }
    if (sz != 3) { FAIL("expected 3 bytes"); return 1; }
    if (buf[0] != 'C' || buf[1] != 'D' || buf[2] != 'E') {
        FAIL("wrap-around data mismatch");
        return 1;
    }

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: clear ─── */
static int test_clear(void) {
    zenit_ring_t *r = zenit_ring_create(16);
    if (r == NULL) { FAIL("create"); return 1; }

    if (zenit_ring_push(r, "data", 4).error != ZENIT_OK) { FAIL("push"); return 1; }
    zenit_ring_clear(r);
    if (zenit_ring_count(r) != 0) { FAIL("count should be 0 after clear"); return 1; }

    /* Should be able to push again after clear */
    if (zenit_ring_push(r, "new", 3).error != ZENIT_OK) { FAIL("push after clear"); return 1; }
    if (zenit_ring_count(r) != 3) { FAIL("count should be 3"); return 1; }

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: NULL / edge cases for ring functions ─── */
static int test_edge_cases(void) {
    zenit_ring_t *r = zenit_ring_create(16);
    if (r == NULL) { FAIL("create"); return 1; }

    /* push with NULL ring */
    if (zenit_ring_push(NULL, "x", 1).error != ZENIT_ERROR_NULL) {
        FAIL("push(NULL) should return NULL error"); return 1;
    }
    /* push with NULL data */
    if (zenit_ring_push(r, NULL, 1).error != ZENIT_ERROR_NULL) {
        FAIL("push(data=NULL) should return NULL error"); return 1;
    }
    /* push with size=0 */
    if (zenit_ring_push(r, "x", 0).error != ZENIT_ERROR_PARAM) {
        FAIL("push(size=0) should return PARAM error"); return 1;
    }

    /* pop with NULL ring */
    size_t sz = 1;
    char buf;
    if (zenit_ring_pop(NULL, &buf, &sz).error != ZENIT_ERROR_NULL) {
        FAIL("pop(NULL) should return NULL error"); return 1;
    }
    /* pop with NULL data */
    if (zenit_ring_pop(r, NULL, &sz).error != ZENIT_ERROR_NULL) {
        FAIL("pop(data=NULL) should return NULL error"); return 1;
    }
    /* pop with NULL size */
    if (zenit_ring_pop(r, &buf, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("pop(size=NULL) should return NULL error"); return 1;
    }
    /* pop with *size=0 */
    sz = 0;
    if (zenit_ring_pop(r, &buf, &sz).error != ZENIT_ERROR_PARAM) {
        FAIL("pop(*size=0) should return PARAM error"); return 1;
    }

    /* peek with NULL ring */
    sz = 1;
    if (zenit_ring_peek(NULL, &buf, &sz).error != ZENIT_ERROR_NULL) {
        FAIL("peek(NULL) should return NULL error"); return 1;
    }

    /* count/capacity with NULL */
    if (zenit_ring_count(NULL) != 0) { FAIL("count(NULL) should be 0"); return 1; }
    if (zenit_ring_capacity(NULL) != 0) { FAIL("capacity(NULL) should be 0"); return 1; }

    /* clear(NULL) should be safe */
    zenit_ring_clear(NULL);

    zenit_ring_destroy(r);
    return 0;
}

/* ─── Test: push/pop larger chunks ─── */
static int test_chunks(void) {
    zenit_ring_t *r = zenit_ring_create(256);
    if (r == NULL) { FAIL("create"); return 1; }

    unsigned char in[128];
    for (size_t i = 0; i < 128; i++) in[i] = (unsigned char)i;

    /* Push 128 bytes */
    if (zenit_ring_push(r, in, 128).error != ZENIT_OK) { FAIL("push 128"); return 1; }
    if (zenit_ring_count(r) != 128) { FAIL("count 128"); return 1; }

    /* Pop 64 bytes */
    unsigned char out[64];
    size_t sz = 64;
    if (zenit_ring_pop(r, out, &sz).error != ZENIT_OK) { FAIL("pop 64"); return 1; }
    if (sz != 64) { FAIL("pop 64 size"); return 1; }
    if (memcmp(in, out, 64) != 0) { FAIL("first 64 mismatch"); return 1; }

    /* Push another 128 bytes (should fill to 192, not full) */
    if (zenit_ring_push(r, in, 128).error != ZENIT_OK) { FAIL("push 128 again"); return 1; }
    if (zenit_ring_count(r) != 192) { FAIL("count 192"); return 1; }

    /* Try to push 128 more — only 64 slots free */
    if (zenit_ring_push(r, in, 128).error != ZENIT_ERROR_FULL) {
        FAIL("expected FULL"); return 1;
    }

    /* Pop remaining 192 and verify second half */
    unsigned char all[192];
    sz = 192;
    if (zenit_ring_pop(r, all, &sz).error != ZENIT_OK) { FAIL("pop 192"); return 1; }
    /* First 64 are original bytes 64..127 */
    if (memcmp(in + 64, all, 64) != 0) { FAIL("second chunk first half"); return 1; }
    /* Next 128 are the second push */
    if (memcmp(in, all + 64, 128) != 0) { FAIL("second chunk second half"); return 1; }

    if (zenit_ring_count(r) != 0) { FAIL("should be empty"); return 1; }

    zenit_ring_destroy(r);
    return 0;
}

int main(void) {
    int ret = 0;
    ret |= test_create_destroy();
    ret |= test_create_zero();
    ret |= test_push_pop_one();
    ret |= test_push_full();
    ret |= test_pop_empty();
    ret |= test_peek();
    ret |= test_wrap_around();
    ret |= test_clear();
    ret |= test_edge_cases();
    ret |= test_chunks();

    if (failures > 0 || ret != 0) {
        fprintf(stderr, "FAIL: %d test(s) had errors\n", failures);
        return 1;
    }

    printf("PASS: ring buffer tests\n");
    return 0;
}
