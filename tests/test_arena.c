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

#include <libzenit/arena.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 1 MB for test purposes */
#define MB (1024ul * 1024ul)

/* ─── Helper to fail fast ─── */
static int failures = 0;
#define FAIL(msg) do { \
    fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
    failures++; \
} while(0)

/* ─── Test: arena create / destroy ─── */
static int test_create_destroy(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create returned NULL"); return 1; }
    zenit_arena_destroy(a);
    zenit_arena_destroy(NULL); /* should be safe */
    return 0;
}

/* ─── Test: invalid create parameters ─── */
static int test_create_invalid(void) {
    /* total_size % block_size != 0 */
    if (zenit_arena_create(100 * MB, 3 * MB) != NULL) {
        FAIL("expected NULL for non-divisible sizes");
        return 1;
    }
    /* zero total_size */
    if (zenit_arena_create(0, 10 * MB) != NULL) {
        FAIL("expected NULL for zero total_size");
        return 1;
    }
    /* zero block_size */
    if (zenit_arena_create(100 * MB, 0) != NULL) {
        FAIL("expected NULL for zero block_size");
        return 1;
    }
    return 0;
}

/* ─── Test: acquire / release ─── */
static int test_acquire_release(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire returned NULL"); return 1; }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release should succeed");
        return 1;
    }

    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: acquire with invalid size (not multiple of block) ─── */
static int test_acquire_invalid_size(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    if (zenit_arena_acquire(a, 25 * MB) != NULL) {
        FAIL("expected NULL for non-divisible acquire size");
        return 1;
    }

    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: acquire with not enough contiguous blocks ─── */
static int test_acquire_not_enough(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    /* Acquire 90 MB (9 blocks), leaving 1 block free */
    zenit_usable_arena_t *ua1 = zenit_arena_acquire(a, 90 * MB);
    if (ua1 == NULL) { FAIL("first acquire"); return 1; }

    /* Try to acquire 20 MB — only 10 MB remain, not contiguous? Actually
     * the remaining 10 MB is contiguous (last block). But 20 MB needs 2
     * blocks. */
    zenit_usable_arena_t *ua2 = zenit_arena_acquire(a, 20 * MB);
    if (ua2 != NULL) {
        FAIL("expected NULL when insufficient space");
        zenit_arena_release(a, ua2);
        return 1;
    }

    zenit_arena_release(a, ua1);
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: acquire with fragmentation (non-contiguous) ─── */
static int test_acquire_fragmented(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    /* Acquire blocks 0, 1, 2, 3 (40 MB) */
    zenit_usable_arena_t *ua1 = zenit_arena_acquire(a, 40 * MB);
    if (ua1 == NULL) { FAIL("ua1 acquire"); return 1; }

    /* Release middle part (blocks 1,2) — this is 2*10=20MB, creates gap */
    /* But we can only release complete usable arenas, not partial blocks.
     * So we need a different approach: acquire and release in patterns. */
    zenit_arena_release(a, ua1);

    /* Acquire first 20 MB, leaving 80 MB free but contiguous */
    zenit_usable_arena_t *ua2 = zenit_arena_acquire(a, 20 * MB);
    if (ua2 == NULL) { FAIL("ua2 acquire"); return 1; }

    /* Release ua2 — now first 20 MB are free again */
    zenit_arena_release(a, ua2);

    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: buffer allocate / free basic ─── */
static int test_buffer_alloc_free(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    zenit_usable_buffer_t buf = zenit_usable_arena_allocate(ua, 1024);
    if (buf.data == NULL) { FAIL("allocate returned NULL"); return 1; }
    if (buf.size < 1024) { FAIL("size too small"); return 1; }

    /* Write and verify */
    memset(buf.data, 0xAB, 1024);
    unsigned char *p = (unsigned char *)buf.data;
    for (size_t i = 0; i < 1024; i++) {
        if (p[i] != 0xAB) { FAIL("data mismatch"); return 1; }
    }

    if (zenit_usable_buffer_data(&buf) != buf.data) {
        FAIL("data() mismatch");
        return 1;
    }
    if (zenit_usable_buffer_size(&buf) < 1024) {
        FAIL("size() mismatch");
        return 1;
    }

    if (zenit_usable_buffer_free(&buf) != 0) {
        FAIL("free returned error");
        return 1;
    }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release");
        return 1;
    }
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: double-free detection ─── */
static int test_double_free(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    zenit_usable_buffer_t buf = zenit_usable_arena_allocate(ua, 1024);
    if (buf.data == NULL) { FAIL("allocate"); return 1; }

    /* First free should succeed */
    if (zenit_usable_buffer_free(&buf) != 0) {
        FAIL("first free failed");
        return 1;
    }

    /* Second free of same buffer should fail (double-free detection) */
    if (zenit_usable_buffer_free(&buf) != -1) {
        FAIL("double-free not detected");
        return 1;
    }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release");
        return 1;
    }
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: NULL / zero-size buffer operations ─── */
static int test_buffer_edge_cases(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    /* Allocate with size 0 should return NULL */
    zenit_usable_buffer_t b0 = zenit_usable_arena_allocate(ua, 0);
    if (b0.data != NULL) {
        FAIL("expected NULL for size=0");
        return 1;
    }

    /* Free NULL buffer */
    if (zenit_usable_buffer_free(NULL) != -1) {
        FAIL("free(NULL) should return -1");
        return 1;
    }

    /* Free buffer with NULL data */
    zenit_usable_buffer_t bnull = { NULL, 0 };
    if (zenit_usable_buffer_free(&bnull) != -1) {
        FAIL("free with NULL data should return -1");
        return 1;
    }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release");
        return 1;
    }
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: allocate too large (bigger than usable arena) ─── */
static int test_allocate_too_large(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    /* Try to allocate more than the entire usable arena */
    zenit_usable_buffer_t buf = zenit_usable_arena_allocate(ua, 40 * MB);
    if (buf.data != NULL) {
        FAIL("expected NULL for oversized allocation");
        return 1;
    }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release");
        return 1;
    }
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: multiple buffers, fragmentation, coalescing ─── */
static int test_multiple_buffers(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    /* Allocate 3 buffers */
    zenit_usable_buffer_t b1 = zenit_usable_arena_allocate(ua, 4096);
    zenit_usable_buffer_t b2 = zenit_usable_arena_allocate(ua, 8192);
    zenit_usable_buffer_t b3 = zenit_usable_arena_allocate(ua, 16384);

    if (b1.data == NULL || b2.data == NULL || b3.data == NULL) {
        FAIL("multiple allocate");
        return 1;
    }

    /* Free middle buffer */
    if (zenit_usable_buffer_free(&b2) != 0) {
        FAIL("free b2");
        return 1;
    }

    /* Free first buffer */
    if (zenit_usable_buffer_free(&b1) != 0) {
        FAIL("free b1");
        return 1;
    }

    /* Free last buffer */
    if (zenit_usable_buffer_free(&b3) != 0) {
        FAIL("free b3");
        return 1;
    }

    /* Now all 30 MB should be one big free block (coalesced).
     * Allocate 25 MB to verify coalescing works. */
    zenit_usable_buffer_t big = zenit_usable_arena_allocate(ua, 25 * MB);
    if (big.data == NULL) {
        FAIL("coalescing failed — big alloc returned NULL");
        return 1;
    }

    if (zenit_usable_buffer_free(&big) != 0) {
        FAIL("free big");
        return 1;
    }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release");
        return 1;
    }
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: release usable arena with outstanding buffers ─── */
static int test_release_with_outstanding(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    zenit_usable_buffer_t buf = zenit_usable_arena_allocate(ua, 4096);
    if (buf.data == NULL) { FAIL("allocate"); return 1; }

    /* Release should fail because buffer is still IN_USE */
    if (zenit_arena_release(a, ua) != -1) {
        FAIL("expected -1 for release with outstanding buffers");
        return 1;
    }

    /* Free buffer, then release should work */
    if (zenit_usable_buffer_free(&buf) != 0) {
        FAIL("free");
        return 1;
    }

    if (zenit_arena_release(a, ua) != 0) {
        FAIL("release after free");
        return 1;
    }
    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: NULL / edge cases for arena functions ─── */
static int test_arena_edge_cases(void) {
    /* destroy(NULL) should be safe — already tested above */

    /* acquire(NULL, ...) */
    if (zenit_arena_acquire(NULL, 10 * MB) != NULL) {
        FAIL("acquire(NULL) should return NULL");
        return 1;
    }

    /* release(NULL, ...) */
    if (zenit_arena_release(NULL, (zenit_usable_arena_t *)1) != -1) {
        FAIL("release(NULL, ...) should return -1");
        return 1;
    }

    /* release(..., NULL) */
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }
    if (zenit_arena_release(a, NULL) != -1) {
        FAIL("release(..., NULL) should return -1");
        return 1;
    }

    /* allocate(NULL, size) */
    zenit_usable_buffer_t b = zenit_usable_arena_allocate(NULL, 1024);
    if (b.data != NULL) {
        FAIL("allocate(NULL) should return NULL data");
        return 1;
    }

    /* data(NULL) */
    if (zenit_usable_buffer_data(NULL) != NULL) {
        FAIL("data(NULL) should return NULL");
        return 1;
    }

    /* size(NULL) */
    if (zenit_usable_buffer_size(NULL) != 0) {
        FAIL("size(NULL) should return 0");
        return 1;
    }

    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: multiple usable arenas from same arena ─── */
static int test_multiple_usable_arenas(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua1 = zenit_arena_acquire(a, 30 * MB);
    zenit_usable_arena_t *ua2 = zenit_arena_acquire(a, 40 * MB);
    zenit_usable_arena_t *ua3 = zenit_arena_acquire(a, 20 * MB);

    if (ua1 == NULL || ua2 == NULL || ua3 == NULL) {
        FAIL("multiple acquire");
        return 1;
    }

    /* 30+40+20 = 90 MB used; try to acquire another 20 (needs 2 blocks,
     * only 10 MB free) */
    zenit_usable_arena_t *ua4 = zenit_arena_acquire(a, 20 * MB);
    if (ua4 != NULL) {
        FAIL("expected NULL — only 10 MB remaining");
        return 1;
    }

    /* Release middle arena, now we have blocks 0-2 free, 3-6 used, 7-9 free.
     * Actually: ua1=0-2, ua2=3-6, ua3=7-8 (2 blocks = 20MB).
     * After releasing ua2: 3-6 are free. Blocks 2 and 7 are still used.
     * Can we acquire 20 MB? We need 2 contiguous blocks. 0-2 are used by ua1,
     * 3-4 are free (from ua2), 5-6 free (from ua2), 7-8 used by ua3.
     * So we have blocks 3-6 (4 contiguous blocks = 40 MB). Acquiring 20 MB
     * should work. */
    if (zenit_arena_release(a, ua2) != 0) {
        FAIL("release ua2");
        return 1;
    }

    zenit_usable_arena_t *ua5 = zenit_arena_acquire(a, 20 * MB);
    if (ua5 == NULL) {
        FAIL("should be able to acquire 20 MB after release");
        return 1;
    }

    /* Now we have: ua1 (0-2), ua5 (3-4), ua3 (7-8). Blocks 5-6 are still
     * free from ua2's release (since ua5 only took 2 blocks). Actually,
     * blocks 3-6 were free. Acquire 20 MB (2 blocks) at 3-4 leaves 5-6 free.
     * Acquire another 20 MB? Only 5-6 free (2 blocks), can acquire. */
    zenit_usable_arena_t *ua6 = zenit_arena_acquire(a, 20 * MB);
    if (ua6 == NULL) {
        FAIL("should be able to acquire remaining 20 MB");
        return 1;
    }

    /* Now all blocks are used. ua1=0-2, ua5=3-4, ua6=5-6, ua3=7-8.
     * Only block 9 is free (10 MB). Acquiring 20 MB should fail. */
    zenit_usable_arena_t *ua7 = zenit_arena_acquire(a, 20 * MB);
    if (ua7 != NULL) {
        FAIL("expected NULL — no more contiguous space");
        return 1;
    }

    /* Cleanup */
    zenit_arena_release(a, ua1);
    zenit_arena_release(a, ua5);
    zenit_arena_release(a, ua6);
    zenit_arena_release(a, ua3);

    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: release with corrupted block (size == 0) ─── */
static int test_release_corrupted(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    /* Allocate and free a buffer so we can locate its header in memory */
    zenit_usable_buffer_t b = zenit_usable_arena_allocate(ua, 64);
    if (b.data == NULL) { FAIL("alloc"); return 1; }

    if (zenit_usable_buffer_free(&b) != 0) { FAIL("free"); return 1; }

    /* The header is immediately before the data pointer.
     * On x86-64 (GCC/Clang): sizeof(header)=40, size field at offset 16.
     * So the size field is at (char*)b.data - 40 + 16 = (char*)b.data - 24.
     * We write 0 there to simulate memory corruption. */
    size_t *size_field = (size_t *)((unsigned char *)b.data - 24);
    size_t saved_size = *size_field;
    *size_field = 0;

    /* Release must detect the corrupted block and refuse */
    int ret = zenit_arena_release(a, ua);
    *size_field = saved_size; /* restore for clean shutdown */
    if (ret != -1) {
        FAIL("release should detect corrupted block (size==0)");
        return 1;
    }

    zenit_arena_destroy(a);
    return 0;
}

/* ─── Test: buffer split and exact-fit ─── */
static int test_buffer_split(void) {
    zenit_arena_t *a = zenit_arena_create(100 * MB, 10 * MB);
    if (a == NULL) { FAIL("create"); return 1; }

    zenit_usable_arena_t *ua = zenit_arena_acquire(a, 30 * MB);
    if (ua == NULL) { FAIL("acquire"); return 1; }

    /* Allocate a small buffer — this should cause a split in the initial
     * 30 MB free block. */
    zenit_usable_buffer_t b1 = zenit_usable_arena_allocate(ua, 1024);
    if (b1.data == NULL) { FAIL("b1 alloc"); return 1; }

    /* Allocate another small buffer */
    zenit_usable_buffer_t b2 = zenit_usable_arena_allocate(ua, 2048);
    if (b2.data == NULL) { FAIL("b2 alloc"); return 1; }

    /* Allocate a third */
    zenit_usable_buffer_t b3 = zenit_usable_arena_allocate(ua, 4096);
    if (b3.data == NULL) { FAIL("b3 alloc"); return 1; }

    /* Free b2 (middle) — b1 and b3 remain, b2 goes to free list */
    if (zenit_usable_buffer_free(&b2) != 0) { FAIL("free b2"); return 1; }

    /* Free b1 — should coalesce with b2 if adjacent */
    if (zenit_usable_buffer_free(&b1) != 0) { FAIL("free b1"); return 1; }

    /* Free b3 */
    if (zenit_usable_buffer_free(&b3) != 0) { FAIL("free b3"); return 1; }

    /* Allocate a big chunk to verify coalescing */
    zenit_usable_buffer_t big = zenit_usable_arena_allocate(ua, 20 * MB);
    if (big.data == NULL) {
        FAIL("big alloc after coalesce failed");
        return 1;
    }

    if (zenit_usable_buffer_free(&big) != 0) { FAIL("free big"); return 1; }

    if (zenit_arena_release(a, ua) != 0) { FAIL("release"); return 1; }
    zenit_arena_destroy(a);
    return 0;
}

int main(void) {
    int ret = 0;

    ret |= test_create_destroy();
    ret |= test_create_invalid();
    ret |= test_acquire_release();
    ret |= test_acquire_invalid_size();
    ret |= test_acquire_not_enough();
    ret |= test_acquire_fragmented();
    ret |= test_buffer_alloc_free();
    ret |= test_double_free();
    ret |= test_buffer_edge_cases();
    ret |= test_allocate_too_large();
    ret |= test_multiple_buffers();
    ret |= test_release_with_outstanding();
    ret |= test_arena_edge_cases();
    ret |= test_multiple_usable_arenas();
    ret |= test_release_corrupted();
    ret |= test_buffer_split();

    if (failures > 0 || ret != 0) {
        fprintf(stderr, "FAIL: %d test(s) had errors\n", failures);
        return 1;
    }

    printf("PASS: arena tests\n");
    return 0;
}
