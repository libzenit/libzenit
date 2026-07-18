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

#include <libzenit/path.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

/* ─── internal helpers ─── */

/* Locate the last occurrence of '/' in s.  Returns NULL if none. */
static const char* last_slash(const char *s) {
    const char *p = NULL;
    for (; *s; s++) {
        if (*s == '/') p = s;
    }
    return p;
}

/* Duplicate a string using the given allocator.  Returns NULL on failure. */
static char* strdup_with_allocator(const char *s, zenit_allocator_t a) {
    size_t len = strlen(s);
    char *out = a.alloc_fn(len + 1, a.ctx);
    if (out == NULL) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

/* ─── zenit_path_join ─── */

static char* join_impl(const char *a, const char *b, zenit_allocator_t a_alloc) {
    /* Bail on NULL input */
    if (a == NULL || b == NULL) return NULL;

    size_t a_len = strlen(a);
    size_t b_len = strlen(b);

    /* Adjust lengths to avoid double slash when a ends with '/' and b starts with '/'.
       We skip the trailing slash of 'a' in that case. */
    size_t a_adj = a_len;
    size_t b_off = 0;
    size_t b_adj = b_len;

    if (a_len > 0 && a[a_len - 1] == '/') a_adj--;
    if (b_len > 0 && b[0] == '/') { b_off++; b_adj--; }

    /* Determine whether a separator is needed between the adjusted parts */
    int need_sep = (a_adj > 0 && b_adj > 0);

    /* Total length: adjusted a + optional slash + adjusted b + NUL */
    size_t total = a_adj + b_adj + (size_t)need_sep + 1;
    char *out = a_alloc.alloc_fn(total, a_alloc.ctx);
    if (out == NULL) return NULL;

    /* Copy adjusted a, then optionally '/', then adjusted b, then NUL */
    size_t pos = 0;
    memcpy(out + pos, a, a_adj);
    pos += a_adj;

    if (need_sep) {
        out[pos++] = '/';
    }

    memcpy(out + pos, b + b_off, b_adj);
    pos += b_adj;
    out[pos] = '\0';

    return out;
}

char* zenit_path_join(const char *a, const char *b) {
    return zenit_path_join_with_allocator(a, b, ZENIT_ALLOCATOR_DEFAULT);
}

char* zenit_path_join_with_allocator(const char *a, const char *b, zenit_allocator_t allocator) {
    return join_impl(a, b, allocator);
}

/* ─── zenit_path_dirname ─── */

static char* dirname_impl(const char *path, zenit_allocator_t a) {
    if (path == NULL) return NULL;

    size_t len = strlen(path);
    if (len == 0) {
        /* Empty path → return "." */
        return strdup_with_allocator(".", a);
    }

    /* Walk backwards to find the last slash */
    const char *slash = last_slash(path);
    if (slash == NULL) {
        /* No slash at all → return "." */
        return strdup_with_allocator(".", a);
    }

    /* If the only slash is at position 0 (e.g. "/foo" or "/"), return "/". */
    if (slash == path) {
        return strdup_with_allocator("/", a);
    }

    /* Return the portion up to (but not including) the last slash */
    size_t dir_len = (size_t)(slash - path);
    char *out = a.alloc_fn(dir_len + 1, a.ctx);
    if (out == NULL) return NULL;
    memcpy(out, path, dir_len);
    out[dir_len] = '\0';
    return out;
}

char* zenit_path_dirname(const char *path) {
    return zenit_path_dirname_with_allocator(path, ZENIT_ALLOCATOR_DEFAULT);
}

char* zenit_path_dirname_with_allocator(const char *path, zenit_allocator_t allocator) {
    return dirname_impl(path, allocator);
}

/* ─── zenit_path_basename ─── */

static char* basename_impl(const char *path, zenit_allocator_t a) {
    if (path == NULL) return NULL;

    size_t len = strlen(path);
    if (len == 0) {
        return strdup_with_allocator("", a);
    }

    /* Find the last slash and return everything after it */
    const char *slash = last_slash(path);
    if (slash == NULL) {
        /* No slash → the whole string is the basename */
        return strdup_with_allocator(path, a);
    }

    const char *base = slash + 1;
    size_t base_len = strlen(base);

    char *out = a.alloc_fn(base_len + 1, a.ctx);
    if (out == NULL) return NULL;
    memcpy(out, base, base_len + 1);
    return out;
}

char* zenit_path_basename(const char *path) {
    return zenit_path_basename_with_allocator(path, ZENIT_ALLOCATOR_DEFAULT);
}

char* zenit_path_basename_with_allocator(const char *path, zenit_allocator_t allocator) {
    return basename_impl(path, allocator);
}

/* ─── zenit_path_extension ─── */

static char* extension_impl(const char *path, zenit_allocator_t a) {
    if (path == NULL) return NULL;

    size_t len = strlen(path);
    if (len == 0) {
        return strdup_with_allocator("", a);
    }

    /* Find the last '.' in the basename only */
    const char *slash = last_slash(path);
    /* Start searching from the basename (after the last slash, or from start) */
    const char *base = (slash != NULL) ? slash + 1 : path;

    const char *dot = NULL;
    for (const char *p = base; *p; p++) {
        if (*p == '.') dot = p;
    }

    if (dot == NULL || dot == base) {
        /* No dot, or dot is at the start of basename (hidden file) → no extension */
        return strdup_with_allocator("", a);
    }

    size_t ext_len = strlen(dot);
    char *out = a.alloc_fn(ext_len + 1, a.ctx);
    if (out == NULL) return NULL;
    memcpy(out, dot, ext_len + 1);
    return out;
}

char* zenit_path_extension(const char *path) {
    return zenit_path_extension_with_allocator(path, ZENIT_ALLOCATOR_DEFAULT);
}

char* zenit_path_extension_with_allocator(const char *path, zenit_allocator_t allocator) {
    return extension_impl(path, allocator);
}

/* ─── zenit_path_normalize ─── */

static char* normalize_impl(const char *path, zenit_allocator_t a) {
    if (path == NULL) return NULL;

    size_t len = strlen(path);
    if (len == 0) {
        return strdup_with_allocator("/", a);
    }

    /* Allocate a working buffer large enough for the result (worst case: same length + 1) */
    size_t cap = len + 1;
    char *stack = a.alloc_fn(cap, a.ctx);
    if (stack == NULL) return NULL;

    size_t pos = 0;   /* write position in stack */
    size_t i = 0;     /* read position in path */

    /* Track whether the result starts with '/' for root-anchored paths */
    int is_absolute = (path[0] == '/');

    /* For absolute paths, start with the root separator */
    if (is_absolute) {
        stack[pos++] = '/';
    }

    while (i <= len) {
        /* Skip consecutive slashes */
        while (i < len && path[i] == '/') i++;

        /* If we hit the end after skipping slashes, we are done */
        if (i >= len) break;

        /* Find the end of the current component */
        size_t start = i;
        while (i < len && path[i] != '/') i++;
        size_t clen = i - start;

        if (clen == 1 && path[start] == '.') {
            /* Single dot → skip (stay in current directory) */
            continue;
        }

        if (clen == 2 && path[start] == '.' && path[start + 1] == '.') {
            /* Double dot → pop one component if possible.
               We must not pop above the root for absolute paths. */
            size_t min_pos = is_absolute ? 1 : 0;
            if (pos > min_pos) {
                /* Remove the last character of the component */
                pos--;
                /* Walk backward past the rest of the component name */
                while (pos > min_pos && stack[pos - 1] != '/') pos--;
                /* Remove the separator that preceded this component */
                if (pos > min_pos) {
                    pos--;
                }
            }
            continue;
        }

        /* Regular component — add separator if needed.
           Skip the separator for the very first component of an absolute path
           (the root '/' is already in the buffer at that point). */
        if (pos > 0) {
            /* The root '/' is at pos=1 for abs paths; we never add '/' there. */
            if (!(is_absolute && pos == 1)) {
                stack[pos++] = '/';
            }
        }

        memcpy(stack + pos, path + start, clen);
        pos += clen;
    }

    /* If the result is empty, return the appropriate root representation */
    if (pos == 0) {
        /* Free the working buffer and return "/" for absolute, "." for relative */
        a.free_fn(stack, a.ctx);
        if (is_absolute) {
            return strdup_with_allocator("/", a);
        } else {
            return strdup_with_allocator(".", a);
        }
    }

    /* Shrink-to-fit: allocate exact result and free the working buffer */
    char *out = a.alloc_fn(pos + 1, a.ctx);
    if (out == NULL) {
        a.free_fn(stack, a.ctx);
        return NULL;
    }
    memcpy(out, stack, pos);
    out[pos] = '\0';
    a.free_fn(stack, a.ctx);
    return out;
}

char* zenit_path_normalize(const char *path) {
    return zenit_path_normalize_with_allocator(path, ZENIT_ALLOCATOR_DEFAULT);
}

char* zenit_path_normalize_with_allocator(const char *path, zenit_allocator_t allocator) {
    return normalize_impl(path, allocator);
}
