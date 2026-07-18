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

#include <libzenit/str.h>
#include "test_malloc_fail.h"
#include <stdio.h>
#include <stdlib.h>

void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_realloc(ptr, size);
}

int main(void) {
    int failed = 0;

    /* split: first malloc (result array) fails */
    {
        malloc_fail_countdown = 0;
        size_t count;
        char **parts = zenit_str_split("a,b,c", ",", &count);
        malloc_fail_countdown = -1;
        if (parts != NULL) {
            fprintf(stderr, "FAIL: split countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: split first alloc fail\n");
        }
        free(parts);
    }

    /* split: realloc fails during growth */
    {
        malloc_fail_countdown = 4;
        size_t count;
        char **parts = zenit_str_split("a,b,c,d,e", ",", &count);
        malloc_fail_countdown = -1;
        if (parts != NULL) {
            fprintf(stderr, "FAIL: split realloc countdown=4 should return NULL\n");
            failed++;
        } else {
            printf("PASS: split realloc fail\n");
        }
        free(parts);
    }

    /* split: token malloc fails after realloc succeeded */
    {
        malloc_fail_countdown = 6;
        size_t count;
        char **parts = zenit_str_split("a,b,c,d,e", ",", &count);
        malloc_fail_countdown = -1;
        if (parts != NULL) {
            fprintf(stderr, "FAIL: split token countdown=6 should return NULL\n");
            failed++;
        } else {
            printf("PASS: split token alloc fail\n");
        }
        free(parts);
    }

    /* split_with_allocator: first alloc fails */
    {
        malloc_fail_countdown = 0;
        size_t count;
        char **parts = zenit_str_split_with_allocator("a,b,c", ",", &count, ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (parts != NULL) {
            fprintf(stderr, "FAIL: split_with_allocator countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: split_with_allocator first alloc fail\n");
        }
        free(parts);
    }

    /* trim_with_allocator: alloc fails */
    {
        malloc_fail_countdown = 0;
        char *trimmed = zenit_str_trim_with_allocator("  hello  ", ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (trimmed != NULL) {
            fprintf(stderr, "FAIL: trim_with_allocator countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: trim_with_allocator alloc fail\n");
        }
        free(trimmed);
    }

    /* join_with_allocator: alloc fails */
    {
        const char *parts[] = {"a", "b", "c"};
        malloc_fail_countdown = 0;
        char *joined = zenit_str_join_with_allocator(parts, 3, ",", ZENIT_ALLOCATOR_DEFAULT);
        malloc_fail_countdown = -1;
        if (joined != NULL) {
            fprintf(stderr, "FAIL: join_with_allocator countdown=0 should return NULL\n");
            failed++;
        } else {
            printf("PASS: join_with_allocator alloc fail\n");
        }
        free(joined);
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
