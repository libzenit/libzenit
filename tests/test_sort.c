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

#include <libzenit/sort.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); return 1; } while (0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); } } while (0)

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

/* ─── quicksort ─── */

static int test_sort_already_sorted(void) {
    int arr[] = {1, 2, 3, 4, 5};
    zenit_sort_quick(arr, 5, sizeof(int), cmp_int);
    for (int i = 0; i < 5; i++) {
        ASSERT(arr[i] == i + 1, "already sorted array modified");
    }
    return 0;
}

static int test_sort_reverse(void) {
    int arr[] = {5, 4, 3, 2, 1};
    zenit_sort_quick(arr, 5, sizeof(int), cmp_int);
    for (int i = 0; i < 5; i++) {
        ASSERT(arr[i] == i + 1, "reverse sort failed");
    }
    return 0;
}

static int test_sort_random(void) {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    size_t n = sizeof(arr) / sizeof(arr[0]);
    zenit_sort_quick(arr, n, sizeof(int), cmp_int);
    for (size_t i = 1; i < n; i++) {
        ASSERT(arr[i - 1] <= arr[i], "random sort not ascending");
    }
    return 0;
}

static int test_sort_single(void) {
    int arr[] = {42};
    zenit_sort_quick(arr, 1, sizeof(int), cmp_int);
    ASSERT(arr[0] == 42, "single element changed");
    return 0;
}

static int test_sort_empty(void) {
    zenit_sort_quick(NULL, 0, sizeof(int), cmp_int);
    /* Should not crash */
    int arr[] = {1};
    zenit_sort_quick(arr, 0, sizeof(int), cmp_int);
    ASSERT(arr[0] == 1, "empty sort modified array");
    zenit_sort_quick(arr, 1, sizeof(int), NULL);
    ASSERT(arr[0] == 1, "NULL compare should be no-op");
    return 0;
}

static int test_sort_doubles(void) {
    double arr[] = {3.14, 1.41, 2.71, 0.0, -1.0};
    size_t n = sizeof(arr) / sizeof(arr[0]);
    zenit_sort_quick(arr, n, sizeof(double), cmp_double);
    for (size_t i = 1; i < n; i++) {
        ASSERT(arr[i - 1] <= arr[i], "double sort not ascending");
    }
    return 0;
}

static int test_sort_with_duplicates(void) {
    int arr[] = {3, 1, 3, 1, 3, 1};
    size_t n = sizeof(arr) / sizeof(arr[0]);
    zenit_sort_quick(arr, n, sizeof(int), cmp_int);
    for (size_t i = 1; i < n; i++) {
        ASSERT(arr[i - 1] <= arr[i], "duplicates sort not ascending");
    }
    return 0;
}

/* ─── binary_search ─── */

static int test_binary_search_found(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 5;
    int *found = (int *)zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "binary_search found returned NULL");
    ASSERT(*found == 5, "binary_search found wrong value");
    return 0;
}

static int test_binary_search_not_found(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 4;
    void *found = zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found == NULL, "binary_search not found should return NULL");
    return 0;
}

static int test_binary_search_first(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 1;
    int *found = (int *)zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "binary_search first returned NULL");
    ASSERT(*found == 1, "binary_search first wrong value");
    return 0;
}

static int test_binary_search_last(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 9;
    int *found = (int *)zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "binary_search last returned NULL");
    ASSERT(*found == 9, "binary_search last wrong value");
    return 0;
}

static int test_binary_search_empty(void) {
    int key = 5;
    void *found = zenit_binary_search(&key, NULL, 0, sizeof(int), cmp_int);
    ASSERT(found == NULL, "binary_search empty should return NULL");
    found = zenit_binary_search(&key, (int[]){1}, 0, sizeof(int), cmp_int);
    ASSERT(found == NULL, "binary_search zero count should return NULL");
    return 0;
}

static int test_binary_search_null_params(void) {
    int arr[] = {1, 2, 3};
    int key = 2;
    ASSERT(zenit_binary_search(NULL, arr, 3, sizeof(int), cmp_int) == NULL, "bs NULL key");
    ASSERT(zenit_binary_search(&key, arr, 3, sizeof(int), NULL) == NULL, "bs NULL compare");
    return 0;
}

/* Large element type to exercise the malloc-based swap path */
typedef struct {
    char data[128];
    int key;
} large_elem_t;

static int cmp_large(const void *a, const void *b) {
    int ka = ((const large_elem_t *)a)->key;
    int kb = ((const large_elem_t *)b)->key;
    if (ka < kb) return -1;
    if (ka > kb) return 1;
    return 0;
}

static int test_sort_large_elements(void) {
    large_elem_t arr[4];
    arr[0].key = 3;
    arr[1].key = 1;
    arr[2].key = 4;
    arr[3].key = 2;
    zenit_sort_quick(arr, 4, sizeof(large_elem_t), cmp_large);
    for (int i = 0; i < 4; i++) {
        ASSERT(arr[i].key == i + 1, "large element sort failed");
    }
    return 0;
}

int main(void) {
    int failed = 0;

    printf("=== sort ===\n");

    printf("  sort_already_sorted ... "); if (test_sort_already_sorted()) failed++; else printf("PASS\n");
    printf("  sort_reverse ... "); if (test_sort_reverse()) failed++; else printf("PASS\n");
    printf("  sort_random ... "); if (test_sort_random()) failed++; else printf("PASS\n");
    printf("  sort_single ... "); if (test_sort_single()) failed++; else printf("PASS\n");
    printf("  sort_empty ... "); if (test_sort_empty()) failed++; else printf("PASS\n");
    printf("  sort_doubles ... "); if (test_sort_doubles()) failed++; else printf("PASS\n");
    printf("  sort_with_duplicates ... "); if (test_sort_with_duplicates()) failed++; else printf("PASS\n");

    printf("  binary_search_found ... "); if (test_binary_search_found()) failed++; else printf("PASS\n");
    printf("  binary_search_not_found ... "); if (test_binary_search_not_found()) failed++; else printf("PASS\n");
    printf("  binary_search_first ... "); if (test_binary_search_first()) failed++; else printf("PASS\n");
    printf("  binary_search_last ... "); if (test_binary_search_last()) failed++; else printf("PASS\n");
    printf("  binary_search_empty ... "); if (test_binary_search_empty()) failed++; else printf("PASS\n");
    printf("  binary_search_null_params ... "); if (test_binary_search_null_params()) failed++;
    else printf("PASS\n");

    printf("  sort_large_elements ... "); if (test_sort_large_elements()) failed++;
    else printf("PASS\n");

    printf("\n%d tests failed\n", failed);
    return failed != 0 ? 1 : 0;
}
