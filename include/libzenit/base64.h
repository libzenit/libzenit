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

#ifndef LIBZENIT_BASE64_H
#define LIBZENIT_BASE64_H

#include <libzenit/allocator.h>
#include <stddef.h>

/**
 * @brief Compute the length of the Base64-encoded output for @p input_len bytes.
 *
 * Includes space for the null terminator.
 *
 * @param input_len Number of raw bytes to encode.
 * @return Number of characters needed in the output buffer (including null).
 */
size_t zenit_base64_encode_len(size_t input_len);

/**
 * @brief Compute the maximum decoded length for a Base64 string.
 *
 * The returned value is an upper bound; the actual decoded length is
 * written to @p out_len by zenit_base64_decode().
 *
 * @param encoded Null-terminated Base64 string.
 * @return Maximum number of bytes the decoded output could occupy.
 */
size_t zenit_base64_decode_len(const char *encoded);

/**
 * @brief Encode raw bytes into a Base64 string (default allocator).
 *
 * Allocates a null-terminated string.  The caller owns the returned
 * pointer and must free it using the default free().
 *
 * @param data Pointer to the raw bytes to encode.
 * @param len  Number of bytes to encode.
 * @return Null-terminated Base64 string, or NULL on allocation failure.
 */
char *zenit_base64_encode(const unsigned char *data, size_t len);

/**
 * @brief Encode raw bytes into a Base64 string (custom allocator).
 *
 * @param data      Pointer to the raw bytes to encode.
 * @param len       Number of bytes to encode.
 * @param allocator Custom allocator for the output string.
 * @return Null-terminated Base64 string, or NULL on allocation failure.
 */
char *zenit_base64_encode_with_allocator(const unsigned char *data, size_t len, zenit_allocator_t allocator);

/**
 * @brief Decode a Base64 string into raw bytes (default allocator).
 *
 * Allocates a buffer.  The caller owns the returned pointer and must
 * free it using the default free().
 *
 * @param encoded Null-terminated Base64 string.
 * @param out_len On success, receives the number of decoded bytes.
 * @return Allocated buffer of decoded bytes, or NULL on allocation failure
 *         or invalid input.
 */
unsigned char *zenit_base64_decode(const char *encoded, size_t *out_len);

/**
 * @brief Decode a Base64 string into raw bytes (custom allocator).
 *
 * @param encoded   Null-terminated Base64 string.
 * @param out_len   On success, receives the number of decoded bytes.
 * @param allocator Custom allocator for the output buffer.
 * @return Allocated buffer of decoded bytes, or NULL on allocation failure
 *         or invalid input.
 */
unsigned char *zenit_base64_decode_with_allocator(const char *encoded, size_t *out_len, zenit_allocator_t allocator);

#endif
