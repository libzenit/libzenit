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

#include <libzenit/result.h>

/**
 * @brief Return a human-readable string describing an error code.
 *
 * Maps every known zenit_error_t value to a static string. If an unrecognised
 * value is passed (impossible under normal use), returns "unknown error".
 *
 * @param error Error code to describe.
 * @return A static string constant.
 */
const char* zenit_error_string(zenit_error_t error) {
    /* Map each error code to a short, descriptive label */
    switch (error) {
        case ZENIT_OK:           return "ok";
        case ZENIT_ERROR_NULL:   return "null pointer";
        case ZENIT_ERROR_ALLOC:  return "allocation failed";
        case ZENIT_ERROR_PARAM:  return "invalid parameter";
        case ZENIT_ERROR_NOT_FOUND: return "not found";
        case ZENIT_ERROR_CORRUPT:   return "corrupted data";
        case ZENIT_ERROR_DOUBLE_FREE: return "double free";
        case ZENIT_ERROR_STATE:  return "invalid state";
        case ZENIT_ERROR_SIZE:   return "size mismatch";
        case ZENIT_ERROR_FULL:   return "buffer full";
        case ZENIT_ERROR_EMPTY:  return "buffer empty";
        case ZENIT_ERROR_OVERFLOW: return "overflow";
    }
    /* Fallback — defensive; should never reach here unless a new error code
     * was added to the enum without a corresponding case. */
    return "unknown error";
}
