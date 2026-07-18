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

#ifndef LIBZENIT_JSON_H
#define LIBZENIT_JSON_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief JSON value type identifiers.
 *
 * Each #zenit_json_value_t carries one of these tags.  Use
 * zenit_json_value_type() to query the tag and the corresponding
 * getter to read the payload.
 */
typedef enum {
    ZENIT_JSON_NULL = 0,    /**< JSON null literal */
    ZENIT_JSON_BOOL,        /**< JSON boolean (true / false) */
    ZENIT_JSON_NUMBER,      /**< JSON number (stored as double) */
    ZENIT_JSON_STRING,      /**< JSON string (null-terminated) */
    ZENIT_JSON_ARRAY,       /**< JSON array (ordered list of values) */
    ZENIT_JSON_OBJECT       /**< JSON object (string-keyed map of values) */
} zenit_json_type_t;

/** @brief Opaque handle for a JSON document.  Owns all values. */
typedef struct zenit_json_t zenit_json_t;

/** @brief A single JSON value within a document.  Stable pointer. */
typedef struct zenit_json_value_t zenit_json_value_t;

/**
 * @brief Parse a null-terminated JSON string.
 *
 * @param input Null-terminated UTF-8 text.  Must not be NULL.
 * @return New document handle, or NULL on parse error or allocation failure.
 */
zenit_json_t *zenit_json_parse(const char *input);

/**
 * @brief Parse a JSON string with explicit length.
 *
 * @param input  UTF-8 text (does not need to be null-terminated).
 * @param length Number of bytes in @p input.
 * @return New document handle, or NULL on parse error or allocation failure.
 */
zenit_json_t *zenit_json_parse_with_length(const char *input, size_t length);

/**
 * @brief Parse with a custom allocator.
 *
 * @param input     Null-terminated UTF-8 text.
 * @param allocator Custom allocator for all internal memory.
 * @return New document handle, or NULL on parse error or allocation failure.
 */
zenit_json_t *zenit_json_parse_with_allocator(const char *input, zenit_allocator_t allocator);

/**
 * @brief Parse with explicit length and a custom allocator.
 *
 * @param input     UTF-8 text (does not need to be null-terminated).
 * @param length    Number of bytes in @p input.
 * @param allocator Custom allocator for all internal memory.
 * @return New document handle, or NULL on parse error or allocation failure.
 */
zenit_json_t *zenit_json_parse_with_length_and_allocator(const char *input, size_t length, zenit_allocator_t allocator);

/**
 * @brief Create an empty document (no root value).
 *
 * @return New document handle, or NULL on allocation failure.
 */
zenit_json_t *zenit_json_create(void);

/**
 * @brief Create an empty document with a custom allocator.
 *
 * @param allocator Custom allocator for all internal memory.
 * @return New document handle, or NULL on allocation failure.
 */
zenit_json_t *zenit_json_create_with_allocator(zenit_allocator_t allocator);

/**
 * @brief Free all memory owned by a JSON document.
 *
 * All value pointers obtained from this document become invalid.
 * Passing NULL is safe and is a no-op.
 *
 * @param json Document handle, or NULL.
 */
void zenit_json_destroy(zenit_json_t *json);

/**
 * @brief Return the root value of a document.
 *
 * @param json Document handle.
 * @return Pointer to the root value, or NULL if the document has no root.
 */
zenit_json_value_t *zenit_json_root(const zenit_json_t *json);

/**
 * @brief Set or replace the root value of a document.
 *
 * The value must belong to this document (created via one of the
 * zenit_json_value_* functions or obtained via parsing).
 *
 * @param json Document handle.
 * @param val  Value to become the new root.  May be NULL to clear the root.
 * @return ZENIT_RESULT_OK on success, or ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL)
 *         if @p json is NULL.
 */
zenit_result_t zenit_json_set_root(zenit_json_t *json, zenit_json_value_t *val);

/**
 * @brief Query the type tag of a value.
 *
 * @param val Value handle.
 * @return Type tag, or ZENIT_JSON_NULL if @p val is NULL.
 */
zenit_json_type_t zenit_json_value_type(const zenit_json_value_t *val);

/**
 * @brief Check whether a value is JSON null.
 *
 * @param val Value handle.
 * @return 1 if the value is JSON null or @p val is NULL, 0 otherwise.
 */
int zenit_json_value_is_null(const zenit_json_value_t *val);

/**
 * @brief Read a boolean value.
 *
 * @param val Value handle.
 * @return 0 if the value is not a boolean or @p val is NULL, otherwise
 *         the boolean payload (0 or 1).
 */
int zenit_json_value_get_bool(const zenit_json_value_t *val);

/**
 * @brief Read a number value.
 *
 * @param val Value handle.
 * @return The number payload, or 0.0 if @p val is NULL or the value
 *         is not a number.
 */
double zenit_json_value_get_number(const zenit_json_value_t *val);

/**
 * @brief Read a string value.
 *
 * The returned pointer is valid until the document is destroyed.
 *
 * @param val Value handle.
 * @return Null-terminated string, or NULL if @p val is NULL or the value
 *         is not a string.
 */
const char *zenit_json_value_get_string(const zenit_json_value_t *val);

/**
 * @name Value constructors
 *
 * All constructors allocate a new value linked to @p json and return
 * a stable pointer, or NULL on allocation failure.
 * @{
 */

zenit_json_value_t *zenit_json_value_null(zenit_json_t *json);
zenit_json_value_t *zenit_json_value_bool(zenit_json_t *json, int val);
zenit_json_value_t *zenit_json_value_number(zenit_json_t *json, double val);
zenit_json_value_t *zenit_json_value_string(zenit_json_t *json, const char *val);
zenit_json_value_t *zenit_json_value_array(zenit_json_t *json);
zenit_json_value_t *zenit_json_value_object(zenit_json_t *json);

/** @} */

/**
 * @name Array operations
 *
 * Operate on values of type ZENIT_JSON_ARRAY.  Passing a non-array value
 * or NULL safely returns 0 / NULL / ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM).
 * @{
 */

size_t zenit_json_array_count(const zenit_json_value_t *val);
zenit_json_value_t *zenit_json_array_get(const zenit_json_value_t *val, size_t index);
zenit_result_t zenit_json_array_append(zenit_json_value_t *arr, zenit_json_value_t *item);
zenit_result_t zenit_json_array_remove(zenit_json_value_t *arr, size_t index);
zenit_result_t zenit_json_array_insert(zenit_json_value_t *arr, size_t index, zenit_json_value_t *item);

/** @} */

/**
 * @name Object operations
 *
 * Operate on values of type ZENIT_JSON_OBJECT.  Passing a non-object value
 * or NULL safely returns 0 / NULL / ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM).
 * @{
 */

size_t zenit_json_object_count(const zenit_json_value_t *val);
const char *zenit_json_object_key(const zenit_json_value_t *val, size_t index);
zenit_json_value_t *zenit_json_object_value_at(const zenit_json_value_t *val, size_t index);
zenit_json_value_t *zenit_json_object_get(const zenit_json_value_t *val, const char *key);
zenit_result_t zenit_json_object_set(zenit_json_value_t *obj, const char *key, zenit_json_value_t *val);
zenit_result_t zenit_json_object_remove(zenit_json_value_t *obj, const char *key);

/** @} */

/**
 * @brief Serialise a document to a null-terminated JSON string.
 *
 * The caller owns the returned string and must free it using the
 * allocator's free_fn.
 *
 * @param json Document to serialise.
 * @return Allocated string, or NULL on allocation failure or if @p json
 *         has no root value.
 */
char *zenit_json_serialize(const zenit_json_t *json);

/**
 * @brief Serialise a document with a custom allocator.
 *
 * @param json      Document to serialise.
 * @param allocator Custom allocator for the output string.
 * @return Allocated string, or NULL on allocation failure.
 */
char *zenit_json_serialize_with_allocator(const zenit_json_t *json, zenit_allocator_t allocator);

/**
 * @brief Serialise a value subtree to a JSON string.
 *
 * The caller owns the returned string.
 *
 * @param val Value to serialise.
 * @return Allocated string, or NULL on allocation failure.
 */
char *zenit_json_value_serialize(const zenit_json_value_t *val);

/**
 * @brief Serialise a value subtree with a custom allocator.
 *
 * @param val       Value to serialise.
 * @param allocator Custom allocator for the output string.
 * @return Allocated string, or NULL on allocation failure.
 */
char *zenit_json_value_serialize_with_allocator(const zenit_json_value_t *val, zenit_allocator_t allocator);

#endif
