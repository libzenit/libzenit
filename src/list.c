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

#include <libzenit/list.h>
#include <libzenit/allocator.h>
#include <string.h>

/* ─── Internal node ───
 * Each node is a doubly linked list node with inline element storage.
 * The element data follows the prev/next pointers.
 */
struct list_node {
    struct list_node *prev;
    struct list_node *next;
    unsigned char data[];
};

/* ─── Internal list state ─── */
struct zenit_list_t {
    struct list_node *head;  /**< Pointer to the first node (NULL if empty) */
    struct list_node *tail;  /**< Pointer to the last node  (NULL if empty) */
    size_t elem_size;        /**< Size in bytes of one element */
    size_t count;            /**< Number of nodes in the list */
    zenit_allocator_t allocator; /**< Allocator used for all memory operations */
};

/* ─── Helper: allocate a new node with a copy of the element ─── */
static struct list_node *node_alloc(zenit_allocator_t allocator, size_t elem_size, const void *elem) {
    struct list_node *n = allocator.alloc_fn(sizeof(struct list_node) + elem_size, allocator.ctx);
    if (n == NULL) {
        return NULL;
    }
    n->prev = NULL;
    n->next = NULL;
    memcpy(n->data, elem, elem_size);
    return n;
}

/* ─── Helper: walk to the node at the given index ───
 * Returns NULL if index >= count.  Chooses head or tail based on
 * which end is closer.
 */
static struct list_node *node_at(const zenit_list_t *list, size_t index) {
    if (index >= list->count) {
        return NULL;
    }

    /* Walk from the closer end */
    if (index < list->count / 2) {
        struct list_node *n = list->head;
        for (size_t i = 0; i < index; i++) {
            n = n->next;
        }
        return n;
    } else {
        struct list_node *n = list->tail;
        size_t from_back = list->count - 1 - index;
        for (size_t i = 0; i < from_back; i++) {
            n = n->prev;
        }
        return n;
    }
}

/* ─── Public API ─── */

zenit_list_t *zenit_list_create(size_t elem_size) {
    return zenit_list_create_with_allocator(elem_size, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_list_t *zenit_list_create_with_allocator(size_t elem_size, zenit_allocator_t allocator) {
    if (elem_size == 0) {
        return NULL;
    }

    zenit_list_t *list = allocator.alloc_fn(sizeof(zenit_list_t), allocator.ctx);
    if (list == NULL) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->elem_size = elem_size;
    list->count = 0;
    list->allocator = allocator;

    return list;
}

void zenit_list_destroy(zenit_list_t *list) {
    if (list == NULL) {
        return;
    }

    /* Free every node by walking forward */
    struct list_node *n = list->head;
    while (n != NULL) {
        struct list_node *next = n->next;
        list->allocator.free_fn(n, list->allocator.ctx);
        n = next;
    }

    list->allocator.free_fn(list, list->allocator.ctx);
}

zenit_result_t zenit_list_push_front(zenit_list_t *list, const void *elem) {
    if (list == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    struct list_node *n = node_alloc(list->allocator, list->elem_size, elem);
    if (n == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    n->next = list->head;
    n->prev = NULL;

    if (list->head != NULL) {
        list->head->prev = n;
    } else {
        list->tail = n;
    }
    list->head = n;
    list->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_list_push_back(zenit_list_t *list, const void *elem) {
    if (list == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    struct list_node *n = node_alloc(list->allocator, list->elem_size, elem);
    if (n == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    n->prev = list->tail;
    n->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = n;
    } else {
        list->head = n;
    }
    list->tail = n;
    list->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_list_pop_front(zenit_list_t *list, void *out_elem) {
    if (list == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (list->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    struct list_node *n = list->head;
    memcpy(out_elem, n->data, list->elem_size);

    list->head = n->next;
    if (list->head != NULL) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }

    list->allocator.free_fn(n, list->allocator.ctx);
    list->count--;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_list_pop_back(zenit_list_t *list, void *out_elem) {
    if (list == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (list->count == 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_EMPTY);
    }

    struct list_node *n = list->tail;
    memcpy(out_elem, n->data, list->elem_size);

    list->tail = n->prev;
    if (list->tail != NULL) {
        list->tail->next = NULL;
    } else {
        list->head = NULL;
    }

    list->allocator.free_fn(n, list->allocator.ctx);
    list->count--;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_list_insert(zenit_list_t *list, size_t index, const void *elem) {
    if (list == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (index > list->count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Insertion at the ends can reuse the fast paths */
    if (index == 0) {
        return zenit_list_push_front(list, elem);
    }
    if (index == list->count) {
        return zenit_list_push_back(list, elem);
    }

    /* Walk to the node currently at `index` — we will insert before it */
    struct list_node *next = node_at(list, index);

    struct list_node *n = node_alloc(list->allocator, list->elem_size, elem);
    if (n == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    n->prev = next->prev;
    n->next = next;
    next->prev->next = n;
    next->prev = n;
    list->count++;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_list_remove(zenit_list_t *list, size_t index, void *out_elem) {
    if (list == NULL || out_elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (index >= list->count) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Removal at the ends can reuse the fast paths */
    if (index == 0) {
        return zenit_list_pop_front(list, out_elem);
    }
    if (index == list->count - 1) {
        return zenit_list_pop_back(list, out_elem);
    }

    struct list_node *n = node_at(list, index);
    memcpy(out_elem, n->data, list->elem_size);

    n->prev->next = n->next;
    n->next->prev = n->prev;

    list->allocator.free_fn(n, list->allocator.ctx);
    list->count--;

    return ZENIT_RESULT_OK;
}

void *zenit_list_get(const zenit_list_t *list, size_t index) {
    if (list == NULL) {
        return NULL;
    }

    struct list_node *n = node_at(list, index);
    if (n == NULL) {
        return NULL;
    }

    return n->data;
}

zenit_result_t zenit_list_set(const zenit_list_t *list, size_t index, const void *elem) {
    if (list == NULL || elem == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    struct list_node *n = node_at(list, index);
    if (n == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    memcpy(n->data, elem, list->elem_size);
    return ZENIT_RESULT_OK;
}

size_t zenit_list_count(const zenit_list_t *list) {
    if (list == NULL) {
        return 0;
    }
    return list->count;
}

int zenit_list_empty(const zenit_list_t *list) {
    if (list == NULL) {
        return 1;
    }
    return list->count == 0 ? 1 : 0;
}

void zenit_list_clear(zenit_list_t *list) {
    if (list == NULL) {
        return;
    }

    /* Free every node */
    struct list_node *n = list->head;
    while (n != NULL) {
        struct list_node *next = n->next;
        list->allocator.free_fn(n, list->allocator.ctx);
        n = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

void zenit_list_foreach(const zenit_list_t *list, zenit_list_visit_fn_t visit, void *ctx) {
    if (list == NULL || visit == NULL) {
        return;
    }

    struct list_node *n = list->head;
    while (n != NULL) {
        visit(n->data, ctx);
        n = n->next;
    }
}

void *zenit_list_front(const zenit_list_t *list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    return list->head->data;
}

void *zenit_list_back(const zenit_list_t *list) {
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }
    return list->tail->data;
}

zenit_iter_t zenit_list_iter(const zenit_list_t *list) {
    zenit_iter_t iter;
    iter.container = (void*)list;
    iter.index = 0;
    iter.count = list ? list->count : 0;
    iter.is_valid = (list != NULL) ? 1 : 0;
    iter.internal = list ? (void*)list->head : NULL;
    return iter;
}

void *zenit_list_iter_next(zenit_iter_t *iter) {
    if (iter == NULL || !iter->is_valid) {
        return NULL;
    }
    struct list_node *n = (struct list_node*)iter->internal;
    if (n == NULL) {
        return NULL;
    }
    /* Capture the data pointer before advancing */
    void *data = n->data;
    iter->internal = (void*)n->next;
    iter->index++;
    return data;
}

void zenit_list_reverse_foreach(const zenit_list_t *list, zenit_list_visit_fn_t visit, void *ctx) {
    if (list == NULL || visit == NULL) {
        return;
    }
    struct list_node *n = list->tail;
    while (n != NULL) {
        visit(n->data, ctx);
        n = n->prev;
    }
}
