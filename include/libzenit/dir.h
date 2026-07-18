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

#ifndef LIBZENIT_DIR_H
#define LIBZENIT_DIR_H

#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque directory iterator.
 *
 * Created by zenit_dir_iter() and advanced with zenit_dir_next().
 * Must be destroyed with zenit_dir_iter_destroy().
 */
typedef struct zenit_dir_iter_t zenit_dir_iter_t;

/**
 * @brief A single directory entry.
 *
 * The @p name field is an inline fixed-size buffer owned by this struct.
 * Entry data is valid only until the next call to zenit_dir_next() on
 * the same iterator.
 */
typedef struct {
    char name[256];      /**< Entry name (null-terminated). */
    int is_directory;    /**< Non-zero if the entry is a subdirectory. */
} zenit_dir_entry_t;

/**
 * @brief Create a directory (like mkdir -p).
 *
 * Creates the directory at @p path, including any intermediate parent
 * directories that do not yet exist.
 *
 * @param path Null-terminated directory path.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path is NULL
 *         - ZENIT_ERROR_NOT_FOUND if a component cannot be created.
 */
zenit_result_t zenit_dir_create(const char *path);

/**
 * @brief Remove an empty directory.
 *
 * The directory must be empty.  Use @p path to identify it.
 *
 * @param path Null-terminated directory path.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the directory does not exist or
 *           cannot be removed.
 */
zenit_result_t zenit_dir_remove(const char *path);

/**
 * @brief Check whether a directory exists.
 *
 * @param path Null-terminated directory path.
 * @return 1 if the directory exists, 0 if it does not or @p path is NULL.
 */
int zenit_dir_exists(const char *path);

/**
 * @brief Open a directory for iteration.
 *
 * Returns an opaque iterator over the entries in @p path, including
 * "." and "..".  The iterator must be destroyed with
 * zenit_dir_iter_destroy() when no longer needed.
 *
 * @param path Null-terminated directory path.
 * @return A pointer to a new iterator, or NULL on failure (directory
 *         not found, allocation failure).
 */
zenit_dir_iter_t* zenit_dir_iter(const char *path);

/**
 * @brief Advance the iterator to the next entry.
 *
 * Fills @p out_entry with the name and type of the next directory entry.
 *
 * @param iter     Iterator returned by zenit_dir_iter().  Must not be NULL.
 * @param out_entry On success, receives the entry data.
 * @return 1 if an entry was read, 0 at the end of the directory or on
 *         error (NULL parameters).
 */
int zenit_dir_next(zenit_dir_iter_t *iter, zenit_dir_entry_t *out_entry);

/**
 * @brief Destroy a directory iterator.
 *
 * Closes the underlying directory handle and frees the iterator.
 *
 * @param iter Iterator to destroy.  If NULL, this is a no-op.
 */
void zenit_dir_iter_destroy(zenit_dir_iter_t *iter);

/**
 * @brief List all entries in a directory.
 *
 * Convenience function that collects every entry name into a dynamically
 * allocated array of strings.  The caller owns both the array and each
 * string, and must free them with free().
 *
 * @param path      Null-terminated directory path.
 * @param out_names On success, receives a pointer to an array of
 *                  null-terminated strings (one per entry).  May be
 *                  NULL if the directory is empty.
 * @param out_count On success, receives the number of entries.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p path, @p out_names, or @p out_count
 *           is NULL.
 *         - ZENIT_ERROR_NOT_FOUND if the directory cannot be opened.
 *         - ZENIT_ERROR_ALLOC if memory allocation fails.
 */
zenit_result_t zenit_dir_list(const char *path, char ***out_names, size_t *out_count);

#endif
