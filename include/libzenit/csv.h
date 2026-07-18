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
//    GNU Affero General Public License more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef LIBZENIT_CSV_H
#define LIBZENIT_CSV_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief A single CSV record (row).
 *
 * Contains an array of field strings.  The fields array and each field
 * string are owned by this struct and freed by zenit_csv_record_destroy().
 */
typedef struct {
    char **fields;  /**< Array of field strings. */
    size_t count;   /**< Number of fields in this record. */
} zenit_csv_record_t;

/**
 * @brief Parse a single CSV record from a string (default allocator).
 *
 * Supports RFC 4180 conventions:
 *   - Comma as delimiter
 *   - Double-quote escaping ("" → ")
 *   - Fields may span multiple lines when quoted
 *   - Leading/trailing whitespace inside quotes is preserved
 *
 * @param line      Null-terminated CSV line.  Must not be NULL.
 * @param delimiter Field delimiter (typically ',').
 * @param out       On success, receives the parsed record.
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_csv_parse_record(const char *line, char delimiter, zenit_csv_record_t *out);

/**
 * @brief Parse a single CSV record from a string (custom allocator).
 *
 * @param line      Null-terminated CSV line.
 * @param delimiter Field delimiter.
 * @param out       On success, receives the parsed record.
 * @param allocator Custom allocator for all allocations.
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_csv_parse_record_with_allocator(const char *line, char delimiter, zenit_csv_record_t *out, zenit_allocator_t allocator);

/**
 * @brief Free a CSV record and all its fields.
 *
 * @param record Record to free.  If NULL, this is a no-op.
 */
void zenit_csv_record_destroy(zenit_csv_record_t *record);

/**
 * @brief Serialise a CSV record to a string (default allocator).
 *
 * Produces a single CSV line with proper quoting and escaping.
 *
 * @param record    Record to serialise.
 * @param delimiter Field delimiter.
 * @param out       On success, receives a pointer to the allocated string (caller frees).
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_csv_serialise_record(const zenit_csv_record_t *record, char delimiter, char **out);

/**
 * @brief Serialise a CSV record to a string (custom allocator).
 *
 * @param record    Record to serialise.
 * @param delimiter Field delimiter.
 * @param out       On success, receives a pointer to the allocated string.
 * @param allocator Custom allocator for the output string.
 * @return ZENIT_RESULT_OK on success, or an error code.
 */
zenit_result_t zenit_csv_serialise_record_with_allocator(const zenit_csv_record_t *record, char delimiter, char **out, zenit_allocator_t allocator);

#endif
