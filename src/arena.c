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

#include <libzenit/arena.h>
#include <stdlib.h>
#include <string.h>

/* ─── Block state machine ───
 * Each arena block is a 2-state FSM:  BLOCK_FREE ↔ BLOCK_ALLOCATED.
 * Each sub-allocated buffer is a 2-state FSM: BUF_FREE ↔ BUF_IN_USE.
 */
enum { BLOCK_FREE, BLOCK_ALLOCATED };
enum { BUF_FREE, BUF_IN_USE };

/* ─── Alignment ─── */
#define ALIGNMENT sizeof(void *)
#define ALIGN(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

/* Smallest block worth keeping after a split (header + alignment + footer) */
#define MIN_SPLIT_TOTAL (sizeof(zenit_buf_header_t) + ALIGNMENT + sizeof(zenit_buf_footer_t))

/* ─── Buffer header — precedes every allocation in usable_arena ─── */
typedef struct zenit_buf_header {
    int state;                              /* BUF_FREE or BUF_IN_USE */
    struct zenit_usable_arena_t *ua;        /* owning usable arena */
    size_t size;                            /* total block size (header+data+footer) */
    struct zenit_buf_header *next;          /* free-list next (if FREE) */
    struct zenit_buf_header *prev;          /* free-list prev (if FREE) */
} zenit_buf_header_t;

/* ─── Buffer footer — boundary tag for O(1) backward coalescing ─── */
typedef struct zenit_buf_footer {
    size_t size;                            /* must match header->size */
    int state;                              /* must match header->state */
} zenit_buf_footer_t;

/* ─── Arena ─── */
struct zenit_arena_t {
    size_t total_size;
    size_t block_size;
    size_t block_count;
    unsigned char *memory;
    unsigned char *bitmap;
    size_t bitmap_size;
};

/* ─── Usable arena ─── */
struct zenit_usable_arena_t {
    zenit_arena_t *arena;
    size_t block_offset;
    size_t block_count;
    unsigned char *memory;
    size_t size;
    zenit_buf_header_t *free_list;
};

/* ─── Footer helpers ─── */

/* Initialise a block's footer with the same size and state as the header. */
static void footer_sync(zenit_buf_header_t *h) {
    zenit_buf_footer_t *f = (zenit_buf_footer_t *)((unsigned char *)h + h->size - sizeof(zenit_buf_footer_t));
    f->size = h->size;
    f->state = h->state;
}

/* ─── Bitmap helpers for arena block tracking ─── */

static void bitmap_set(unsigned char *bitmap, size_t index, int value) {
    size_t byte = index / 8;
    size_t bit = index % 8;
    if (value) {
        bitmap[byte] |= (1u << bit);
    } else {
        bitmap[byte] &= ~(1u << bit);
    }
}

static int bitmap_get(const unsigned char *bitmap, size_t index) {
    size_t byte = index / 8;
    size_t bit = index % 8;
    return (bitmap[byte] >> bit) & 1u;
}

/*
 * Find `needed` consecutive zero bits in the bitmap.
 * Returns 0 and writes the start index to *out_start on success.
 * Returns -1 if no run of `needed` free blocks exists.
 */
static int bitmap_find_contiguous(
    const unsigned char *bitmap,
    size_t block_count,
    size_t needed,
    size_t *out_start
) {
    size_t run = 0;
    for (size_t i = 0; i < block_count; i++) {
        if (bitmap_get(bitmap, i) == 0) {
            run++;
            if (run == needed) {
                *out_start = i - needed + 1;
                return 0;
            }
        } else {
            run = 0;
        }
    }
    return -1;
}

/* ─── Arena API ─── */

zenit_arena_t *zenit_arena_create(size_t total_size, size_t block_size) {
    /* Validate that parameters are sane and evenly divisible */
    if (total_size == 0 || block_size == 0 || total_size % block_size != 0) {
        return NULL;
    }

    /* Allocate the arena handle */
    zenit_arena_t *arena = malloc(sizeof(zenit_arena_t));
    if (arena == NULL) {
        return NULL;
    }

    arena->total_size = total_size;
    arena->block_size = block_size;
    arena->block_count = total_size / block_size;
    arena->bitmap_size = (arena->block_count + 7) / 8;

    /* Allocate the raw memory pool — calloc so it is zeroed */
    arena->memory = calloc(1, total_size);
    if (arena->memory == NULL) {
        free(arena);
        return NULL;
    }

    /* Allocate the bitmap — calloc so all bits start at 0 (FREE) */
    arena->bitmap = calloc(1, arena->bitmap_size);
    if (arena->bitmap == NULL) {
        free(arena->memory);
        free(arena);
        return NULL;
    }

    return arena;
}

void zenit_arena_destroy(zenit_arena_t *arena) {
    if (arena == NULL) {
        return;
    }
    free(arena->bitmap);
    free(arena->memory);
    free(arena);
}

zenit_usable_arena_t *zenit_arena_acquire(zenit_arena_t *arena, size_t size) {
    if (arena == NULL || size == 0 || size % arena->block_size != 0) {
        return NULL;
    }

    size_t needed = size / arena->block_size;
    size_t start = 0;

    /* Find contiguous free blocks in the arena bitmap */
    if (bitmap_find_contiguous(arena->bitmap, arena->block_count, needed, &start) != 0) {
        return NULL;
    }

    /* Mark blocks as ALLOCATED (BLOCK_FREE → BLOCK_ALLOCATED) */
    for (size_t i = start; i < start + needed; i++) {
        bitmap_set(arena->bitmap, i, 1);
    }

    /* Allocate usable arena handle */
    zenit_usable_arena_t *ua = malloc(sizeof(zenit_usable_arena_t));
    if (ua == NULL) {
        /* Roll back bitmap changes */
        for (size_t i = start; i < start + needed; i++) {
            bitmap_set(arena->bitmap, i, 0);
        }
        return NULL;
    }

    ua->arena = arena;
    ua->block_offset = start;
    ua->block_count = needed;
    ua->memory = arena->memory + start * arena->block_size;
    ua->size = size;

    /* Initialise the free-list with one large free block covering the region */
    zenit_buf_header_t *initial = (zenit_buf_header_t *)ua->memory;
    initial->state = BUF_FREE;
    initial->ua = ua;
    initial->size = size;
    initial->next = NULL;
    initial->prev = NULL;
    footer_sync(initial);

    ua->free_list = initial;

    return ua;
}

zenit_result_t zenit_arena_release(zenit_arena_t *arena, zenit_usable_arena_t *ua) {
    /* Reject NULL pointers on either parameter */
    if (arena == NULL || ua == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Walk every block in the usable arena's memory and verify
     * that no buffer is still IN_USE. This prevents leaking
     * allocated buffers when releasing the usable arena. */
    const unsigned char *pos = ua->memory;
    const unsigned char *end = ua->memory + ua->size;
    while (pos < end) {
        const zenit_buf_header_t *h = (const zenit_buf_header_t *)pos;
        if (h->state == BUF_IN_USE) {
            /* Outstanding buffer — reject the release */
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_STATE);
        }
        /* Advance to the next block (all blocks have headers, free or not) */
        if (h->size == 0) {
            /* Corrupted block: can't advance safely */
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_CORRUPT);
        }
        pos += h->size;
    }

    /* Transition arena blocks back to FREE (BLOCK_ALLOCATED → BLOCK_FREE) */
    for (size_t i = ua->block_offset; i < ua->block_offset + ua->block_count; i++) {
        bitmap_set(arena->bitmap, i, 0);
    }

    /* Release the usable arena handle */
    free(ua);
    return ZENIT_RESULT_OK;
}

/* ─── Usable arena / buffer API ─── */

/*
 * Take a free block, split it if the leftover is large enough, remove it
 * from the free list, and mark it IN_USE.  Returns a populated buffer
 * descriptor (caller provides the skeleton { NULL, 0 }).
 */
static zenit_usable_buffer_t claim_block(
    zenit_usable_arena_t *ua,
    zenit_buf_header_t *walk,
    size_t needed,
    size_t aligned
) {
    zenit_usable_buffer_t result = { NULL, 0 };

    size_t remainder = walk->size - needed;
    if (remainder >= MIN_SPLIT_TOTAL) {
        walk->size = needed;
        footer_sync(walk);

        zenit_buf_header_t *split = (zenit_buf_header_t *)((unsigned char *)walk + needed);
        split->state = BUF_FREE;
        split->ua = ua;
        split->size = remainder;
        footer_sync(split);

        split->next = walk->next;
        split->prev = walk;
        if (walk->next != NULL) {
            walk->next->prev = split;
        }
        walk->next = split;
    }

    if (walk->prev != NULL) {
        walk->prev->next = walk->next;
    } else {
        ua->free_list = walk->next;
    }
    if (walk->next != NULL) {
        walk->next->prev = walk->prev;
    }
    walk->prev = NULL;
    walk->next = NULL;

    walk->state = BUF_IN_USE;
    footer_sync(walk);

    result.data = (unsigned char *)walk + sizeof(zenit_buf_header_t);
    result.size = aligned;
    return result;
}

zenit_usable_buffer_t zenit_usable_arena_allocate(
    zenit_usable_arena_t *ua, size_t size
) {
    zenit_usable_buffer_t result = { NULL, 0 };

    if (ua == NULL || size == 0) {
        return result;
    }

    size_t aligned = ALIGN(size);
    size_t needed = sizeof(zenit_buf_header_t) + aligned + sizeof(zenit_buf_footer_t);

    zenit_buf_header_t *walk = ua->free_list;
    while (walk != NULL) {
        if (walk->state == BUF_FREE && walk->size >= needed) {
            return claim_block(ua, walk, needed, aligned);
        }
        walk = walk->next;
    }

    return result;
}

zenit_result_t zenit_usable_buffer_free(zenit_usable_buffer_t *buf) {
    /* NULL buffer or NULL data pointer is an invalid operation */
    if (buf == NULL || buf->data == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Locate the block header */
    zenit_buf_header_t *h = (zenit_buf_header_t *)((unsigned char *)buf->data - sizeof(zenit_buf_header_t));

    /* Validate state: reject double-free or corrupted state */
    if (h->state != BUF_IN_USE) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_DOUBLE_FREE);
    }

    /* Transition: BUF_IN_USE → BUF_FREE */
    h->state = BUF_FREE;
    footer_sync(h);

    zenit_usable_arena_t *ua = h->ua;

    /* ─── Coalesce with next block if it is free ─── */
    unsigned char *next_ptr = (unsigned char *)h + h->size;
    const unsigned char *ua_end = ua->memory + ua->size;
    if (next_ptr < ua_end) {
        zenit_buf_header_t *next = (zenit_buf_header_t *)next_ptr;
        if (next->state == BUF_FREE) {
            /* Merge next into h */
            h->size += next->size;
            footer_sync(h);

            /* Remove next from free list */
            if (next->prev != NULL) {
                next->prev->next = next->next;
            } else {
                ua->free_list = next->next;
            }
            if (next->next != NULL) {
                next->next->prev = next->prev;
            }
        }
    }

    /* ─── Coalesce with previous block if it is free ─── */
    if ((unsigned char *)h > ua->memory) {
        const zenit_buf_footer_t *prev_f = (const zenit_buf_footer_t *)((unsigned char *)h - sizeof(zenit_buf_footer_t));
        if (prev_f->state == BUF_FREE) {
            zenit_buf_header_t *prev = (zenit_buf_header_t *)((unsigned char *)h - prev_f->size);
            if (prev->state == BUF_FREE) {
                /* Merge h into prev */
                prev->size += h->size;
                footer_sync(prev);

                /* Remove h from free list if it was re-added (it isn't yet —
                 * we only remove next above, not h) but h is not in the free
                 * list at this point. So we just discard h and keep prev. */
                h = prev;
            }
        }
    }

    /* Insert the (possibly coalesced) block into the free list */
    h->next = ua->free_list;
    h->prev = NULL;
    if (ua->free_list != NULL) {
        ua->free_list->prev = h;
    }
    ua->free_list = h;

    return ZENIT_RESULT_OK;
}

void *zenit_usable_buffer_data(zenit_usable_buffer_t *buf) {
    if (buf == NULL) {
        return NULL;
    }
    return buf->data;
}

size_t zenit_usable_buffer_size(const zenit_usable_buffer_t *buf) {
    if (buf == NULL) {
        return 0;
    }
    return buf->size;
}
