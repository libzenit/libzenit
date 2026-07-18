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

#include <libzenit/queue.h>
#include <stdio.h>
#include "test_malloc_fail.h"

int main(void) {
    int failed = 0;

    /* create: handle malloc fails */
    {
        malloc_fail_countdown = 0;
        zenit_queue_t *q = zenit_queue_create(sizeof(int));
        malloc_fail_countdown = -1;
        if (q != NULL) {
            fprintf(stderr, "FAIL: create handle malloc fail should return NULL\n");
            zenit_queue_destroy(q);
            failed++;
        } else {
            printf("PASS: queue create handle malloc fail\n");
        }
    }

    /* create: deque malloc fails (after handle succeeds) */
    {
        malloc_fail_countdown = 1;
        zenit_queue_t *q = zenit_queue_create(sizeof(int));
        malloc_fail_countdown = -1;
        if (q != NULL) {
            fprintf(stderr, "FAIL: create deque malloc fail should return NULL\n");
            zenit_queue_destroy(q);
            failed++;
        } else {
            printf("PASS: queue create deque malloc fail\n");
        }
    }

    if (failed > 0) {
        fprintf(stderr, "%d tests failed\n", failed);
        return 1;
    }
    return 0;
}
