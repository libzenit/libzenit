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

#ifndef LIBZENIT_SORT_H
#define LIBZENIT_SORT_H

#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Comparison function type for sorting and searching.
 *
 * @param a Pointer to the first element.
 * @param b Pointer to the second element.
 * @return Negative if *a < *b, zero if *a == *b, positive if *a > *b.
 */
typedef int (*zenit_sort_compare_fn_t)(const void *a, const void *b);

/**
 * @brief Sort an array in-place using quicksort (median-of-three pivot).
 *
 * Sorts @p count elements of size @p elem_size starting at @p base,
 * ordered by @p compare.  The sort is not stable.
 *
 * @param base      Pointer to the start of the array.
 * @param count     Number of elements in the array.
 * @param elem_size Size in bytes of each element (must be > 0).
 * @param compare   Comparison function (must not be NULL).
 */
void zenit_sort_quick(void *base, size_t count, size_t elem_size, zenit_sort_compare_fn_t compare);

/**
 * @brief Search a sorted array using binary search.
 *
 * Finds an element equal to @p key in a sorted array of @p count elements.
 * If multiple elements match, which one is returned is unspecified.
 *
 * @param key       Pointer to the value to search for.
 * @param base      Pointer to the start of the sorted array.
 * @param count     Number of elements in the array.
 * @param elem_size Size in bytes of each element (must be > 0).
 * @param compare   Comparison function (must not be NULL).
 * @return Pointer to a matching element, or NULL if not found.
 */
void *zenit_binary_search(const void *key, const void *base, size_t count, size_t elem_size, zenit_sort_compare_fn_t compare);

/**
 * @brief Find the first element not less than @p key (lower bound).
 *
 * Returns a pointer to the first element in the sorted array that is
 * not less than @p key, or NULL if all elements are less.
 *
 * @param key       Pointer to the value to search for.
 * @param base      Pointer to the start of the sorted array.
 * @param count     Number of elements in the array.
 * @param elem_size Size in bytes of each element (must be > 0).
 * @param compare   Comparison function (must not be NULL).
 * @return Pointer to the first element >= key, or NULL if none.
 */
void *zenit_lower_bound(const void *key, const void *base, size_t count, size_t elem_size, zenit_sort_compare_fn_t compare);

/**
 * @brief Find the first element greater than @p key (upper bound).
 *
 * Returns a pointer to the first element in the sorted array that is
 * greater than @p key, or NULL if all elements are less or equal.
 *
 * @param key       Pointer to the value to search for.
 * @param base      Pointer to the start of the sorted array.
 * @param count     Number of elements in the array.
 * @param elem_size Size in bytes of each element (must be > 0).
 * @param compare   Comparison function (must not be NULL).
 * @return Pointer to the first element > key, or NULL if none.
 */
void *zenit_upper_bound(const void *key, const void *base, size_t count, size_t elem_size, zenit_sort_compare_fn_t compare);

/**
 * @brief Sort an array in-place using merge sort (stable).
 *
 * Sorts @p count elements of size @p elem_size starting at @p base,
 * ordered by @p compare.  The sort is stable: equal elements retain
 * their original relative order.
 *
 * Allocates a temporary buffer of @p count * @p elem_size bytes.
 * Returns ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC) on allocation failure.
 *
 * @param base      Pointer to the start of the array.
 * @param count     Number of elements in the array.
 * @param elem_size Size in bytes of each element (must be > 0).
 * @param compare   Comparison function (must not be NULL).
 * @return ZENIT_OK on success, or an error code on failure.
 */
zenit_result_t zenit_sort_stable(void *base, size_t count, size_t elem_size, zenit_sort_compare_fn_t compare);

#endif
