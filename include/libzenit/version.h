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

#ifndef LIBZENIT_VERSION_H
#define LIBZENIT_VERSION_H

/**
 * @brief Version descriptor for the libzenit library.
 */
typedef struct {
    int major;      /**< Major version — incremented on breaking changes */
    int minor;      /**< Minor version — incremented on feature additions */
    int patch;      /**< Patch version — incremented on bug fixes */
    const char* name; /**< Pre-release / codename label (e.g. "alpha") */
} libzenit_version_t;

/**
 * @brief Retrieve the compiled-in library version.
 *
 * @return A fully populated libzenit_version_t describing the current release.
 */
libzenit_version_t libzenit_version(void);

#endif
