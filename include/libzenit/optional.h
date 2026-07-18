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

#ifndef LIBZENIT_OPTIONAL_H
#define LIBZENIT_OPTIONAL_H

#include <stddef.h>
#include <string.h>

/**
 * @brief Generic optional value container.
 *
 * Manages an optional value of any type.  Stores a has_value flag alongside
 * the value.  The value is stored inline (no heap allocation).
 *
 * Usage:
 * @code
 * ZENIT_OPTIONAL(int) opt = zenit_optional_none();
 * zenit_optional_set(&opt, 42);
 * if (zenit_optional_has(&opt)) {
 *     int v = zenit_optional_get(&opt);
 * }
 * @endcode
 */
#define ZENIT_OPTIONAL(type) \
    struct { \
        int has_value; \
        type value; \
    }

/**
 * @brief Create an empty (none) optional value.
 *
 * Use this as an initializer:
 * @code ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_NONE; @endcode
 */
#define ZENIT_OPTIONAL_NONE \
    { .has_value = 0 }

/**
 * @brief Create an optional containing a value.
 *
 * Use this as an initializer:
 * @code ZENIT_OPTIONAL(int) opt = ZENIT_OPTIONAL_SOME(42); @endcode
 */
#define ZENIT_OPTIONAL_SOME(v) \
    { .has_value = 1, .value = (v) }

/**
 * @brief Check if an optional contains a value.
 *
 * @param opt Pointer to the optional.
 * @return 1 if the optional has a value, 0 otherwise.
 */
#define zenit_optional_has(opt) \
    ((opt)->has_value)

/**
 * @brief Get the value from an optional.
 *
 * Behaviour is undefined if the optional is empty.  Check zenit_optional_has()
 * first.
 *
 * @param opt Pointer to the optional.
 * @return The stored value.
 */
#define zenit_optional_get(opt) \
    ((opt)->value)

/**
 * @brief Set the value of an optional, overwriting any existing value.
 *
 * @param opt Pointer to the optional.
 * @param v   New value to store.
 */
#define zenit_optional_set(opt, v) \
    do { \
        (opt)->has_value = 1; \
        (opt)->value = (v); \
    } while (0)

/**
 * @brief Clear (reset) an optional to the empty state.
 *
 * @param opt Pointer to the optional.
 */
#define zenit_optional_clear(opt) \
    do { \
        (opt)->has_value = 0; \
    } while (0)

/**
 * @brief Get the value from an optional, or a default if empty.
 *
 * @param opt    Pointer to the optional.
 * @param default_val Value to return if the optional is empty.
 * @return The stored value if has_value is true, otherwise @p default_val.
 */
#define zenit_optional_get_or(opt, default_val) \
    ((opt)->has_value ? (opt)->value : (default_val))

/**
 * @brief Copy the value from an optional into a destination.
 *
 * Copies the value by assignment if has_value is true.  Does nothing if
 * the optional is empty.
 *
 * @param opt Pointer to the optional.
 * @param dst Pointer to the destination (same type).
 */
#define zenit_optional_copy(opt, dst) \
    do { \
        if ((opt)->has_value) { \
            *(dst) = (opt)->value; \
        } \
    } while (0)

/**
 * @brief Map a function over an optional's value.
 *
 * If the optional has a value, applies the function and stores the result
 * in @p out.  If empty, clears @p out.
 *
 * @param opt Pointer to the optional.
 * @param fn  Function to apply (takes value type, returns result type).
 * @param out Pointer to the output optional.
 */
#define zenit_optional_map(opt, fn, out) \
    do { \
        if ((opt)->has_value) { \
            (out)->has_value = 1; \
            (out)->value = (fn((opt)->value)); \
        } else { \
            (out)->has_value = 0; \
        } \
    } while (0)

#endif
