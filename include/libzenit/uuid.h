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

#ifndef LIBZENIT_UUID_H
#define LIBZENIT_UUID_H

#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief RFC 4122 UUID (16 bytes).
 */
typedef struct {
    uint8_t bytes[16]; /**< Raw 128-bit UUID value. */
} zenit_uuid_t;

/**
 * @brief UUID string length including NUL terminator (36 chars + NUL).
 */
#define ZENIT_UUID_STR_LEN 37

/**
 * @brief Generate a random UUID v4.
 *
 * Uses a cryptographically secure random source:
 *   - Linux:   getrandom() or /dev/urandom
 *   - macOS:   arc4random_buf()
 *   - Windows: BCryptGenRandom()
 *
 * The UUID version (4) and variant (10xx) bits are set after generation.
 *
 * @param out On success, receives a 16-byte UUID.
 * @return ZENIT_RESULT_OK on success, or ZENIT_ERROR_ALLOC on failure.
 */
zenit_result_t zenit_uuid_generate(zenit_uuid_t *out);

/**
 * @brief Format a UUID as a 36-char string (e.g. "550e8400-e29b-41d4-a716-446655440000").
 *
 * @param uuid  UUID to format. Must not be NULL.
 * @param out   Output buffer, must be at least ZENIT_UUID_STR_LEN bytes.
 *              If NULL, returns ZENIT_ERROR_NULL.
 * @return ZENIT_RESULT_OK on success, or ZENIT_ERROR_NULL if @p uuid or @p out is NULL.
 */
zenit_result_t zenit_uuid_format(const zenit_uuid_t *uuid, char *out);

/**
 * @brief Parse a 36-char UUID string into a zenit_uuid_t.
 *
 * Accepts both lowercase and uppercase hex digits.
 *
 * @param str  Null-terminated 36-char UUID string (e.g. "550e8400-e29b-41d4-a716-446655440000").
 *             Must not be NULL.
 * @param out  On success, receives the parsed UUID.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_NULL if @p str or @p out is NULL,
 *         or ZENIT_ERROR_PARAM if the string is malformed.
 */
zenit_result_t zenit_uuid_parse(const char *str, zenit_uuid_t *out);

/**
 * @brief Compare two UUIDs for equality.
 *
 * @param a First UUID. NULL returns 0.
 * @param b Second UUID. NULL returns 0.
 * @return 1 if equal, 0 otherwise.
 */
int zenit_uuid_equal(const zenit_uuid_t *a, const zenit_uuid_t *b);

#endif
