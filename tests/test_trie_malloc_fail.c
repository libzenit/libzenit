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

#include <libzenit/trie.h>
#include <stdio.h>
#include "test_malloc_fail.h"

int main(void) {
    /* ── trie_create fails on first alloc (the handle itself) ── */
    {
        malloc_fail_countdown = 0;
        zenit_trie_t* trie = zenit_trie_create();
        malloc_fail_countdown = -1;

        if (trie != NULL) {
            fprintf(stderr, "FAIL: expected NULL when malloc fails on create\n");
            zenit_trie_destroy(trie);
            return 1;
        }
    }

    /* ── trie_create_with_allocator fails on first alloc ── */
    {
        malloc_fail_countdown = 0;
        zenit_trie_t* trie = zenit_trie_create_with_allocator(ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;

        if (trie != NULL) {
            fprintf(stderr, "FAIL: expected NULL when malloc fails on create_with_allocator\n");
            zenit_trie_destroy(trie);
            return 1;
        }
    }

    /* ── trie_create succeeds first alloc but root node alloc fails ── */
    {
        /* First malloc succeeds (handle), second fails (root node) */
        malloc_fail_countdown = 1;
        zenit_trie_t* trie = zenit_trie_create();
        malloc_fail_countdown = -1;

        if (trie != NULL) {
            fprintf(stderr, "FAIL: expected NULL when root node alloc fails\n");
            zenit_trie_destroy(trie);
            return 1;
        }
    }

    /* ── insert fails when creating a child node ── */
    {
        /* Create the trie successfully */
        malloc_fail_countdown = -1;
        zenit_trie_t* trie = zenit_trie_create();
        if (trie == NULL) {
            fprintf(stderr, "FAIL: create should succeed\n");
            return 1;
        }

        /* Insert "ab" — first char 'a' creates child of root.
         * Two mallocs: one for 'a' child, one for 'b' child.
         * Fail the second one. */
        malloc_fail_countdown = 1;
        zenit_result_t r = zenit_trie_insert(trie, "ab", 1);
        malloc_fail_countdown = -1;

        if (r.error != ZENIT_ERROR_ALLOC) {
            fprintf(stderr, "FAIL: expected ALLOC error when child node alloc fails, got %d\n", (int)r.error);
            zenit_trie_destroy(trie);
            return 1;
        }

        /* The trie should still be usable — 'a' node was created, 'b' node
         * was not.  The key "ab" must not be found. */
        if (zenit_trie_search(trie, "ab", NULL) != 0) {
            fprintf(stderr, "FAIL: key should not exist after failed insert\n");
            zenit_trie_destroy(trie);
            return 1;
        }

        zenit_trie_destroy(trie);
    }

    printf("PASS: trie malloc fail\n");
    return 0;
}
