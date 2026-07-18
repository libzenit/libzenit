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

#include <libzenit/logger.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

/* Internal buffer size for formatted log messages */
#define LOGGER_BUF_SIZE 4096

struct zenit_logger_t {
    zenit_log_level_t level;
    zenit_log_sink_fn sink;
    void *sink_ctx;
    zenit_allocator_t allocator;
};

/* Level label strings — kept short for alignment */
static const char* level_label(zenit_log_level_t lvl) {
    switch (lvl) {
        case ZENIT_LOG_TRACE: return "TRACE";
        case ZENIT_LOG_DEBUG: return "DEBUG";
        case ZENIT_LOG_INFO:  return "INFO";
        case ZENIT_LOG_WARN:  return "WARN";
        case ZENIT_LOG_ERROR: return "ERROR";
        case ZENIT_LOG_FATAL: return "FATAL";
    }
    return "?????";
}

/* Default sink: writes to stderr */
static void default_sink(const char *message, void *ctx) {
    (void)ctx;
    fprintf(stderr, "%s\n", message);
}

zenit_logger_t* zenit_logger_create(zenit_log_sink_fn sink, void *sink_ctx) {
    return zenit_logger_create_with_allocator(sink, sink_ctx, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_logger_t* zenit_logger_create_with_allocator(zenit_log_sink_fn sink, void *sink_ctx, zenit_allocator_t allocator) {
    zenit_logger_t *logger = allocator.alloc_fn(sizeof(zenit_logger_t), allocator.ctx);
    if (logger == NULL) {
        return NULL;
    }

    logger->level = ZENIT_LOG_TRACE;
    logger->sink = sink ? sink : &default_sink;
    logger->sink_ctx = sink ? sink_ctx : NULL;
    logger->allocator = allocator;

    return logger;
}

void zenit_logger_destroy(zenit_logger_t *logger) {
    if (logger == NULL) {
        return;
    }
    zenit_allocator_t a = logger->allocator;
    a.free_fn(logger, a.ctx);
}

void zenit_logger_set_level(zenit_logger_t *logger, zenit_log_level_t level) {
    if (logger == NULL) {
        return;
    }
    logger->level = level;
}

zenit_log_level_t zenit_logger_get_level(const zenit_logger_t *logger) {
    if (logger == NULL) {
        return ZENIT_LOG_ERROR;
    }
    return logger->level;
}

void zenit_logger_log(zenit_logger_t *logger, zenit_log_level_t level, const char *fmt, ...) {
    if (logger == NULL) {
        return;
    }

    /* Filter by level */
    if (level < logger->level) {
        return;
    }

    /* Get current timestamp */
    time_t now = time(NULL);
    const struct tm *tm_info;

#if defined(_WIN32)
    struct tm tm_buf;
    tm_info = localtime_s(&tm_buf, &now) == 0 ? &tm_buf : NULL;
#else
    struct tm tm_result;
    tm_info = localtime_r(&now, &tm_result);
#endif

    /* Format: "HH:MM:SS LEVEL message" */
    char timestamp[32] = {0};
    if (tm_info != NULL) {
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    } else {
        memcpy(timestamp, "??:??:??", 9);
    }

    /* Format the user message */
    char user_msg[LOGGER_BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int user_len = vsnprintf(user_msg, sizeof(user_msg), fmt, args);
    va_end(args);

    if (user_len < 0) {
        /* Format error — still try to log something */
        user_msg[0] = '\0';
    } else if ((size_t)user_len >= sizeof(user_msg)) {
        /* Truncated — message is safe, just clipped */
    }

    /* Build final message: "HH:MM:SS LEVEL msg" */
    char final_msg[LOGGER_BUF_SIZE + 64];
    int final_len = snprintf(final_msg, sizeof(final_msg), "%s %-5s %s",
                             timestamp, level_label(level), user_msg);

    if (final_len < 0) {
        final_msg[0] = '\0';
    }

    /* Send to sink */
    logger->sink(final_msg, logger->sink_ctx);

    /* FATAL → abort after logging */
    if (level == ZENIT_LOG_FATAL) {
        abort();
    }
}

/* Convenience per-level functions are now macros defined in logger.h */
