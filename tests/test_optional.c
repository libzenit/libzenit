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

#include <libzenit/optional.h>
#include <stdio.h>

static int double_it(int x) { return x * 2; }

int main(void) {
    /* Test 1: Create none and check has */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_NONE;
        if (zenit_optional_has(&opt)) {
            fprintf(stderr, "FAIL: optional none should not have value\n");
            return 1;
        }
    }
    printf("PASS: optional none\n");

    /* Test 2: Create some and get value */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_SOME(42);
        if (!zenit_optional_has(&opt)) {
            fprintf(stderr, "FAIL: optional some should have value\n");
            return 1;
        }
        if (zenit_optional_get(&opt) != 42) {
            fprintf(stderr, "FAIL: optional get expected 42\n");
            return 1;
        }
    }
    printf("PASS: optional some\n");

    /* Test 3: Set value on none */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_NONE;
        zenit_optional_set(&opt, 99);
        if (!zenit_optional_has(&opt)) {
            fprintf(stderr, "FAIL: optional set should set has\n");
            return 1;
        }
        if (zenit_optional_get(&opt) != 99) {
            fprintf(stderr, "FAIL: optional get after set\n");
            return 1;
        }
    }
    printf("PASS: optional set\n");

    /* Test 4: Clear */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_SOME(42);
        zenit_optional_clear(&opt);
        if (zenit_optional_has(&opt)) {
            fprintf(stderr, "FAIL: optional clear should clear has\n");
            return 1;
        }
    }
    printf("PASS: optional clear\n");

    /* Test 5: get_or */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_NONE;
        int v = zenit_optional_get_or(&opt, -1);
        if (v != -1) {
            fprintf(stderr, "FAIL: optional get_or default\n");
            return 1;
        }
        zenit_optional_set(&opt, 42);
        v = zenit_optional_get_or(&opt, -1);
        if (v != 42) {
            fprintf(stderr, "FAIL: optional get_or value\n");
            return 1;
        }
    }
    printf("PASS: optional get_or\n");

    /* Test 6: Copy */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_SOME(42);
        int v;
        zenit_optional_copy(&opt, &v);
        if (v != 42) {
            fprintf(stderr, "FAIL: optional copy\n");
            return 1;
        }
    }
    printf("PASS: optional copy\n");

    /* Test 7: Map */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_SOME(5);
        ZENIT_OPTIONAL(int) mapped = ZENIT_OPTIONAL_NONE;
        zenit_optional_map(&opt, double_it, &mapped);
        if (!zenit_optional_has(&mapped)) {
            fprintf(stderr, "FAIL: optional map has\n");
            return 1;
        }
        if (zenit_optional_get(&mapped) != 10) {
            fprintf(stderr, "FAIL: optional map value\n");
            return 1;
        }
    }
    printf("PASS: optional map\n");

    /* Test 8: Map on none */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_NONE;
        ZENIT_OPTIONAL(int) mapped = ZENIT_OPTIONAL_SOME(99);
        zenit_optional_map(&opt, double_it, &mapped);
        if (zenit_optional_has(&mapped)) {
            fprintf(stderr, "FAIL: optional map none should be none\n");
            return 1;
        }
    }
    printf("PASS: optional map none\n");

    /* Test 9: Double type */
    {
        ZENIT_OPTIONAL(double) opt = ZENIT_OPTIONAL_SOME(3.14);
        if (!zenit_optional_has(&opt)) {
            fprintf(stderr, "FAIL: optional double has\n");
            return 1;
        }
        if (zenit_optional_get(&opt) != 3.14) {
            fprintf(stderr, "FAIL: optional double value\n");
            return 1;
        }
    }
    printf("PASS: optional double\n");

    /* Test 10: Pointer type */
    {
        int x = 42;
        ZENIT_OPTIONAL(int*) opt = ZENIT_OPTIONAL_SOME(&x);
        if (!zenit_optional_has(&opt)) {
            fprintf(stderr, "FAIL: optional ptr has\n");
            return 1;
        }
        if (zenit_optional_get(&opt) != &x) {
            fprintf(stderr, "FAIL: optional ptr value\n");
            return 1;
        }
    }
    printf("PASS: optional pointer\n");

    /* Test 11: Overwrite */
    {
        ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_SOME(1);
        zenit_optional_set(&opt, 2);
        if (zenit_optional_get(&opt) != 2) {
            fprintf(stderr, "FAIL: optional overwrite\n");
            return 1;
        }
    }
    printf("PASS: optional overwrite\n");

    printf("PASS: optional\n");
    return 0;
}
