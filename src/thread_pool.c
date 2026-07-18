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
#include <stdlib.h>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif

#define POOL_QUEUE_CAPACITY 4096

typedef struct {
    zenit_thread_task_fn fn;
    void *ctx;
} task_t;

struct zenit_thread_pool_t {
    size_t thread_count;
    volatile int running;

    task_t queue[POOL_QUEUE_CAPACITY];
    volatile size_t queue_head;
    volatile size_t queue_tail;

#if defined(_WIN32)
    HANDLE *threads;
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE cv_not_empty;
    CONDITION_VARIABLE cv_not_full;
#else
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t cv_not_empty;
    pthread_cond_t cv_not_full;
#endif
};

#if defined(_WIN32)
static DWORD WINAPI worker_thread(LPVOID arg) {
    zenit_thread_pool_t *pool = (zenit_thread_pool_t *)arg;

    while (1) {
        EnterCriticalSection(&pool->mutex);

        while (pool->queue_head == pool->queue_tail && pool->running) {
            SleepConditionVariableCS(&pool->cv_not_empty, &pool->mutex, INFINITE);
        }

        if (!pool->running && pool->queue_head == pool->queue_tail) {
            LeaveCriticalSection(&pool->mutex);
            break;
        }

        size_t index = pool->queue_tail % POOL_QUEUE_CAPACITY;
        task_t task = pool->queue[index];
        pool->queue_tail++;

        LeaveCriticalSection(&pool->mutex);
        WakeConditionVariable(&pool->cv_not_full);

        task.fn(task.ctx);
    }

    return 0;
}
#else
static void* worker_thread(void *arg) {
    zenit_thread_pool_t *pool = (zenit_thread_pool_t *)arg;

    while (1) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->queue_head == pool->queue_tail && pool->running) {
            pthread_cond_wait(&pool->cv_not_empty, &pool->mutex);
        }

        if (!pool->running && pool->queue_head == pool->queue_tail) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        size_t index = pool->queue_tail % POOL_QUEUE_CAPACITY;
        task_t task = pool->queue[index];
        pool->queue_tail++;

        pthread_mutex_unlock(&pool->mutex);
        pthread_cond_signal(&pool->cv_not_full);

        task.fn(task.ctx);
    }

    return NULL;
}
#endif

zenit_thread_pool_t* zenit_thread_pool_create(size_t thread_count) {
    if (thread_count == 0) {
        return NULL;
    }

    zenit_thread_pool_t *pool = malloc(sizeof(zenit_thread_pool_t));
    if (pool == NULL) {
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->running = 1;
    pool->queue_head = 0;
    pool->queue_tail = 0;

#if defined(_WIN32)
    pool->threads = malloc(thread_count * sizeof(HANDLE));
    if (pool->threads == NULL) {
        free(pool);
        return NULL;
    }

    InitializeCriticalSection(&pool->mutex);
    InitializeConditionVariable(&pool->cv_not_empty);
    InitializeConditionVariable(&pool->cv_not_full);

    for (size_t i = 0; i < thread_count; i++) {
        pool->threads[i] = CreateThread(NULL, 0, worker_thread, pool, 0, NULL);
        if (pool->threads[i] == NULL) {
            pool->running = 0;
            WakeAllConditionVariable(&pool->cv_not_empty);
            for (size_t j = 0; j < i; j++) {
                WaitForSingleObject(pool->threads[j], INFINITE);
                CloseHandle(pool->threads[j]);
            }
            DeleteCriticalSection(&pool->mutex);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }
#else
    pool->threads = malloc(thread_count * sizeof(pthread_t));
    if (pool->threads == NULL) {
        free(pool);
        return NULL;
    }

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cv_not_empty, NULL);
    pthread_cond_init(&pool->cv_not_full, NULL);

    for (size_t i = 0; i < thread_count; i++) {
        int rc = pthread_create(&pool->threads[i], NULL, worker_thread, pool);
        if (rc != 0) {
            pool->running = 0;
            pthread_cond_broadcast(&pool->cv_not_empty);
            for (size_t j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_mutex_destroy(&pool->mutex);
            pthread_cond_destroy(&pool->cv_not_empty);
            pthread_cond_destroy(&pool->cv_not_full);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }
#endif

    return pool;
}

void zenit_thread_pool_destroy(zenit_thread_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

#if defined(_WIN32)
    EnterCriticalSection(&pool->mutex);
    pool->running = 0;
    LeaveCriticalSection(&pool->mutex);
    WakeAllConditionVariable(&pool->cv_not_empty);

    for (size_t i = 0; i < pool->thread_count; i++) {
        WaitForSingleObject(pool->threads[i], INFINITE);
        CloseHandle(pool->threads[i]);
    }
    DeleteCriticalSection(&pool->mutex);
    free(pool->threads);
#else
    pthread_mutex_lock(&pool->mutex);
    pool->running = 0;
    pthread_mutex_unlock(&pool->mutex);
    pthread_cond_broadcast(&pool->cv_not_empty);

    for (size_t i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cv_not_empty);
    pthread_cond_destroy(&pool->cv_not_full);
    free(pool->threads);
#endif

    free(pool);
}

zenit_result_t zenit_thread_pool_enqueue(zenit_thread_pool_t *pool, zenit_thread_task_fn fn, void *ctx) {
    if (pool == NULL || fn == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

#if defined(_WIN32)
    EnterCriticalSection(&pool->mutex);

    while (pool->queue_head - pool->queue_tail >= POOL_QUEUE_CAPACITY && pool->running) {
        SleepConditionVariableCS(&pool->cv_not_full, &pool->mutex, INFINITE);
    }

    if (!pool->running) {
        LeaveCriticalSection(&pool->mutex);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_STATE);
    }

    size_t index = pool->queue_head % POOL_QUEUE_CAPACITY;
    pool->queue[index].fn = fn;
    pool->queue[index].ctx = ctx;
    pool->queue_head++;

    LeaveCriticalSection(&pool->mutex);
    WakeConditionVariable(&pool->cv_not_empty);
#else
    pthread_mutex_lock(&pool->mutex);

    while (pool->queue_head - pool->queue_tail >= POOL_QUEUE_CAPACITY && pool->running) {
        pthread_cond_wait(&pool->cv_not_full, &pool->mutex);
    }

    if (!pool->running) {
        pthread_mutex_unlock(&pool->mutex);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_STATE);
    }

    size_t index = pool->queue_head % POOL_QUEUE_CAPACITY;
    pool->queue[index].fn = fn;
    pool->queue[index].ctx = ctx;
    pool->queue_head++;

    pthread_mutex_unlock(&pool->mutex);
    pthread_cond_signal(&pool->cv_not_empty);
#endif

    return ZENIT_RESULT_OK;
}

size_t zenit_thread_pool_thread_count(const zenit_thread_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }
    return pool->thread_count;
}

size_t zenit_thread_pool_pending(const zenit_thread_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }
    return pool->queue_head - pool->queue_tail;
}

void zenit_thread_pool_wait(zenit_thread_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

#if defined(_WIN32)
    EnterCriticalSection(&pool->mutex);
    while (pool->queue_head != pool->queue_tail) {
        LeaveCriticalSection(&pool->mutex);
        SwitchToThread();
        EnterCriticalSection(&pool->mutex);
    }
    LeaveCriticalSection(&pool->mutex);
#else
    pthread_mutex_lock(&pool->mutex);
    while (pool->queue_head != pool->queue_tail) {
        pthread_mutex_unlock(&pool->mutex);
        sched_yield();
        pthread_mutex_lock(&pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);
#endif
}
