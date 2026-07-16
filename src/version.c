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

#include <libzenit/version.h>

/**
 * @brief Return the compiled-in library version.
 *
 * Uses C99 designated initialisers to build the version struct so every
 * field is explicitly set — no risk of uninitialised padding or fields.
 *
 * @return libzenit_version_t populated with the current release numbers.
 */
libzenit_version_t libzenit_version(void) {
    /* Designated-initialiser list — fields are clear even if the struct layout
     * changes, and the compiler warns on unknown fields */
    libzenit_version_t v = {
        .major = 0,
        .minor = 1,
        .patch = 0,
        .name = "alpha"
    };
    /* Return by value (small struct, easily elided by the ABI / inlining) */
    return v;
}
