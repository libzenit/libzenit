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

#include <stddef.h>

/**
 * @brief Percent-encode a string for use in a URI.
 *
 * Unreserved characters (A-Z, a-z, 0-9, '-', '_', '.', '~') are passed
 * through unchanged.  All other characters are encoded as %XX.
 *
 * Allocates a null-terminated string via malloc().  The caller owns
 * the returned pointer and must free() it.
 *
 * @param input Null-terminated string to encode.
 * @return Percent-encoded string, or NULL on allocation failure.
 */
char *zenit_uri_encode(const char *input);

/**
 * @brief Decode a percent-encoded URI string.
 *
 * Converts %XX sequences to their byte values and '+' to space.
 *
 * Allocates a null-terminated string via malloc().  The caller owns
 * the returned pointer and must free() it.
 *
 * @param encoded Null-terminated percent-encoded string.
 * @return Decoded string, or NULL on allocation failure or invalid input
 *         (e.g. incomplete %XX sequence).
 */
char *zenit_uri_decode(const char *encoded);

#endif
