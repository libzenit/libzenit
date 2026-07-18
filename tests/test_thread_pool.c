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

#include <libzenit/thread_pool.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#define ATOMIC_INC(var) InterlockedIncrement(&var)
#define ATOMIC_READ(var) ((long)InterlockedExchangeAdd(&var, 0))
#else
#define ATOMIC_INC(var) __sync_fetch_and_add(&(var), 1)
#define ATOMIC_READ(var) __sync_fetch_and_add(&(var), 0)
#endif

static volatile long test_counter = 0;

static void increment_task(void *ctx) {
    (void)ctx;
    ATOMIC_INC(test_counter);
}

int main(void) {
    /* Test 1: Create/destroy */
    {
        zenit_thread_pool_t *pool = zenit_thread_pool_create(4);
        if (pool == NULL) {
            fprintf(stderr, "FAIL: thread pool create\n");
            return 1;
        }
        if (zenit_thread_pool_thread_count(pool) != 4) {
            fprintf(stderr, "FAIL: thread count\n");
            zenit_thread_pool_destroy(pool);
            return 1;
        }
        zenit_thread_pool_destroy(pool);
    }
    printf("PASS: thread pool create/destroy\n");

    /* Test 2: Enqueue tasks */
    {
        zenit_thread_pool_t *pool = zenit_thread_pool_create(2);
        test_counter = 0;

        for (int i = 0; i < 10; i++) {
            zenit_result_t r = zenit_thread_pool_enqueue(pool, increment_task, NULL);
            if (r.error != ZENIT_OK) {
                fprintf(stderr, "FAIL: enqueue\n");
                zenit_thread_pool_destroy(pool);
                return 1;
            }
        }

        zenit_thread_pool_wait(pool);

        if (ATOMIC_READ(test_counter) != 10) {
            fprintf(stderr, "FAIL: counter %ld != 10\n", ATOMIC_READ(test_counter));
            zenit_thread_pool_destroy(pool);
            return 1;
        }

        zenit_thread_pool_destroy(pool);
    }
    printf("PASS: thread pool enqueue\n");

    /* Test 3: Pending count */
    {
        zenit_thread_pool_t *pool = zenit_thread_pool_create(1);
        test_counter = 0;

        zenit_thread_pool_enqueue(pool, increment_task, NULL);
        zenit_thread_pool_wait(pool);

        if (zenit_thread_pool_pending(pool) != 0) {
            fprintf(stderr, "FAIL: pending should be 0\n");
            zenit_thread_pool_destroy(pool);
            return 1;
        }

        zenit_thread_pool_destroy(pool);
    }
    printf("PASS: thread pool pending\n");

    /* Test 4: NULL params */
    {
        zenit_result_t r = zenit_thread_pool_enqueue(NULL, increment_task, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: enqueue NULL pool\n");
            return 1;
        }
        zenit_thread_pool_t *pool = zenit_thread_pool_create(1);
        r = zenit_thread_pool_enqueue(pool, NULL, NULL);
        if (r.error != ZENIT_ERROR_NULL) {
            fprintf(stderr, "FAIL: enqueue NULL fn\n");
            zenit_thread_pool_destroy(pool);
            return 1;
        }
        if (zenit_thread_pool_thread_count(NULL) != 0) {
            fprintf(stderr, "FAIL: thread count NULL\n");
            zenit_thread_pool_destroy(pool);
            return 1;
        }
        if (zenit_thread_pool_pending(NULL) != 0) {
            fprintf(stderr, "FAIL: pending NULL\n");
            zenit_thread_pool_destroy(pool);
            return 1;
        }
        zenit_thread_pool_destroy(NULL);
        zenit_thread_pool_wait(NULL);
        zenit_thread_pool_destroy(pool);
    }
    printf("PASS: thread pool NULL params\n");

    /* Test 5: Zero threads */
    {
        zenit_thread_pool_t *pool = zenit_thread_pool_create(0);
        if (pool != NULL) {
            fprintf(stderr, "FAIL: create 0 threads should fail\n");
            zenit_thread_pool_destroy(pool);
            return 1;
        }
    }
    printf("PASS: thread pool zero threads\n");

    /* Test 6: Single thread */
    {
        zenit_thread_pool_t *pool = zenit_thread_pool_create(1);
        test_counter = 0;

        for (int i = 0; i < 5; i++) {
            zenit_thread_pool_enqueue(pool, increment_task, NULL);
        }

        zenit_thread_pool_wait(pool);

        if (ATOMIC_READ(test_counter) != 5) {
            fprintf(stderr, "FAIL: single thread counter %ld != 5\n", ATOMIC_READ(test_counter));
            zenit_thread_pool_destroy(pool);
            return 1;
        }

        zenit_thread_pool_destroy(pool);
    }
    printf("PASS: thread pool single thread\n");

    printf("PASS: thread_pool\n");
    return 0;
}
