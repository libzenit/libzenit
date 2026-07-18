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
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const int *found = (const int *)zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "binary_search found returned NULL");
    ASSERT(*found == 5, "binary_search found wrong value");
    return 0;
}

static int test_binary_search_not_found(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 4;
    const void *found = zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found == NULL, "binary_search not found should return NULL");
    return 0;
}

static int test_binary_search_first(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 1;
    const int *found = (const int *)zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "binary_search first returned NULL");
    ASSERT(*found == 1, "binary_search first wrong value");
    return 0;
}

static int test_binary_search_last(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 9;
    const int *found = (const int *)zenit_binary_search(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "binary_search last returned NULL");
    ASSERT(*found == 9, "binary_search last wrong value");
    return 0;
}

static int test_binary_search_empty(void) {
    int key = 5;
    const void *found = zenit_binary_search(&key, NULL, 0, sizeof(int), cmp_int);
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

/* ─── lower_bound ─── */

static int test_lower_bound_basic(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 4;
    const int *found = (const int *)zenit_lower_bound(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "lower_bound returned NULL");
    ASSERT(*found == 5, "lower_bound should be first >= key");
    return 0;
}

static int test_lower_bound_exact(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 5;
    const int *found = (const int *)zenit_lower_bound(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "lower_bound exact returned NULL");
    ASSERT(*found == 5, "lower_bound exact wrong value");
    return 0;
}

static int test_lower_bound_all_less(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 10;
    const void *found = zenit_lower_bound(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found == NULL, "lower_bound all less should be NULL");
    return 0;
}

static int test_lower_bound_duplicates(void) {
    int arr[] = {1, 3, 5, 5, 5, 7, 9};
    int key = 5;
    const int *found = (const int *)zenit_lower_bound(&key, arr, 7, sizeof(int), cmp_int);
    ASSERT(found != NULL, "lower_bound duplicates returned NULL");
    ASSERT(*found == 5, "lower_bound duplicates wrong value");
    ASSERT(found == &arr[2], "lower_bound should be first of dupes");
    return 0;
}

/* ─── upper_bound ─── */

static int test_upper_bound_basic(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 4;
    const int *found = (const int *)zenit_upper_bound(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "upper_bound returned NULL");
    ASSERT(*found == 5, "upper_bound should be first > key");
    return 0;
}

static int test_upper_bound_exact(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 5;
    const int *found = (const int *)zenit_upper_bound(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found != NULL, "upper_bound exact returned NULL");
    ASSERT(*found == 7, "upper_bound exact should be after last 5");
    return 0;
}

static int test_upper_bound_all_less_equal(void) {
    int arr[] = {1, 3, 5, 7, 9};
    int key = 9;
    const void *found = zenit_upper_bound(&key, arr, 5, sizeof(int), cmp_int);
    ASSERT(found == NULL, "upper_bound all <= key should be NULL");
    return 0;
}

static int test_upper_bound_duplicates(void) {
    int arr[] = {1, 3, 5, 5, 5, 7, 9};
    int key = 5;
    const int *found = (const int *)zenit_upper_bound(&key, arr, 7, sizeof(int), cmp_int);
    ASSERT(found != NULL, "upper_bound duplicates returned NULL");
    ASSERT(*found == 7, "upper_bound duplicates should be after last 5");
    ASSERT(found == &arr[5], "upper_bound duplicates wrong index");
    return 0;
}

/* ─── stable_sort ─── */

typedef struct {
    int key;
    int order;
} stable_elem_t;

static int cmp_stable(const void *a, const void *b) {
    int ka = ((const stable_elem_t *)a)->key;
    int kb = ((const stable_elem_t *)b)->key;
    if (ka < kb) return -1;
    if (ka > kb) return 1;
    return 0;
}

static int test_stable_sort_basic(void) {
    int arr[] = {3, 1, 4, 1, 5, 9};
    size_t n = sizeof(arr) / sizeof(arr[0]);
    zenit_result_t r = zenit_sort_stable(arr, n, sizeof(int), cmp_int);
    ASSERT(r.error == ZENIT_OK, "stable_sort basic failed");
    for (size_t i = 1; i < n; i++) {
        ASSERT(arr[i - 1] <= arr[i], "stable sort not ascending");
    }
    return 0;
}

static int test_stable_sort_preserves_order(void) {
    stable_elem_t arr[] = {
        {3, 1}, {1, 1}, {3, 2}, {2, 1}, {1, 2}, {2, 2}
    };
    size_t n = sizeof(arr) / sizeof(arr[0]);
    zenit_result_t r = zenit_sort_stable(arr, n, sizeof(stable_elem_t), cmp_stable);
    ASSERT(r.error == ZENIT_OK, "stable_sort stability failed");

    /* Verify stable sort preserved insertion order for equal keys */
    ASSERT(arr[0].key == 1 && arr[0].order == 1, "stable elem 0");
    ASSERT(arr[1].key == 1 && arr[1].order == 2, "stable elem 1");
    ASSERT(arr[2].key == 2 && arr[2].order == 1, "stable elem 2");
    ASSERT(arr[3].key == 2 && arr[3].order == 2, "stable elem 3");
    ASSERT(arr[4].key == 3 && arr[4].order == 1, "stable elem 4");
    ASSERT(arr[5].key == 3 && arr[5].order == 2, "stable elem 5");
    return 0;
}

static int test_stable_sort_empty(void) {
    zenit_result_t r = zenit_sort_stable(NULL, 0, sizeof(int), cmp_int);
    ASSERT(r.error == ZENIT_OK, "stable_sort NULL base OK");
    int arr[] = {1};
    r = zenit_sort_stable(arr, 0, sizeof(int), cmp_int);
    ASSERT(r.error == ZENIT_OK, "stable_sort zero count OK");
    ASSERT(arr[0] == 1, "stable_sort empty no-op");
    r = zenit_sort_stable(arr, 1, sizeof(int), NULL);
    ASSERT(r.error == ZENIT_OK, "stable_sort NULL compare OK");
    return 0;
}

static int test_stable_sort_single(void) {
    int arr[] = {42};
    zenit_result_t r = zenit_sort_stable(arr, 1, sizeof(int), cmp_int);
    ASSERT(r.error == ZENIT_OK, "stable_sort single");
    ASSERT(arr[0] == 42, "stable_sort single unchanged");
    return 0;
}

static int test_stable_sort_reverse(void) {
    int arr[] = {5, 4, 3, 2, 1};
    size_t n = sizeof(arr) / sizeof(arr[0]);
    zenit_result_t r = zenit_sort_stable(arr, n, sizeof(int), cmp_int);
    ASSERT(r.error == ZENIT_OK, "stable_sort reverse");
    for (size_t i = 1; i < n; i++) {
        ASSERT(arr[i - 1] <= arr[i], "stable sort reverse ascending");
    }
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
    int (*tests[])(void) = {
        &test_sort_already_sorted,
        &test_sort_reverse,
        &test_sort_random,
        &test_sort_single,
        &test_sort_empty,
        &test_sort_doubles,
        &test_sort_with_duplicates,
        &test_binary_search_found,
        &test_binary_search_not_found,
        &test_binary_search_first,
        &test_binary_search_last,
        &test_binary_search_empty,
        &test_binary_search_null_params,
        &test_sort_large_elements,
        &test_lower_bound_basic,
        &test_lower_bound_exact,
        &test_lower_bound_all_less,
        &test_lower_bound_duplicates,
        &test_upper_bound_basic,
        &test_upper_bound_exact,
        &test_upper_bound_all_less_equal,
        &test_upper_bound_duplicates,
        &test_stable_sort_basic,
        &test_stable_sort_preserves_order,
        &test_stable_sort_empty,
        &test_stable_sort_single,
        &test_stable_sort_reverse,
    };
    const char *names[] = {
        "sort_already_sorted",
        "sort_reverse",
        "sort_random",
        "sort_single",
        "sort_empty",
        "sort_doubles",
        "sort_with_duplicates",
        "binary_search_found",
        "binary_search_not_found",
        "binary_search_first",
        "binary_search_last",
        "binary_search_empty",
        "binary_search_null_params",
        "sort_large_elements",
        "lower_bound_basic",
        "lower_bound_exact",
        "lower_bound_all_less",
        "lower_bound_duplicates",
        "upper_bound_basic",
        "upper_bound_exact",
        "upper_bound_all_less_equal",
        "upper_bound_duplicates",
        "stable_sort_basic",
        "stable_sort_preserves_order",
        "stable_sort_empty",
        "stable_sort_single",
        "stable_sort_reverse",
    };
    ZENIT_RUN_TESTS("sort", tests, names);
}
