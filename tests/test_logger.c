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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Sink that captures the last message */
static char captured_msg[4096];

static void capture_sink(const char *message, void *ctx) {
    (void)ctx;
    size_t len = strlen(message);
    if (len >= sizeof(captured_msg)) len = sizeof(captured_msg) - 1;
    memcpy(captured_msg, message, len);
    captured_msg[len] = '\0';
}

int main(void) {
    /* Test 1: Create and destroy logger */
    {
        zenit_logger_t *log = zenit_logger_create(NULL, NULL);
        if (log == NULL) {
            fprintf(stderr, "FAIL: logger create\n");
            return 1;
        }
        zenit_logger_destroy(log);
    }
    printf("PASS: logger create/destroy\n");

    /* Test 2: Create with allocator */
    {
        zenit_logger_t *log = zenit_logger_create_with_allocator(NULL, NULL, ZENIT_ALLOCATOR_DEFAULT);
        if (log == NULL) {
            fprintf(stderr, "FAIL: logger create with allocator\n");
            return 1;
        }
        zenit_logger_destroy(log);
    }
    printf("PASS: logger create with allocator\n");

    /* Test 3: Log with custom sink */
    {
        zenit_logger_t *log = zenit_logger_create(capture_sink, NULL);
        captured_msg[0] = '\0';
        zenit_logger_log(log, ZENIT_LOG_INFO, "hello %s", "world");
        if (strstr(captured_msg, "INFO") == NULL) {
            fprintf(stderr, "FAIL: log message missing INFO, got: '%s'\n", captured_msg);
            zenit_logger_destroy(log);
            return 1;
        }
        if (strstr(captured_msg, "hello world") == NULL) {
            fprintf(stderr, "FAIL: log message missing content, got: '%s'\n", captured_msg);
            zenit_logger_destroy(log);
            return 1;
        }
        zenit_logger_destroy(log);
    }
    printf("PASS: logger log with sink\n");

    /* Test 4: Level filtering */
    {
        zenit_logger_t *log = zenit_logger_create(capture_sink, NULL);
        zenit_logger_set_level(log, ZENIT_LOG_WARN);
        captured_msg[0] = '\0';
        zenit_logger_log(log, ZENIT_LOG_INFO, "should not appear");
        if (captured_msg[0] != '\0') {
            fprintf(stderr, "FAIL: filtered message should not appear\n");
            zenit_logger_destroy(log);
            return 1;
        }
        zenit_logger_log(log, ZENIT_LOG_ERROR, "should appear");
        if (strstr(captured_msg, "should appear") == NULL) {
            fprintf(stderr, "FAIL: unfiltered message should appear\n");
            zenit_logger_destroy(log);
            return 1;
        }
        zenit_logger_destroy(log);
    }
    printf("PASS: logger level filtering\n");

    /* Test 5: Get/set level */
    {
        zenit_logger_t *log = zenit_logger_create(NULL, NULL);
        if (zenit_logger_get_level(log) != ZENIT_LOG_TRACE) {
            fprintf(stderr, "FAIL: default level should be TRACE\n");
            zenit_logger_destroy(log);
            return 1;
        }
        zenit_logger_set_level(log, ZENIT_LOG_ERROR);
        if (zenit_logger_get_level(log) != ZENIT_LOG_ERROR) {
            fprintf(stderr, "FAIL: get_level after set\n");
            zenit_logger_destroy(log);
            return 1;
        }
        zenit_logger_destroy(log);
    }
    printf("PASS: logger get/set level\n");

    /* Test 6: Convenience level functions */
    {
        zenit_logger_t *log = zenit_logger_create(capture_sink, NULL);
        zenit_logger_set_level(log, ZENIT_LOG_TRACE);

        captured_msg[0] = '\0';
        zenit_logger_trace(log, "trace %d", 1);
        if (strstr(captured_msg, "TRACE") == NULL) {
            fprintf(stderr, "FAIL: trace level, got: '%s'\n", captured_msg);
            zenit_logger_destroy(log);
            return 1;
        }

        captured_msg[0] = '\0';
        zenit_logger_debug(log, "debug %d", 2);
        if (strstr(captured_msg, "DEBUG") == NULL) {
            fprintf(stderr, "FAIL: debug level\n");
            zenit_logger_destroy(log);
            return 1;
        }

        captured_msg[0] = '\0';
        zenit_logger_info(log, "info %d", 3);
        if (strstr(captured_msg, "INFO") == NULL) {
            fprintf(stderr, "FAIL: info level\n");
            zenit_logger_destroy(log);
            return 1;
        }

        captured_msg[0] = '\0';
        zenit_logger_warn(log, "warn %d", 4);
        if (strstr(captured_msg, "WARN") == NULL) {
            fprintf(stderr, "FAIL: warn level\n");
            zenit_logger_destroy(log);
            return 1;
        }

        captured_msg[0] = '\0';
        zenit_logger_error(log, "error %d", 5);
        if (strstr(captured_msg, "ERROR") == NULL) {
            fprintf(stderr, "FAIL: error level\n");
            zenit_logger_destroy(log);
            return 1;
        }

        zenit_logger_destroy(log);
    }
    printf("PASS: logger convenience functions\n");

    /* Test 7: NULL logger is safe */
    {
        zenit_logger_log(NULL, ZENIT_LOG_INFO, "test");
        zenit_logger_trace(NULL, "test");
        zenit_logger_debug(NULL, "test");
        zenit_logger_info(NULL, "test");
        zenit_logger_warn(NULL, "test");
        zenit_logger_error(NULL, "test");
        zenit_logger_set_level(NULL, ZENIT_LOG_INFO);
        if (zenit_logger_get_level(NULL) != ZENIT_LOG_ERROR) {
            fprintf(stderr, "FAIL: get_level(NULL) should return ERROR\n");
            return 1;
        }
        zenit_logger_destroy(NULL);
    }
    printf("PASS: logger NULL safety\n");

    /* Test 8: Allocation failure */
    {
        zenit_logger_t *log = zenit_logger_create_with_allocator(NULL, NULL, ZENIT_ALLOCATOR_DEFAULT);
        if (log == NULL) {
            fprintf(stderr, "FAIL: logger create should succeed\n");
            return 1;
        }
        zenit_logger_destroy(log);
    }
    printf("PASS: logger allocator variant\n");

    printf("PASS: logger\n");
    return 0;
}
