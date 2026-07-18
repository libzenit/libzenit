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

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *failing_alloc(size_t size, void *ctx) {
    (void)size; (void)ctx;
    return NULL;
}

int main(void) {
    /* Verify every known error code produces a non-empty string */
    zenit_error_t codes[] = {
        ZENIT_OK,
        ZENIT_ERROR_NULL,
        ZENIT_ERROR_ALLOC,
        ZENIT_ERROR_PARAM,
        ZENIT_ERROR_NOT_FOUND,
        ZENIT_ERROR_CORRUPT,
        ZENIT_ERROR_DOUBLE_FREE,
        ZENIT_ERROR_STATE,
        ZENIT_ERROR_SIZE,
        ZENIT_ERROR_FULL,
        ZENIT_ERROR_EMPTY,
        ZENIT_ERROR_OVERFLOW,
    };
    size_t n = sizeof(codes) / sizeof(codes[0]);

    for (size_t i = 0; i < n; i++) {
        const char* s = zenit_error_string(codes[i]);
        if (s == NULL || strlen(s) == 0) {
            fprintf(stderr, "FAIL: empty string for error code %d\n", (int)codes[i]);
            return 1;
        }
    }

    /* Verify unknown code returns fallback string */
    const char* fallback = zenit_error_string((zenit_error_t)999);
    if (fallback == NULL || strlen(fallback) == 0) {
        fprintf(stderr, "FAIL: empty string for unknown error code\n");
        return 1;
    }

    /* Verify macro helpers compile and produce correct values */
    zenit_result_t ok = ZENIT_RESULT_OK;
    if (ok.error != ZENIT_OK) {
        fprintf(stderr, "FAIL: ZENIT_RESULT_OK has non-zero error\n");
        return 1;
    }

    zenit_result_t err = ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    if (err.error != ZENIT_ERROR_NULL) {
        fprintf(stderr, "FAIL: ZENIT_RESULT_ERROR wrong code\n");
        return 1;
    }

    /* Verify zenit_result_is_ok / zenit_result_is_error */
    if (!zenit_result_is_ok(ok)) {
        fprintf(stderr, "FAIL: zenit_result_is_ok(ZENIT_RESULT_OK) should be true\n");
        return 1;
    }
    if (zenit_result_is_ok(err)) {
        fprintf(stderr, "FAIL: zenit_result_is_ok(error) should be false\n");
        return 1;
    }
    if (!zenit_result_is_error(err)) {
        fprintf(stderr, "FAIL: zenit_result_is_error(ZENIT_RESULT_ERROR) should be true\n");
        return 1;
    }
    if (zenit_result_is_error(ok)) {
        fprintf(stderr, "FAIL: zenit_result_is_error(ZENIT_RESULT_OK) should be false\n");
        return 1;
    }

    /* Verify allocator no-realloc fallback path (success and failure) */
    {
        zenit_allocator_t no_realloc = {
            .alloc_fn = zenit_default_alloc,
            .realloc_fn = NULL,
            .free_fn = zenit_default_free,
            .ctx = NULL
        };
        void *p = no_realloc.alloc_fn(100, NULL);
        if (p == NULL) return 1;
        void *r = zenit_allocator_realloc(no_realloc, p, 100, 200);
        if (r == NULL) {
            fprintf(stderr, "FAIL: allocator_realloc fallback should succeed\n");
            return 1;
        }
        no_realloc.free_fn(r, NULL);

        /* Test fallback with realloc-like semantics (ptr=NULL) */
        void *s = zenit_allocator_realloc(no_realloc, NULL, 0, 64);
        if (s == NULL) {
            fprintf(stderr, "FAIL: realloc(NULL, 64) should succeed\n");
            return 1;
        }
        no_realloc.free_fn(s, NULL);

        /* Test fallback allocation failure */
        {
            zenit_allocator_t fail = {
                .alloc_fn = failing_alloc,
                .realloc_fn = NULL,
                .free_fn = zenit_default_free,
                .ctx = NULL
            };
            void *t = zenit_allocator_realloc(fail, NULL, 0, 100);
            if (t != NULL) {
                fprintf(stderr, "FAIL: realloc with failing alloc should return NULL\n");
                fail.free_fn(t, fail.ctx);
                return 1;
            }
        }
    }

    printf("PASS: result type\n");
    return 0;
}
