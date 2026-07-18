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

#ifndef LIBZENIT_URI_H
#define LIBZENIT_URI_H

#include <libzenit/allocator.h>
#include <stddef.h>

/**
 * @brief Percent-encode a string for use in a URI (default allocator).
 *
 * Unreserved characters (A-Z, a-z, 0-9, '-', '_', '.', '~') are passed
 * through unchanged.  All other characters are encoded as %XX.
 *
 * @param input Null-terminated string to encode.
 * @return Percent-encoded string, or NULL on allocation failure.
 */
char *zenit_uri_encode(const char *input);

/**
 * @brief Percent-encode a string for use in a URI (custom allocator).
 *
 * @param input     Null-terminated string to encode.
 * @param allocator Custom allocator for the output string.
 * @return Percent-encoded string, or NULL on allocation failure.
 */
char *zenit_uri_encode_with_allocator(const char *input, zenit_allocator_t allocator);

/**
 * @brief Decode a percent-encoded URI string (default allocator).
 *
 * Converts %XX sequences to their byte values and '+' to space.
 *
 * @param encoded Null-terminated percent-encoded string.
 * @return Decoded string, or NULL on allocation failure or invalid input
 *         (e.g. incomplete %XX sequence).
 */
char *zenit_uri_decode(const char *encoded);

/**
 * @brief Decode a percent-encoded URI string (custom allocator).
 *
 * @param encoded   Null-terminated percent-encoded string.
 * @param allocator Custom allocator for the output string.
 * @return Decoded string, or NULL on allocation failure or invalid input
 *         (e.g. incomplete %XX sequence).
 */
char *zenit_uri_decode_with_allocator(const char *encoded, zenit_allocator_t allocator);

#endif
