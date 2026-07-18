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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ─── Internal: merge two sorted halves ─── */
static void merge(void *base, size_t es, size_t lo, size_t mid, size_t hi,
                  unsigned char *buf, zenit_sort_compare_fn_t compare) {
    unsigned char *b = (unsigned char *)base;
    size_t i = lo;
    size_t j = mid;
    size_t k = 0;

    while (i < mid && j < hi) {
        if (compare(b + i * es, b + j * es) <= 0) {
            memcpy(buf + k * es, b + i * es, es);
            i++;
        } else {
            memcpy(buf + k * es, b + j * es, es);
            j++;
        }
        k++;
    }

    while (i < mid) {
        memcpy(buf + k * es, b + i * es, es);
        i++; k++;
    }
    while (j < hi) {
        memcpy(buf + k * es, b + j * es, es);
        j++; k++;
    }

    memcpy(b + lo * es, buf, k * es);
}

static void mergesort_impl(void *base, size_t es, size_t lo, size_t hi,
                           unsigned char *buf, zenit_sort_compare_fn_t compare) {
    if (hi - lo < 2) return;
    size_t mid = lo + (hi - lo) / 2;
    mergesort_impl(base, es, lo, mid, buf, compare);
    mergesort_impl(base, es, mid, hi, buf, compare);
    merge(base, es, lo, mid, hi, buf, compare);
}

/* Swap two elements of size @p es */
static void swap_elements(void *a, void *b, size_t es) {
    if (es <= 64) {
        char tmp[64];
        memcpy(tmp, a, es);
        memcpy(a, b, es);
        memcpy(b, tmp, es);
    } else {
        /* Byte-by-byte swap avoids heap allocation — deterministic, no failure */
        unsigned char *ca = (unsigned char *)a;
        unsigned char *cb = (unsigned char *)b;
        for (size_t i = 0; i < es; i++) {
            unsigned char t = ca[i];
            ca[i] = cb[i];
            cb[i] = t;
        }
    }
}

static size_t median_of_three(void *base, size_t es, size_t lo, size_t hi,
                               zenit_sort_compare_fn_t compare) {
    size_t mid = lo + (hi - lo) / 2;
    char *b = (char *)base;
    if (compare(b + lo * es, b + mid * es) > 0) swap_elements(b + lo * es, b + mid * es, es);
    if (compare(b + lo * es, b + hi * es) > 0) swap_elements(b + lo * es, b + hi * es, es);
    if (compare(b + mid * es, b + hi * es) > 0) swap_elements(b + mid * es, b + hi * es, es);
    return mid;
}

static size_t partition(void *base, size_t es, size_t lo, size_t hi,
                        zenit_sort_compare_fn_t compare) {
    size_t mid = median_of_three(base, es, lo, hi, compare);
    char *b = (char *)base;
    swap_elements(b + lo * es, b + mid * es, es);

    const void *pivot = b + lo * es;
    size_t i = lo - 1;
    size_t j = hi + 1;

    for (;;) {
        do { i++; } while (compare(b + i * es, pivot) < 0);
        do { j--; } while (compare(b + j * es, pivot) > 0);
        if (i >= j) return j;
        swap_elements(b + i * es, b + j * es, es);
    }
}

static void quicksort(void *base, size_t es, size_t lo, size_t hi,
                      zenit_sort_compare_fn_t compare) {
    if (lo < hi) {
        size_t p = partition(base, es, lo, hi, compare);
        if (p - lo < hi - p) {
            quicksort(base, es, lo, p, compare);
            quicksort(base, es, p + 1, hi, compare);
        } else {
            quicksort(base, es, p + 1, hi, compare);
            quicksort(base, es, lo, p, compare);
        }
    }
}

void zenit_sort_quick(void *base, size_t count, size_t elem_size,
                      zenit_sort_compare_fn_t compare) {
    if (base == NULL || count < 2 || elem_size == 0 || compare == NULL) return;
    quicksort(base, elem_size, 0, count - 1, compare);
}

void *zenit_binary_search(const void *key, const void *base, size_t count,
                          size_t elem_size, zenit_sort_compare_fn_t compare) {
    if (key == NULL || base == NULL || elem_size == 0 || compare == NULL) return NULL;

    const unsigned char *b = (const unsigned char *)base;
    size_t lo = 0;
    size_t hi = count;
    size_t match = (size_t)-1;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        int cmp = compare(key, b + mid * elem_size);
        if (cmp == 0) {
            match = mid;
            break;
        } else if (cmp < 0) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    if (match != (size_t)-1) {
        return (void *)((uintptr_t)base + match * elem_size);
    }
    return NULL;
}

void *zenit_lower_bound(const void *key, const void *base, size_t count,
                        size_t elem_size, zenit_sort_compare_fn_t compare) {
    if (key == NULL || base == NULL || elem_size == 0 || compare == NULL) return NULL;

    const unsigned char *b = (const unsigned char *)base;
    size_t lo = 0;
    size_t hi = count;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (compare(key, b + mid * elem_size) <= 0) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    if (lo < count) {
        return (void *)((uintptr_t)base + lo * elem_size);
    }
    return NULL;
}

void *zenit_upper_bound(const void *key, const void *base, size_t count,
                        size_t elem_size, zenit_sort_compare_fn_t compare) {
    if (key == NULL || base == NULL || elem_size == 0 || compare == NULL) return NULL;

    const unsigned char *b = (const unsigned char *)base;
    size_t lo = 0;
    size_t hi = count;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (compare(key, b + mid * elem_size) < 0) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    if (lo < count) {
        return (void *)((uintptr_t)base + lo * elem_size);
    }
    return NULL;
}

zenit_result_t zenit_sort_stable(void *base, size_t count, size_t elem_size,
                                 zenit_sort_compare_fn_t compare) {
    if (base == NULL || count < 2 || elem_size == 0 || compare == NULL) {
        return ZENIT_RESULT_OK;
    }

    unsigned char *buf = malloc(count * elem_size);
    if (buf == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    mergesort_impl(base, elem_size, 0, count, buf, compare);
    free(buf);
    return ZENIT_RESULT_OK;
}
