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

#include <libzenit/trie.h>
#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* Alphabet size — 26 lowercase English letters */
#define TRIE_ALPHABET_SIZE 26

/* ---------------------------------------------------------------------------
 * Internal node structure.
 *
 * Each node holds a fixed array of 26 child pointers (one per lowercase
 * letter) plus an int value and a flag indicating whether this node is
 * the end of an inserted key.
 * -------------------------------------------------------------------------*/

struct zenit_trie_node_t {
    int value;                              /* User value stored at this node */
    int is_end;                             /* 1 if this node terminates a key */
    struct zenit_trie_node_t* children[TRIE_ALPHABET_SIZE]; /* Child pointers (one per letter) */
};

/* ---------------------------------------------------------------------------
 * Trie handle — owns the root node, a key counter, and an allocator.
 * -------------------------------------------------------------------------*/

struct zenit_trie_t {
    struct zenit_trie_node_t* root;         /* Sentinel root (always present, never stores a value) */
    size_t count;                           /* Number of keys currently stored */
    zenit_allocator_t allocator;            /* Allocator used for all node allocations */
};

/* ---------------------------------------------------------------------------
 * Map an ASCII letter to a 0..25 index.
 *
 * Returns -1 for non-alphabetic characters.  Both uppercase and lowercase
 * are accepted and mapped to the same index.
 * -------------------------------------------------------------------------*/

static inline int char_to_index(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    }
    return -1;
}

/* ---------------------------------------------------------------------------
 * Allocate and zero-initialise a single trie node through a given allocator.
 * Returns NULL on allocation failure.
 * -------------------------------------------------------------------------*/

static struct zenit_trie_node_t* node_create(zenit_allocator_t a) {
    /* Use alloc_zero so all child pointers start as NULL and is_end is 0 */
    struct zenit_trie_node_t* node = zenit_allocator_alloc_zero(a, 1, sizeof(struct zenit_trie_node_t));
    return node;
}

/* ---------------------------------------------------------------------------
 * Recursively free a node and all its descendants.
 * Called from destroy and clear.
 * -------------------------------------------------------------------------*/

static void node_destroy(struct zenit_trie_node_t* node, zenit_allocator_t a) {
    /* NULL is safe — simplifies recursion termination */
    if (node == NULL) {
        return;
    }
    /* Recurse into every child slot */
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            node_destroy(node->children[i], a);
            node->children[i] = NULL;
        }
    }
    /* Free the node itself through the allocator */
    a.free_fn(node, a.ctx);
}

/* =========================================================================
 * Public API
 * =========================================================================*/

zenit_trie_t* zenit_trie_create(void) {
    /* Delegate to the allocator variant with the default allocator */
    return zenit_trie_create_with_allocator(ZENIT_ALLOCATOR_DEFAULT);
}

zenit_trie_t* zenit_trie_create_with_allocator(zenit_allocator_t allocator) {
    /* Allocate the handle itself through the given allocator */
    zenit_trie_t* trie = allocator.alloc_fn(sizeof(zenit_trie_t), allocator.ctx);
    if (trie == NULL) {
        return NULL;
    }

    /* Store the allocator so all future operations use the correct functions */
    trie->allocator = allocator;

    /* No keys yet */
    trie->count = 0;

    /* Create the sentinel root node — its children will hold actual keys */
    trie->root = node_create(allocator);
    if (trie->root == NULL) {
        /* Roll back: free the handle we just allocated */
        allocator.free_fn(trie, allocator.ctx);
        return NULL;
    }

    return trie;
}

void zenit_trie_destroy(zenit_trie_t* trie) {
    /* NULL check — makes caller code simpler */
    if (trie == NULL) {
        return;
    }

    /* Snapshot the allocator before we free anything */
    zenit_allocator_t a = trie->allocator;

    /* Recursively free every node (root + all descendants) */
    node_destroy(trie->root, a);

    /* Free the handle */
    a.free_fn(trie, a.ctx);
}

zenit_result_t zenit_trie_insert(zenit_trie_t* trie, const char* key, int value) {
    /* Validate inputs */
    if (trie == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Start at the root and walk the tree, creating nodes on demand */
    struct zenit_trie_node_t* node = trie->root;

    for (const char* p = key; *p != '\0'; p++) {
        int idx = char_to_index(*p);

        /* Skip non-alphabetic characters (digits, punctuation, etc.) */
        if (idx < 0 || idx >= TRIE_ALPHABET_SIZE) {
            continue;
        }

        /* Create child if it does not exist yet */
        if (node->children[idx] == NULL) {
            node->children[idx] = node_create(trie->allocator);
            if (node->children[idx] == NULL) {
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            }
        }

        /* Advance to the child */
        node = node->children[idx];
    }

    /* At the final node — mark it as end-of-key if it is not already */
    if (!node->is_end) {
        node->is_end = 1;
        trie->count++;
    }

    /* Set/update the stored value */
    node->value = value;

    return ZENIT_RESULT_OK;
}

int zenit_trie_search(const zenit_trie_t* trie, const char* key, int* out_value) {
    /* NULL inputs → not found */
    if (trie == NULL || key == NULL) {
        return 0;
    }

    /* Walk the tree following the characters of the key */
    const struct zenit_trie_node_t* node = trie->root;

    for (const char* p = key; *p != '\0'; p++) {
        int idx = char_to_index(*p);

        if (idx < 0 || idx >= TRIE_ALPHABET_SIZE) {
            continue;
        }

        if (node->children[idx] == NULL) {
            return 0;
        }

        node = node->children[idx];
    }

    if (!node->is_end) {
        return 0;
    }

    if (out_value != NULL) {
        *out_value = node->value;
    }

    return 1;
}

int zenit_trie_starts_with(const zenit_trie_t* trie, const char* prefix) {
    if (trie == NULL || prefix == NULL) {
        return 0;
    }

    const struct zenit_trie_node_t* node = trie->root;

    for (const char* p = prefix; *p != '\0'; p++) {
        int idx = char_to_index(*p);

        /* Skip non-alphabetic characters */
        if (idx < 0 || idx >= TRIE_ALPHABET_SIZE) {
            continue;
        }

        /* Missing child → no word has this prefix */
        if (node->children[idx] == NULL) {
            return 0;
        }

        node = node->children[idx];
    }

    /* We successfully walked the entire prefix path without hitting NULL */
    return 1;
}

zenit_result_t zenit_trie_delete(zenit_trie_t* trie, const char* key) {
    /* Validate inputs */
    if (trie == NULL || key == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Walk the tree — every child must exist for the key to be present */
    struct zenit_trie_node_t* node = trie->root;

    for (const char* p = key; *p != '\0'; p++) {
        int idx = char_to_index(*p);

        /* Skip non-alphabetic characters */
        if (idx < 0 || idx >= TRIE_ALPHABET_SIZE) {
            continue;
        }

        /* Missing child → key is not stored */
        if (node->children[idx] == NULL) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
        }

        node = node->children[idx];
    }

    /* The terminal node must be an actual end-of-key */
    if (!node->is_end) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    /* Unmark the node and decrement the key counter */
    node->is_end = 0;
    trie->count--;

    return ZENIT_RESULT_OK;
}

size_t zenit_trie_count(const zenit_trie_t* trie) {
    /* NULL handle → empty */
    if (trie == NULL) {
        return 0;
    }
    return trie->count;
}

void zenit_trie_clear(zenit_trie_t* trie) {
    /* NULL check — safe no-op */
    if (trie == NULL) {
        return;
    }

    zenit_allocator_t a = trie->allocator;

    /* Destroy every direct child of the root node */
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (trie->root->children[i] != NULL) {
            node_destroy(trie->root->children[i], a);
            trie->root->children[i] = NULL;
        }
    }

    /* Reset the key counter — the trie is ready for reuse */
    trie->count = 0;
}
