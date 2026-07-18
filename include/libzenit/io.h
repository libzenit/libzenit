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

#ifndef LIBZENIT_IO_H
#define LIBZENIT_IO_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Read the entire contents of a file into memory (default allocator).
 *
 * Opens @p path in binary mode, determines its size, allocates a buffer,
 * reads the full content, and returns it.  The caller owns the buffer and
 * must free it.
 *
 * @param path    Null-terminated file path.
 * @param out_data On success, receives a pointer to the allocated buffer.
 * @param out_len  On success, receives the number of bytes read.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path, @p out_data, or @p out_len is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the file cannot be opened
 *         - ZENIT_ERROR_ALLOC if memory allocation fails.
 */
zenit_result_t zenit_file_read(const char *path, void **out_data, size_t *out_len);

/**
 * @brief Read the entire contents of a file into memory (custom allocator).
 *
 * @param path      Null-terminated file path.
 * @param out_data  On success, receives a pointer to the allocated buffer.
 * @param out_len   On success, receives the number of bytes read.
 * @param allocator Custom allocator for the output buffer.
 * @return ZENIT_RESULT_OK on success, or an error code on failure.
 */
zenit_result_t zenit_file_read_with_allocator(const char *path, void **out_data, size_t *out_len, zenit_allocator_t allocator);

/**
 * @brief Write data to a file, creating or truncating it.
 *
 * Opens @p path in binary write mode.  If the file exists it is truncated.
 * If the file does not exist it is created.
 *
 * @param path Null-terminated file path.
 * @param data Pointer to the data to write.
 * @param len  Number of bytes to write.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path or @p data is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the file cannot be opened (permissions, etc.)
 *         - ZENIT_ERROR_SIZE if not all bytes were written.
 */
zenit_result_t zenit_file_write(const char *path, const void *data, size_t len);

/**
 * @brief Append data to a file, creating it if needed.
 *
 * Opens @p path in binary append mode.  If the file does not exist it is
 * created.
 *
 * @param path Null-terminated file path.
 * @param data Pointer to the data to append.
 * @param len  Number of bytes to append.
 * @return ZENIT_RESULT_OK on success, or an error code on failure.
 */
zenit_result_t zenit_file_append(const char *path, const void *data, size_t len);

/**
 * @brief Check whether a file exists.
 *
 * @param path Null-terminated file path.
 * @return 1 if the file exists, 0 if it does not or @p path is NULL.
 */
int zenit_file_exists(const char *path);

/**
 * @brief Delete a file.
 *
 * @param path Null-terminated file path.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the file does not exist or cannot be deleted.
 */
zenit_result_t zenit_file_delete(const char *path);

/**
 * @brief Get the size of a file in bytes.
 *
 * @param path    Null-terminated file path.
 * @param out_size On success, receives the file size in bytes.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path or @p out_size is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the file cannot be accessed.
 */
zenit_result_t zenit_file_size(const char *path, size_t *out_size);

/**
 * @brief Read the first line from a text file into an allocated buffer (default allocator).
 *
 * Reads bytes from @p path until a newline ('\\n') is found or EOF is reached.
 * The newline character is included if present.  The buffer is null-terminated.
 *
 * @param path    Null-terminated file path.
 * @param out_line On success, receives a pointer to the allocated line (including newline).
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path or @p out_line is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the file cannot be opened
 *         - ZENIT_ERROR_ALLOC if memory allocation fails.
 */
zenit_result_t zenit_file_read_line(const char *path, char **out_line);

/**
 * @brief Read the first line from a text file with a custom allocator.
 *
 * @param path      Null-terminated file path.
 * @param out_line  On success, receives a pointer to the allocated line.
 * @param allocator Custom allocator for the output buffer.
 * @return ZENIT_RESULT_OK on success, or an error code on failure.
 */
zenit_result_t zenit_file_read_line_with_allocator(const char *path, char **out_line, zenit_allocator_t allocator);

/**
 * @brief Copy a file from source to destination.
 *
 * Opens @p src for reading and @p dst for writing (create/truncate).
 * Reads and writes in chunks to handle large files without loading
 * the entire content into memory.
 *
 * @param src Path of the source file.
 * @param dst Path of the destination file.
 * @return ZENIT_RESULT_OK on success, or an error code on failure.
 */
zenit_result_t zenit_file_copy(const char *src, const char *dst);

#endif
