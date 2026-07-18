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

#include <libzenit/benchmark.h>
#include <libzenit/trie.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------------------*/

#define NUM_KEYS  100000
#define KEY_LEN   6
#define ITERATIONS 100000

/* ---------------------------------------------------------------------------
 * Context holding a trie and a pre-generated key array.
 *
 * The index field is advanced sequentially by each benchmark function call
 * so that every invocation operates on a different key.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_trie_t* trie;
    char** keys;
    int count;
    int index;
} bench_ctx_t;

/* ---------------------------------------------------------------------------
 * Generate @p count unique keys of exactly @p len lowercase letters.
 *
 * Each key is a base-26 encoded representation of its index, zero-padded
 * to @p len characters.  This yields 26^len unique strings.
 * -------------------------------------------------------------------------*/

static char** generate_keys(int count, int len) {
    char** keys = malloc(sizeof(char*) * (size_t)count);
    if (keys == NULL) {
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        keys[i] = malloc(sizeof(char) * (size_t)(len + 1));
        if (keys[i] == NULL) {
            /* Free everything that was allocated so far */
            for (int j = 0; j < i; j++) {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        /* Encode index i as a base-26 string of length len */
        int n = i;
        for (int j = len - 1; j >= 0; j--) {
            keys[i][j] = (char)('a' + (n % 26));
            n /= 26;
        }
        keys[i][len] = '\0';
    }
    return keys;
}

/* ---------------------------------------------------------------------------
 * Free a key array previously returned by generate_keys().
 * -------------------------------------------------------------------------*/

static void free_keys(char** keys, int count) {
    if (keys == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(keys[i]);
    }
    free(keys);
}

/* ---------------------------------------------------------------------------
 * Benchmark function: insert one key.
 * -------------------------------------------------------------------------*/

static void bench_insert_fn(void* ctx) {
    bench_ctx_t* c = (bench_ctx_t*)ctx;
    int i = c->index++ % c->count;
    zenit_trie_insert(c->trie, c->keys[i], i);
}

/* ---------------------------------------------------------------------------
 * Benchmark function: search an existing key (hit).
 * -------------------------------------------------------------------------*/

static void bench_search_hit_fn(void* ctx) {
    bench_ctx_t* c = (bench_ctx_t*)ctx;
    int i = c->index++ % c->count;
    zenit_trie_search(c->trie, c->keys[i], NULL);
}

/* ---------------------------------------------------------------------------
 * Benchmark function: search a non-existent key (miss).
 *
 * We construct a 7-character key on the fly — all stored keys are exactly
 * 6 characters, so these never collide.
 * -------------------------------------------------------------------------*/

static void bench_search_miss_fn(void* ctx) {
    bench_ctx_t* c = (bench_ctx_t*)ctx;
    int i = c->index++;
    /* Use the same base-26 encoding but add an extra character */
    char miss[8];
    int n = i;
    for (int j = 6; j >= 0; j--) {
        miss[j] = (char)('a' + (n % 26));
        n /= 26;
    }
    miss[7] = '\0';
    zenit_trie_search(c->trie, miss, NULL);
}

/* ---------------------------------------------------------------------------
 * Benchmark function: check starts_with for the first 3 characters of a key.
 *
 * Since every key in the trie has at least KEY_LEN characters, the 3-char
 * prefix always exists for any stored key.
 * -------------------------------------------------------------------------*/

static void bench_starts_with_fn(void* ctx) {
    bench_ctx_t* c = (bench_ctx_t*)ctx;
    int i = c->index % c->count;
    c->index = i + 1;
    /* First 3 characters of key[i] — always exists */
    char prefix[4];
    prefix[0] = c->keys[i][0];
    prefix[1] = c->keys[i][1];
    prefix[2] = c->keys[i][2];
    prefix[3] = '\0';
    zenit_trie_starts_with(c->trie, prefix);
}

/* ---------------------------------------------------------------------------
 * Main — run all trie benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Generate the shared key set */
    char** keys = generate_keys(NUM_KEYS, KEY_LEN);
    if (keys == NULL) {
        return 1;
    }

    /* Create a fresh trie and pre-load it with all keys */
    zenit_trie_t* trie = zenit_trie_create();
    if (trie == NULL) {
        free_keys(keys, NUM_KEYS);
        return 1;
    }
    for (int i = 0; i < NUM_KEYS; i++) {
        zenit_trie_insert(trie, keys[i], i);
    }

    bench_ctx_t ctx;
    ctx.trie = trie;
    ctx.keys = keys;
    ctx.count = NUM_KEYS;

    /* ── insert benchmark ──
     * Use a separate trie so every call actually creates nodes.
     * We cannot reuse the pre-loaded trie because subsequent calls
     * would only overwrite existing values (no node creation). */
    {
        zenit_trie_t* insert_trie = zenit_trie_create();
        if (insert_trie != NULL) {
            bench_ctx_t ictx;
            ictx.trie = insert_trie;
            ictx.keys = keys;
            ictx.count = NUM_KEYS;
            ictx.index = 0;

            zenit_bench_result_t r = zenit_bench_run(
                "trie_insert", bench_insert_fn, &ictx, ITERATIONS
            );
            zenit_bench_print(&r);
            zenit_trie_destroy(insert_trie);
        }
    }

    /* ── search hit benchmark ── */
    {
        ctx.index = 0;
        zenit_bench_result_t r = zenit_bench_run(
            "trie_search_hit", bench_search_hit_fn, &ctx, ITERATIONS
        );
        zenit_bench_print(&r);
    }

    /* ── search miss benchmark ── */
    {
        ctx.index = 0;
        zenit_bench_result_t r = zenit_bench_run(
            "trie_search_miss", bench_search_miss_fn, &ctx, ITERATIONS
        );
        zenit_bench_print(&r);
    }

    /* ── starts_with benchmark ── */
    {
        ctx.index = 0;
        zenit_bench_result_t r = zenit_bench_run(
            "trie_starts_with", bench_starts_with_fn, &ctx, ITERATIONS
        );
        zenit_bench_print(&r);
    }

    zenit_trie_destroy(trie);
    free_keys(keys, NUM_KEYS);

    return 0;
}
