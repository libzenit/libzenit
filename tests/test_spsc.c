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

#include <libzenit/spsc.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Test 1: Create/destroy */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 16);
        if (s == NULL) {
            fprintf(stderr, "FAIL: spsc create\n");
            return 1;
        }
        if (zenit_spsc_capacity(s) != 16) {
            fprintf(stderr, "FAIL: spsc capacity %zu != 16\n", zenit_spsc_capacity(s));
            zenit_spsc_destroy(s);
            return 1;
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc create/destroy\n");

    /* Test 2: Push/pop single element */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
        int val = 42;
        zenit_result_t r = zenit_spsc_push(s, &val);
        if (r.error != ZENIT_OK) {
            fprintf(stderr, "FAIL: spsc push\n");
            zenit_spsc_destroy(s);
            return 1;
        }
        int out = 0;
        r = zenit_spsc_pop(s, &out);
        if (r.error != ZENIT_OK || out != 42) {
            fprintf(stderr, "FAIL: spsc pop got %d\n", out);
            zenit_spsc_destroy(s);
            return 1;
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc push/pop\n");

    /* Test 3: Full buffer */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 2);
        int v = 1; zenit_spsc_push(s, &v);
        v = 2; zenit_spsc_push(s, &v);
        v = 3;
        zenit_result_t r = zenit_spsc_push(s, &v);
        if (r.error != ZENIT_ERROR_FULL) {
            fprintf(stderr, "FAIL: spsc full not detected\n");
            zenit_spsc_destroy(s);
            return 1;
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc full\n");

    /* Test 4: Empty pop */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
        int out;
        zenit_result_t r = zenit_spsc_pop(s, &out);
        if (r.error != ZENIT_ERROR_EMPTY) {
            fprintf(stderr, "FAIL: spsc empty not detected\n");
            zenit_spsc_destroy(s);
            return 1;
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc empty\n");

    /* Test 5: Count and full/empty */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
        if (!zenit_spsc_empty(s)) {
            fprintf(stderr, "FAIL: spsc empty\n");
            zenit_spsc_destroy(s);
            return 1;
        }
        if (zenit_spsc_full(s)) {
            fprintf(stderr, "FAIL: spsc full\n");
            zenit_spsc_destroy(s);
            return 1;
        }
        int v = 1; zenit_spsc_push(s, &v);
        if (zenit_spsc_empty(s)) {
            fprintf(stderr, "FAIL: spsc not empty\n");
            zenit_spsc_destroy(s);
            return 1;
        }
        if (zenit_spsc_count(s) != 1) {
            fprintf(stderr, "FAIL: spsc count %zu != 1\n", zenit_spsc_count(s));
            zenit_spsc_destroy(s);
            return 1;
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc count/empty/full\n");

    /* Test 6: Wrap-around */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
        int v;
        for (int i = 0; i < 10; i++) {
            v = i; zenit_spsc_push(s, &v);
            zenit_spsc_pop(s, &v);
        }
        /* After 10 push/pop cycles, the internal indices should have wrapped */
        v = 99; zenit_spsc_push(s, &v);
        int out;
        zenit_spsc_pop(s, &out);
        if (out != 99) {
            fprintf(stderr, "FAIL: spsc wrap got %d\n", out);
            zenit_spsc_destroy(s);
            return 1;
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc wrap-around\n");

    /* Test 7: Multiple elements */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 8);
        for (int i = 0; i < 7; i++) {
            zenit_spsc_push(s, &i);
        }
        if (zenit_spsc_count(s) != 7) {
            fprintf(stderr, "FAIL: spsc multi count %zu\n", zenit_spsc_count(s));
            zenit_spsc_destroy(s);
            return 1;
        }
        for (int i = 0; i < 7; i++) {
            int out;
            zenit_spsc_pop(s, &out);
            if (out != i) {
                fprintf(stderr, "FAIL: spsc multi pop %d != %d\n", out, i);
                zenit_spsc_destroy(s);
                return 1;
            }
        }
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc multiple elements\n");

    /* Test 8: NULL params */
    {
        zenit_spsc_t *s = zenit_spsc_create(sizeof(int), 4);
        zenit_result_t r = zenit_spsc_push(NULL, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: spsc push NULL\n");
            zenit_spsc_destroy(s); return 1;
        }
        r = zenit_spsc_push(s, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: spsc push elem NULL\n");
            zenit_spsc_destroy(s); return 1;
        }
        r = zenit_spsc_pop(NULL, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: spsc pop NULL\n");
            zenit_spsc_destroy(s); return 1;
        }
        if (zenit_spsc_count(NULL) != 0) {
            fprintf(stderr, "FAIL: spsc count NULL\n");
            zenit_spsc_destroy(s); return 1;
        }
        if (zenit_spsc_capacity(NULL) != 0) {
            fprintf(stderr, "FAIL: spsc capacity NULL\n");
            zenit_spsc_destroy(s); return 1;
        }
        zenit_spsc_destroy(NULL);
        zenit_spsc_destroy(s);
    }
    printf("PASS: spsc NULL params\n");

    /* Test 9: Invalid creation params */
    {
        zenit_spsc_t *s = zenit_spsc_create(0, 16);
        if (s != NULL) {
            fprintf(stderr, "FAIL: spsc create zero elem_size\n");
            zenit_spsc_destroy(s); return 1;
        }
        s = zenit_spsc_create(sizeof(int), 0);
        if (s != NULL) {
            fprintf(stderr, "FAIL: spsc create zero capacity\n");
            zenit_spsc_destroy(s); return 1;
        }
    }
    printf("PASS: spsc invalid params\n");

    printf("PASS: spsc\n");
    return 0;
}
