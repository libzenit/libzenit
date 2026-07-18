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
#include <libzenit/result.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    zenit_trie_t *trie;
    int val;

    /* ── create / destroy ── */
    trie = zenit_trie_create();
    if (trie == NULL) {
        fprintf(stderr, "FAIL: create returned NULL\n");
        return 1;
    }
    zenit_trie_destroy(trie);

    /* ── count on empty trie ── */
    trie = zenit_trie_create();
    if (zenit_trie_count(trie) != 0) {
        fprintf(stderr, "FAIL: count should be 0 on empty trie\n");
        return 1;
    }

    /* ── search miss on empty ── */
    if (zenit_trie_search(trie, "hello", NULL) != 0) {
        fprintf(stderr, "FAIL: search should miss on empty trie\n");
        return 1;
    }

    /* ── insert + search hit ── */
    if (zenit_trie_insert(trie, "hello", 42).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert hello failed\n");
        return 1;
    }
    if (zenit_trie_count(trie) != 1) {
        fprintf(stderr, "FAIL: count should be 1 after insert\n");
        return 1;
    }
    if (!zenit_trie_search(trie, "hello", &val)) {
        fprintf(stderr, "FAIL: search hello should hit\n");
        return 1;
    }
    if (val != 42) {
        fprintf(stderr, "FAIL: value should be 42, got %d\n", val);
        return 1;
    }

    /* ── search miss for different word ── */
    if (zenit_trie_search(trie, "world", NULL) != 0) {
        fprintf(stderr, "FAIL: search world should miss\n");
        return 1;
    }

    /* ── starts_with true ── */
    if (!zenit_trie_starts_with(trie, "hel")) {
        fprintf(stderr, "FAIL: starts_with hel should be true\n");
        return 1;
    }

    /* ── starts_with false ── */
    if (zenit_trie_starts_with(trie, "xyz")) {
        fprintf(stderr, "FAIL: starts_with xyz should be false\n");
        return 1;
    }

    /* ── multiple words sharing prefix ── */
    if (zenit_trie_insert(trie, "hell", 1).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert hell failed\n");
        return 1;
    }
    if (zenit_trie_insert(trie, "helicopter", 2).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert helicopter failed\n");
        return 1;
    }
    if (zenit_trie_insert(trie, "help", 3).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert help failed\n");
        return 1;
    }
    if (zenit_trie_insert(trie, "HELLO", 99).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert uppercase HELLO failed\n");
        return 1;
    }
    if (zenit_trie_count(trie) != 4) {
        fprintf(stderr, "FAIL: count should be 4 (HELLO overwrites hello), got %zu\n", zenit_trie_count(trie));
        return 1;
    }
    /* HELLO should have overwritten hello's value (case folding) */
    if (!zenit_trie_search(trie, "hello", &val)) {
        fprintf(stderr, "FAIL: search hello should hit after HELLO insert\n");
        return 1;
    }
    if (val != 99) {
        fprintf(stderr, "FAIL: hello value should be 99 (overwritten by HELLO), got %d\n", val);
        return 1;
    }

    /* ── starts_with with exact key ── */
    if (!zenit_trie_starts_with(trie, "help")) {
        fprintf(stderr, "FAIL: starts_with help should be true\n");
        return 1;
    }
    if (!zenit_trie_starts_with(trie, "h")) {
        fprintf(stderr, "FAIL: starts_with h should be true\n");
        return 1;
    }

    /* ── delete a word ── */
    if (zenit_trie_delete(trie, "hell").error != ZENIT_OK) {
        fprintf(stderr, "FAIL: delete hell failed\n");
        return 1;
    }
    if (zenit_trie_count(trie) != 3) {
        fprintf(stderr, "FAIL: count should be 3 after delete, got %zu\n", zenit_trie_count(trie));
        return 1;
    }
    if (zenit_trie_search(trie, "hell", NULL) != 0) {
        fprintf(stderr, "FAIL: hell should be gone after delete\n");
        return 1;
    }
    /* Sibling keys should still exist */
    if (!zenit_trie_search(trie, "hello", NULL)) {
        fprintf(stderr, "FAIL: hello should still exist after sibling delete\n");
        return 1;
    }
    if (!zenit_trie_search(trie, "help", NULL)) {
        fprintf(stderr, "FAIL: help should still exist after sibling delete\n");
        return 1;
    }

    /* ── delete non-existent word ── */
    if (zenit_trie_delete(trie, "nonexistent").error != ZENIT_ERROR_NOT_FOUND) {
        fprintf(stderr, "FAIL: delete non-existent should return NOT_FOUND\n");
        return 1;
    }
    if (zenit_trie_delete(trie, "hell").error != ZENIT_ERROR_NOT_FOUND) {
        fprintf(stderr, "FAIL: delete already-deleted hell should return NOT_FOUND\n");
        return 1;
    }

    /* ── clear ── */
    zenit_trie_clear(trie);
    if (zenit_trie_count(trie) != 0) {
        fprintf(stderr, "FAIL: count should be 0 after clear, got %zu\n", zenit_trie_count(trie));
        return 1;
    }
    if (zenit_trie_search(trie, "hello", NULL) != 0) {
        fprintf(stderr, "FAIL: hello should be gone after clear\n");
        return 1;
    }
    if (zenit_trie_starts_with(trie, "h") != 0) {
        fprintf(stderr, "FAIL: starts_with h should be false after clear\n");
        return 1;
    }
    /* Trie should be reusable after clear */
    if (zenit_trie_insert(trie, "reuse", 100).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert after clear failed\n");
        return 1;
    }
    if (zenit_trie_count(trie) != 1) {
        fprintf(stderr, "FAIL: count should be 1 after reinsert, got %zu\n", zenit_trie_count(trie));
        return 1;
    }
    if (!zenit_trie_search(trie, "reuse", NULL)) {
        fprintf(stderr, "FAIL: reuse should exist after reinsert\n");
        return 1;
    }

    zenit_trie_destroy(trie);

    /* ── allocator variant ── */
    trie = zenit_trie_create_with_allocator(ZENIT_ALLOCATOR_DEFAULT);
    if (trie == NULL) {
        fprintf(stderr, "FAIL: create_with_allocator returned NULL\n");
        return 1;
    }
    if (zenit_trie_insert(trie, "allocator_test", 7).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert with allocator trie failed\n");
        return 1;
    }
    if (!zenit_trie_search(trie, "allocator_test", &val)) {
        fprintf(stderr, "FAIL: search with allocator trie should hit\n");
        return 1;
    }
    if (val != 7) {
        fprintf(stderr, "FAIL: value should be 7, got %d\n", val);
        return 1;
    }
    zenit_trie_destroy(trie);

    /* ── NULL param safety ── */
    trie = zenit_trie_create();
    zenit_trie_insert(trie, "sample", 10);

    if (zenit_trie_search(NULL, "sample", NULL) != 0) {
        fprintf(stderr, "FAIL: search NULL trie should return 0\n");
        return 1;
    }
    if (zenit_trie_search(trie, NULL, NULL) != 0) {
        fprintf(stderr, "FAIL: search NULL key should return 0\n");
        return 1;
    }
    if (zenit_trie_starts_with(NULL, "s") != 0) {
        fprintf(stderr, "FAIL: starts_with NULL trie should return 0\n");
        return 1;
    }
    if (zenit_trie_starts_with(trie, NULL) != 0) {
        fprintf(stderr, "FAIL: starts_with NULL prefix should return 0\n");
        return 1;
    }
    if (zenit_trie_insert(NULL, "key", 0).error != ZENIT_ERROR_NULL) {
        fprintf(stderr, "FAIL: insert NULL trie should return NULL error\n");
        return 1;
    }
    if (zenit_trie_insert(trie, NULL, 0).error != ZENIT_ERROR_NULL) {
        fprintf(stderr, "FAIL: insert NULL key should return NULL error\n");
        return 1;
    }
    if (zenit_trie_delete(NULL, "key").error != ZENIT_ERROR_NULL) {
        fprintf(stderr, "FAIL: delete NULL trie should return NULL error\n");
        return 1;
    }
    if (zenit_trie_delete(trie, NULL).error != ZENIT_ERROR_NULL) {
        fprintf(stderr, "FAIL: delete NULL key should return NULL error\n");
        return 1;
    }
    if (zenit_trie_count(NULL) != 0) {
        fprintf(stderr, "FAIL: count NULL trie should return 0\n");
        return 1;
    }

    zenit_trie_destroy(trie);

    /* ── NULL out_value ── */
    trie = zenit_trie_create();
    zenit_trie_insert(trie, "test", 123);
    if (!zenit_trie_search(trie, "test", NULL)) {
        fprintf(stderr, "FAIL: search with NULL out_value should still return found\n");
        return 1;
    }
    zenit_trie_destroy(trie);

    /* ── NULL destroy / clear safe ── */
    zenit_trie_destroy(NULL);
    zenit_trie_clear(NULL);

    /* ── non-alphabetic characters are skipped ── */
    trie = zenit_trie_create();
    /* "hello123world" should be treated as "helloworld" */
    if (zenit_trie_insert(trie, "hello123world", 5).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: insert with digits failed\n");
        return 1;
    }
    if (zenit_trie_count(trie) != 1) {
        fprintf(stderr, "FAIL: count should be 1 after insert with digits\n");
        return 1;
    }
    if (!zenit_trie_search(trie, "helloworld", &val)) {
        fprintf(stderr, "FAIL: search helloworld should hit (digits skipped)\n");
        return 1;
    }
    if (val != 5) {
        fprintf(stderr, "FAIL: value should be 5, got %d\n", val);
        return 1;
    }
    if (!zenit_trie_starts_with(trie, "hello_world")) {
        fprintf(stderr, "FAIL: starts_with with non-alpha chars should work\n");
        return 1;
    }
    if (zenit_trie_delete(trie, "hello_world").error != ZENIT_OK) {
        fprintf(stderr, "FAIL: delete with underscores should work\n");
        return 1;
    }
    if (zenit_trie_count(trie) != 0) {
        fprintf(stderr, "FAIL: count should be 0 after delete with underscores\n");
        return 1;
    }
    zenit_trie_destroy(trie);

    printf("PASS: trie\n");
    return 0;
}
