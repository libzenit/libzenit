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

#include <libzenit/string.h>
#include <libzenit/vector.h>
#include <string.h>

/**
 * @brief Internal string state.
 *
 * Stores characters (including the null terminator) in a vector of
 * unsigned char.  The vector count always includes the trailing '\0'.
 * The allocator is stored separately so we can free the handle in destroy
 * (the vector keeps its own copy internally, but that is opaque to us).
 */
struct zenit_string_t {
    zenit_vector_t *vec;       /**< Vector storing chars, including null terminator */
    zenit_allocator_t allocator; /**< Allocator used for the string handle */
};

/* Re-add the null terminator at the end of the vector */
static zenit_result_t push_null(zenit_string_t *s) {
    char nul = '\0';
    return zenit_vector_push(s->vec, &nul);
}

static zenit_string_t *string_create_empty(zenit_allocator_t allocator) {
    /* Allocate the string handle using the custom allocator */
    zenit_string_t *s = allocator.alloc_fn(sizeof(zenit_string_t), allocator.ctx);
    if (s == NULL) {
        return NULL;
    }

    s->allocator = allocator;

    /* Create the backing vector — elem_size = 1 for chars */
    s->vec = zenit_vector_create_with_allocator(1, allocator);
    if (s->vec == NULL) {
        allocator.free_fn(s, allocator.ctx);
        return NULL;
    }

    /* Push the initial null terminator so the vector always has one */
    zenit_result_t r = push_null(s);
    if (r.error != ZENIT_OK) {
        zenit_vector_destroy(s->vec);
        allocator.free_fn(s, allocator.ctx);
        return NULL;
    }

    return s;
}

/* ─── Constructors ──────────────────────────────────────────────────────── */

zenit_string_t *zenit_string_create(void) {
    return string_create_empty(ZENIT_ALLOCATOR_DEFAULT);
}

zenit_string_t *zenit_string_create_with_allocator(zenit_allocator_t allocator) {
    return string_create_empty(allocator);
}

zenit_string_t *zenit_string_create_from(const char *cstr) {
    return zenit_string_create_from_with_allocator(cstr, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_string_t *zenit_string_create_from_with_allocator(const char *cstr, zenit_allocator_t allocator) {
    /* First create an empty string (which already has a null terminator) */
    zenit_string_t *s = string_create_empty(allocator);
    if (s == NULL) {
        return NULL;
    }

    if (cstr == NULL || cstr[0] == '\0') {
        return s;
    }

    /* Append all bytes from the C string (this will maintain the null terminator) */
    size_t len = strlen(cstr);
    zenit_result_t r = zenit_string_append(s, cstr, len);
    if (r.error != ZENIT_OK) {
        zenit_string_destroy(s);
        return NULL;
    }

    return s;
}

/* ─── Destructor ────────────────────────────────────────────────────────── */

void zenit_string_destroy(zenit_string_t *s) {
    if (s == NULL) {
        return;
    }
    /* Destroy the backing vector first (frees its buffer and handle) */
    zenit_vector_destroy(s->vec);
    /* Free the string handle using the stored allocator */
    s->allocator.free_fn(s, s->allocator.ctx);
}

/* ─── Append ────────────────────────────────────────────────────────────── */

zenit_result_t zenit_string_append(zenit_string_t *s, const void *data, size_t len) {
    if (s == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (len > 0 && data == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    if (len == 0) {
        return ZENIT_RESULT_OK;
    }

    /* Pre-reserve enough capacity for current data + new data + null terminator.
       This ensures all subsequent pushes cannot fail due to allocation,
       so the string invariants are always maintained. */
    size_t count = zenit_vector_count(s->vec);
    size_t needed = count + len;
    zenit_result_t r = zenit_vector_reserve(s->vec, needed);
    if (r.error != ZENIT_OK) {
        /* Allocation failed — string state unchanged */
        return r;
    }

    /* Pop the null terminator */
    char nul;
    zenit_vector_pop(s->vec, &nul);

    /* Push each byte — no grow needed since we pre-reserved */
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        zenit_vector_push(s->vec, &bytes[i]);
    }

    /* Re-add the null terminator */
    push_null(s);

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_string_append_cstr(zenit_string_t *s, const char *cstr) {
    if (s == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    if (cstr == NULL || cstr[0] == '\0') {
        return ZENIT_RESULT_OK;
    }

    return zenit_string_append(s, cstr, strlen(cstr));
}

/* ─── Accessors ─────────────────────────────────────────────────────────── */

const char *zenit_string_cstr(const zenit_string_t *s) {
    if (s == NULL) {
        return NULL;
    }
    /* The vector stores chars contiguously, so getting index 0 gives us
       the start of the buffer castable to const char* */
    return (const char *)zenit_vector_get(s->vec, 0);
}

size_t zenit_string_length(const zenit_string_t *s) {
    if (s == NULL) {
        return 0;
    }
    size_t count = zenit_vector_count(s->vec);
    /* count >= 1 because we always maintain a null terminator */
    return count - 1;
}

size_t zenit_string_capacity(const zenit_string_t *s) {
    if (s == NULL) {
        return 0;
    }
    return zenit_vector_capacity(s->vec);
}

/* ─── Mutators ──────────────────────────────────────────────────────────── */

void zenit_string_clear(zenit_string_t *s) {
    if (s == NULL) {
        return;
    }
    /* Clear the vector and re-add the null terminator */
    zenit_vector_clear(s->vec);
    push_null(s);
}

zenit_result_t zenit_string_reserve(zenit_string_t *s, size_t capacity) {
    if (s == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_vector_reserve(s->vec, capacity);
}

zenit_result_t zenit_string_shrink_to_fit(zenit_string_t *s) {
    if (s == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
    return zenit_vector_shrink_to_fit(s->vec);
}

int zenit_string_empty(const zenit_string_t *s) {
    if (s == NULL) {
        return 1;
    }
    /* String is empty when length == 0, i.e. count <= 1 */
    size_t count = zenit_vector_count(s->vec);
    return (count <= 1) ? 1 : 0;
}
