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

#ifndef LIBZENIT_ARENA_H
#define LIBZENIT_ARENA_H

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a fixed-block memory arena.
 *
 * An arena manages a single large allocation divided into fixed-size blocks.
 * Blocks are tracked via an internal bitmap, forming a two-state machine
 * (FREE ↔ ALLOCATED) per block to prevent double-use or double-release.
 */
typedef struct zenit_arena_t zenit_arena_t;

/**
 * @brief Opaque handle for a contiguous sub-region acquired from an arena.
 *
 * A usable arena holds one or more contiguous arena blocks and provides
 * sub-allocation via an explicit free-list. Each sub-allocation carries
 * a header with its own two-state machine (FREE ↔ IN_USE) for safety.
 */
typedef struct zenit_usable_arena_t zenit_usable_arena_t;

/**
 * @brief A sub-allocated buffer within a usable arena.
 *
 * Returned by zenit_usable_arena_allocate(). The @p data pointer is valid
 * until the buffer is freed with zenit_usable_buffer_free().
 */
typedef struct {
    void *data;  /**< Pointer to the buffer payload (NULL on allocation failure). */
    size_t size; /**< Usable size requested by the caller. */
} zenit_usable_buffer_t;

/**
 * @brief Create a fixed-block arena.
 *
 * Allocates @p total_size bytes of raw memory and divides it into
 * @p total_size / @p block_size blocks, each tracked by a bitmap.
 * Both @p total_size and @p block_size must be > 0, and
 * @p total_size must be evenly divisible by @p block_size.
 *
 * @param total_size Total memory to manage (must be > 0).
 * @param block_size Size of each block (must be > 0).
 * @return Arena handle, or NULL on invalid parameters or allocation failure.
 */
zenit_arena_t *zenit_arena_create(size_t total_size, size_t block_size);

/**
 * @brief Create a fixed-block arena with a custom allocator.
 *
 * @param total_size Total memory to manage (must be > 0).
 * @param block_size Size of each block (must be > 0).
 * @param allocator  Custom allocator for the arena's metadata.
 * @return Arena handle, or NULL on invalid parameters or allocation failure.
 */
zenit_arena_t *zenit_arena_create_with_allocator(size_t total_size, size_t block_size, zenit_allocator_t allocator);

/**
 * @brief Destroy an arena, freeing all memory.
 *
 * Calling this while usable arenas are still alive is undefined behaviour.
 * Passing NULL is safe and is a no-op.
 *
 * @param arena Arena handle, or NULL.
 */
void zenit_arena_destroy(zenit_arena_t *arena);

/**
 * @brief Acquire a contiguous region from the arena.
 *
 * Finds @p size / @p block_size consecutive FREE blocks in the arena's bitmap,
 * transitions them to ALLOCATED, and returns a usable arena handle.
 *
 * @p size must be > 0 and evenly divisible by the arena's @p block_size.
 *
 * @param arena Arena handle.
 * @param size  Requested region size (must be a multiple of block_size).
 * @return Usable arena handle, or NULL if insufficient contiguous space
 *         or invalid parameters.
 */
zenit_usable_arena_t *zenit_arena_acquire(zenit_arena_t *arena, size_t size);

/**
 * @brief Release a usable arena back to the parent arena.
 *
 * Verifies that all sub-allocated buffers inside @p ua have been freed
 * (i.e., none are IN_USE), then transitions the arena's blocks back to
 * FREE and frees the usable arena handle.
 *
 * @param arena Parent arena handle.
 * @param ua    Usable arena handle acquired from @p arena.
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p arena or @p ua is NULL
 *         - ZENIT_ERROR_STATE if any buffer is still IN_USE
 *         - ZENIT_ERROR_CORRUPT if a corrupted block is detected.
 */
zenit_result_t zenit_arena_release(zenit_arena_t *arena, zenit_usable_arena_t *ua);

/**
 * @brief Sub-allocate a buffer from a usable arena.
 *
 * Walks the explicit free-list looking for a block large enough to satisfy
 * @p size bytes. On success the block is split if necessary, its state
 * transitions from FREE to IN_USE, and a buffer descriptor is returned.
 *
 * @param ua   Usable arena handle.
 * @param size Requested buffer size (must be > 0).
 * @return Buffer descriptor with .data == NULL if out of memory.
 */
zenit_usable_buffer_t zenit_usable_arena_allocate(
    zenit_usable_arena_t *ua, size_t size);

/**
 * @brief Free a buffer, returning its block to the usable arena's free-list.
 *
 * Validates that the buffer's header state is IN_USE (rejects double-free),
 * transitions it to FREE, inserts it into the free-list, and coalesces
 * with adjacent free blocks if possible.
 *
 * @param buf Buffer descriptor returned by zenit_usable_arena_allocate().
 * @return ZENIT_RESULT_OK on success, or an error:
 *         - ZENIT_ERROR_NULL if @p buf or its data is NULL
 *         - ZENIT_ERROR_DOUBLE_FREE if the buffer was already freed.
 */
zenit_result_t zenit_usable_buffer_free(zenit_usable_buffer_t *buf);

/**
 * @brief Retrieve the data pointer from a buffer.
 *
 * @param buf Buffer descriptor.
 * @return The data pointer (always valid if @p buf was successfully allocated).
 */
void *zenit_usable_buffer_data(zenit_usable_buffer_t *buf);

/**
 * @brief Retrieve the usable size of a buffer.
 *
 * @param buf Buffer descriptor.
 * @return The requested size.
 */
size_t zenit_usable_buffer_size(const zenit_usable_buffer_t *buf);

#endif
