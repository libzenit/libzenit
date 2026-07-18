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

#ifndef LIBZENIT_HEX_H
#define LIBZENIT_HEX_H

#include <libzenit/allocator.h>
#include <stddef.h>

/**
 * @brief Compute the length of the hex-encoded output for @p data_len bytes.
 *
 * Each byte becomes two hex characters plus a null terminator.
 *
 * @param data_len Number of raw bytes to encode.
 * @return Number of characters needed in the output buffer (including null).
 */
size_t zenit_hex_encode_len(size_t data_len);

/**
 * @brief Compute the decoded length of a hex string.
 *
 * Returns (strlen(hex) / 2).  If the string has an odd length it is
 * treated as if a leading '0' were present.
 *
 * @param hex Null-terminated hex string.
 * @return Number of bytes the decoded output will occupy.
 */
size_t zenit_hex_decode_len(const char *hex);

/**
 * @brief Encode raw bytes into a lowercase hex string (default allocator).
 *
 * @param data Pointer to the raw bytes to encode.
 * @param len  Number of bytes to encode.
 * @return Null-terminated hex string (lowercase), or NULL on allocation failure.
 */
char *zenit_hex_encode(const unsigned char *data, size_t len);

/**
 * @brief Encode raw bytes into a lowercase hex string (custom allocator).
 *
 * @param data      Pointer to the raw bytes to encode.
 * @param len       Number of bytes to encode.
 * @param allocator Custom allocator for the output string.
 * @return Null-terminated hex string (lowercase), or NULL on allocation failure.
 */
char *zenit_hex_encode_with_allocator(const unsigned char *data, size_t len, zenit_allocator_t allocator);

/**
 * @brief Decode a hex string into raw bytes (default allocator).
 *
 * Accepts both uppercase and lowercase hex digits.  If the input has
 * an odd length it is treated as if a leading '0' were present.
 *
 * @param hex     Null-terminated hex string.
 * @param out_len On success, receives the number of decoded bytes.
 * @return Allocated buffer of decoded bytes, or NULL on allocation failure
 *         or invalid input (non-hex character).
 */
unsigned char *zenit_hex_decode(const char *hex, size_t *out_len);

/**
 * @brief Decode a hex string into raw bytes (custom allocator).
 *
 * @param hex       Null-terminated hex string.
 * @param out_len   On success, receives the number of decoded bytes.
 * @param allocator Custom allocator for the output buffer.
 * @return Allocated buffer of decoded bytes, or NULL on allocation failure
 *         or invalid input (non-hex character).
 */
unsigned char *zenit_hex_decode_with_allocator(const char *hex, size_t *out_len, zenit_allocator_t allocator);

#endif
