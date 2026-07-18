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

#ifndef LIBZENIT_TRIE_H
#define LIBZENIT_TRIE_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque handle for a prefix tree (trie).
 *
 * Stores key-value pairs where keys are lowercase a-z strings.  Only
 * alphabetic characters are indexed; digits, punctuation, and whitespace
 * are silently skipped during insert, search, and delete operations.
 */
typedef struct zenit_trie_t zenit_trie_t;

/**
 * @brief Create an empty trie with the default allocator.
 *
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_trie_t* zenit_trie_create(void);

/**
 * @brief Create an empty trie with a custom allocator.
 *
 * @param allocator Custom allocator to use for all memory operations.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_trie_t* zenit_trie_create_with_allocator(zenit_allocator_t allocator);

/**
 * @brief Destroy a trie and free all owned memory.
 *
 * Passing NULL is safe and is a no-op.
 *
 * @param trie Handle returned by zenit_trie_create(), or NULL.
 */
void zenit_trie_destroy(zenit_trie_t* trie);

/**
 * @brief Insert or update a key-value pair in the trie.
 *
 * If the key already exists its value is overwritten.
 * Non-alphabetic characters in the key are silently skipped.
 *
 * @param trie  Trie handle.
 * @param key   Null-terminated string (case is folded to lower).
 * @param value Value to associate with the key.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p trie or @p key is NULL
 *         - ZENIT_ERROR_ALLOC if a node allocation fails.
 */
zenit_result_t zenit_trie_insert(zenit_trie_t* trie, const char* key, int value);

/**
 * @brief Search for a key in the trie.
 *
 * @param trie     Trie handle.
 * @param key      Null-terminated string to search for.
 * @param out_value Optional pointer to receive the stored value (may be NULL).
 * @return 1 if the key exists, 0 if not (or if @p trie or @p key is NULL).
 */
int zenit_trie_search(const zenit_trie_t* trie, const char* key, int* out_value);

/**
 * @brief Check whether any key in the trie starts with the given prefix.
 *
 * @param trie   Trie handle.
 * @param prefix Null-terminated prefix string.
 * @return 1 if at least one stored key starts with @p prefix, 0 otherwise
 *         (or if @p trie or @p prefix is NULL).
 */
int zenit_trie_starts_with(const zenit_trie_t* trie, const char* prefix);

/**
 * @brief Delete a key from the trie.
 *
 * @param trie Trie handle.
 * @param key  Null-terminated string to delete.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p trie or @p key is NULL
 *         - ZENIT_ERROR_NOT_FOUND if the key does not exist.
 */
zenit_result_t zenit_trie_delete(zenit_trie_t* trie, const char* key);

/**
 * @brief Return the number of keys stored in the trie.
 *
 * @param trie Trie handle.
 * @return Number of keys (0 if @p trie is NULL).
 */
size_t zenit_trie_count(const zenit_trie_t* trie);

/**
 * @brief Remove all keys from the trie without destroying it.
 *
 * After this call the trie is empty and can be reused.
 * Passing NULL is safe and is a no-op.
 *
 * @param trie Trie handle, or NULL.
 */
void zenit_trie_clear(zenit_trie_t* trie);

#endif
