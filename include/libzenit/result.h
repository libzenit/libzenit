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

#ifndef LIBZENIT_RESULT_H
#define LIBZENIT_RESULT_H

/**
 * @brief Error codes used across all LibZenit modules.
 *
 * Every public function that can fail returns a zenit_result_t containing
 * one of these codes. ZENIT_OK indicates success; any other value indicates
 * a specific error condition.
 */
typedef enum {
    ZENIT_OK = 0,           /**< Success */
    ZENIT_ERROR_NULL = 1,   /**< NULL pointer passed where non-NULL required */
    ZENIT_ERROR_ALLOC = 2,  /**< Memory allocation failed (malloc/calloc returned NULL) */
    ZENIT_ERROR_PARAM = 3,  /**< Invalid parameter (e.g. zero size where >0 required) */
    ZENIT_ERROR_NOT_FOUND = 4,  /**< No matching entry found (e.g. transition rule) */
    ZENIT_ERROR_CORRUPT = 5,    /**< Data structure is corrupted */
    ZENIT_ERROR_DOUBLE_FREE = 6, /**< Attempt to free an already-free resource */
    ZENIT_ERROR_STATE = 7,      /**< Operation not allowed in the current state */
    ZENIT_ERROR_SIZE = 8,       /**< Size mismatch (e.g. not divisible by block size) */
    ZENIT_ERROR_FULL = 9,       /**< Buffer or container is full */
    ZENIT_ERROR_EMPTY = 10,     /**< Buffer or container is empty */
} zenit_error_t;

/**
 * @brief Result of a mutator operation that can fail.
 *
 * Functions that modify library state return this type. Check the @p error
 * field against ZENIT_OK to determine success.
 *
 * @code
 * zenit_result_t r = zenit_arena_release(arena, ua);
 * if (r.error != ZENIT_OK) {
 *     fprintf(stderr, "release failed: %s\n", zenit_error_string(r.error));
 * }
 * @endcode
 */
typedef struct {
    zenit_error_t error; /**< ZENIT_OK on success, or an error code on failure */
} zenit_result_t;

/** @brief Convenience constant for a successful result. */
#define ZENIT_RESULT_OK ((zenit_result_t){ .error = ZENIT_OK })

/** @brief Build a result with the given error code. */
#define ZENIT_RESULT_ERROR(e) ((zenit_result_t){ .error = (e) })

/**
 * @brief Return a human-readable string describing an error code.
 *
 * The returned string is a static constant and must not be freed.
 *
 * @param error Error code to describe.
 * @return A static string (e.g. "null pointer", "allocation failed").
 */
const char* zenit_error_string(zenit_error_t error);

#endif
