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

#ifndef LIBZENIT_LOGGER_H
#define LIBZENIT_LOGGER_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Log severity levels.
 */
typedef enum {
    ZENIT_LOG_TRACE = 0, /**< Finest-grained debugging information */
    ZENIT_LOG_DEBUG = 1, /**< Debug-level messages */
    ZENIT_LOG_INFO = 2,  /**< Informational messages */
    ZENIT_LOG_WARN = 3,  /**< Warning conditions */
    ZENIT_LOG_ERROR = 4, /**< Error conditions */
    ZENIT_LOG_FATAL = 5, /**< Fatal conditions (triggers abort by default) */
} zenit_log_level_t;

/**
 * @brief Log sink function.
 *
 * Called for each log message.  The @p message string includes the full
 * formatted log line (with timestamp, level, etc.) and is null-terminated.
 * The sink must not free the message.
 *
 * @param message The formatted log line.
 * @param ctx     User-supplied context pointer.
 */
typedef void (*zenit_log_sink_fn)(const char *message, void *ctx);

/**
 * @brief Opaque logger handle.
 */
typedef struct zenit_logger_t zenit_logger_t;

/**
 * @brief Create a logger with a custom sink (default allocator).
 *
 * @param sink     Callback invoked for each log message.  If NULL, messages
 *                 are written to stderr via fprintf.
 * @param sink_ctx User context passed to the sink.  Ignored if sink is NULL.
 * @return New logger, or NULL on allocation failure.
 */
zenit_logger_t* zenit_logger_create(zenit_log_sink_fn sink, void *sink_ctx);

/**
 * @brief Create a logger with a custom sink (custom allocator).
 *
 * @param sink      Callback invoked for each log message.  If NULL, uses stderr.
 * @param sink_ctx  User context passed to the sink.
 * @param allocator Custom allocator for the logger and internal buffers.
 * @return New logger, or NULL on allocation failure.
 */
zenit_logger_t* zenit_logger_create_with_allocator(zenit_log_sink_fn sink, void *sink_ctx, zenit_allocator_t allocator);

/**
 * @brief Destroy the logger.  NULL-safe.
 *
 * @param logger Logger to destroy.
 */
void zenit_logger_destroy(zenit_logger_t *logger);

/**
 * @brief Set the minimum log level.  Messages below this level are dropped.
 *
 * Default is ZENIT_LOG_TRACE (log everything).
 *
 * @param logger Logger handle.
 * @param level  Minimum level to emit.
 */
void zenit_logger_set_level(zenit_logger_t *logger, zenit_log_level_t level);

/**
 * @brief Get the current minimum log level.
 *
 * @param logger Logger handle (may be NULL, returns ZENIT_LOG_ERROR).
 * @return Current minimum level.
 */
zenit_log_level_t zenit_logger_get_level(const zenit_logger_t *logger);

/**
 * @brief Log a message with format string (printf-style).
 *
 * @param logger Logger handle.  If NULL, the message is silently dropped.
 * @param level  Severity level.  If below the configured minimum, the
 *               message is dropped.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_log(zenit_logger_t *logger, zenit_log_level_t level, const char *fmt, ...);

/**
 * @brief Log a message at TRACE level.
 *
 * @param logger Logger handle.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_trace(zenit_logger_t *logger, const char *fmt, ...);

/**
 * @brief Log a message at DEBUG level.
 *
 * @param logger Logger handle.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_debug(zenit_logger_t *logger, const char *fmt, ...);

/**
 * @brief Log a message at INFO level.
 *
 * @param logger Logger handle.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_info(zenit_logger_t *logger, const char *fmt, ...);

/**
 * @brief Log a message at WARN level.
 *
 * @param logger Logger handle.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_warn(zenit_logger_t *logger, const char *fmt, ...);

/**
 * @brief Log a message at ERROR level.
 *
 * @param logger Logger handle.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_error(zenit_logger_t *logger, const char *fmt, ...);

/**
 * @brief Log a message at FATAL level and abort.
 *
 * After logging, calls abort().  The logger is flushed before aborting.
 *
 * @param logger Logger handle.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void zenit_logger_fatal(zenit_logger_t *logger, const char *fmt, ...);

#endif
