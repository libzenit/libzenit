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

#ifndef LIBZENIT_THREAD_POOL_H
#define LIBZENIT_THREAD_POOL_H

#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Task function executed by a thread pool worker.
 *
 * @param ctx User-supplied context pointer.
 */
typedef void (*zenit_thread_task_fn)(void *ctx);

/**
 * @brief Opaque thread pool handle.
 */
typedef struct zenit_thread_pool_t zenit_thread_pool_t;

/**
 * @brief Create a thread pool with a fixed number of worker threads.
 *
 * Workers are created immediately and begin waiting for tasks.
 *
 * @param thread_count Number of worker threads.  Must be > 0.
 * @return New thread pool, or NULL on invalid params or OOM.
 */
zenit_thread_pool_t* zenit_thread_pool_create(size_t thread_count);

/**
 * @brief Destroy a thread pool.
 *
 * Waits for all queued tasks to complete, then shuts down all workers.
 * NULL-safe.
 *
 * @param pool Thread pool to destroy.
 */
void zenit_thread_pool_destroy(zenit_thread_pool_t *pool);

/**
 * @brief Enqueue a task for execution.
 *
 * The task is executed by the next available worker thread.  Tasks are
 * executed in FIFO order.
 *
 * @param pool Thread pool handle.
 * @param fn   Task function to execute.
 * @param ctx  User context passed to the task function.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_NULL if pool is NULL,
 *         ZENIT_ERROR_ALLOC if the task queue is full.
 */
zenit_result_t zenit_thread_pool_enqueue(zenit_thread_pool_t *pool, zenit_thread_task_fn fn, void *ctx);

/**
 * @brief Get the number of worker threads.
 *
 * @param pool Thread pool handle.
 * @return Number of threads (0 if NULL).
 */
size_t zenit_thread_pool_thread_count(const zenit_thread_pool_t *pool);

/**
 * @brief Get the number of tasks currently queued or in progress.
 *
 * @param pool Thread pool handle.
 * @return Number of pending tasks (0 if NULL).
 */
size_t zenit_thread_pool_pending(const zenit_thread_pool_t *pool);

/**
 * @brief Wait for all queued tasks to complete.
 *
 * Blocks the calling thread until the task queue is empty and all workers
 * are idle.
 *
 * @param pool Thread pool handle.
 */
void zenit_thread_pool_wait(zenit_thread_pool_t *pool);

#endif
