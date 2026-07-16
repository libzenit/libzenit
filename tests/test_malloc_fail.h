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

#ifndef LIBZENIT_TEST_MALLOC_FAIL_H
#define LIBZENIT_TEST_MALLOC_FAIL_H

#include <stdlib.h>

/*
 * Shared malloc/calloc wrapper infrastructure for allocation-failure tests.
 *
 * Each test executable using this header MUST also link with:
 *   -Wl,--wrap=malloc -Wl,--wrap=calloc
 *
 * The countdown is reset to -1 before printf/stdio calls to avoid
 * accidental interception of libc allocations.
 *
 * Usage:
 *   malloc_fail_countdown = 0;  // next malloc/calloc returns NULL
 *   malloc_fail_countdown = 2;  // first two succeed, third fails
 *   malloc_fail_countdown = -1; // never fail (default)
 */

static int malloc_fail_countdown = -1;

void *__real_malloc(size_t size);
void *__wrap_malloc(size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_malloc(size);
}

void *__real_calloc(size_t nmemb, size_t size);
void *__wrap_calloc(size_t nmemb, size_t size) {
    if (malloc_fail_countdown == 0) {
        return NULL;
    }
    if (malloc_fail_countdown > 0) {
        malloc_fail_countdown--;
    }
    return __real_calloc(nmemb, size);
}

#endif
