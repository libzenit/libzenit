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

#ifndef LIBZENIT_SEMVER_H
#define LIBZENIT_SEMVER_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Maximum length of a pre-release or build metadata string (including NUL).
 */
#define ZENIT_SEMVER_MAX_PRERELEASE 64
#define ZENIT_SEMVER_MAX_BUILD 64

/**
 * @brief Parsed semantic version (SemVer 2.0.0).
 *
 * Fields are zeroed on parse failure.  The pre_release and build strings
 * point into the internal buffer and must not be freed separately.
 */
typedef struct {
    int major;                      /**< Major version */
    int minor;                      /**< Minor version */
    int patch;                      /**< Patch version */
    char pre_release[ZENIT_SEMVER_MAX_PRERELEASE]; /**< Pre-release (e.g. "alpha.1") */
    char build[ZENIT_SEMVER_MAX_BUILD];            /**< Build metadata (e.g. "20260101") */
} zenit_semver_t;

/**
 * @brief Parse a SemVer 2.0.0 string.
 *
 * Accepts formats like "1.2.3", "1.2.3-alpha.1", "1.2.3+001",
 * "1.2.3-alpha.1+build.2".
 *
 * @param str  Null-terminated version string.
 * @param out  On success, receives the parsed version.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_NULL if @p str or @p out is NULL,
 *         or ZENIT_ERROR_PARAM if the string is malformed.
 */
zenit_result_t zenit_semver_parse(const char *str, zenit_semver_t *out);

/**
 * @brief Format a semantic version as a string.
 *
 * Formats as "MAJOR.MINOR.PATCH" with optional pre-release and/or build.
 * Uses the default allocator.
 *
 * @param v     Version to format. Must not be NULL.
 * @param out   On success, receives a pointer to the allocated string (caller frees).
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_semver_format(const zenit_semver_t *v, char **out);

/**
 * @brief Format a semantic version as a string (custom allocator).
 *
 * @param v         Version to format.
 * @param out       On success, receives a pointer to the allocated string.
 * @param allocator Custom allocator for the output string.
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_semver_format_with_allocator(const zenit_semver_t *v, char **out, zenit_allocator_t allocator);

/**
 * @brief Compare two semantic versions.
 *
 * Uses SemVer 2.0.0 precedence rules:
 *   1. Compare major/minor/patch numerically.
 *   2. If equal, compare pre-release fields (dot-separated identifiers,
 *      numeric vs string ordering, shorter has lower precedence).
 *   3. Build metadata is ignored for precedence.
 *
 * @param a First version. NULL is treated as less than any non-NULL.
 * @param b Second version.
 * @return Negative if a < b, zero if equal, positive if a > b.
 */
int zenit_semver_compare(const zenit_semver_t *a, const zenit_semver_t *b);

#endif
