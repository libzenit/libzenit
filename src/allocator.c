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

#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

void *zenit_default_alloc(size_t size, void *ctx) {
    (void)ctx;
    return malloc(size);
}

void *zenit_default_realloc(void *ptr, size_t size, void *ctx) {
    (void)ctx;
    return realloc(ptr, size);
}

void zenit_default_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

void *zenit_allocator_realloc(zenit_allocator_t a, void *ptr, size_t old_size, size_t new_size) {
    if (a.realloc_fn != NULL) {
        return a.realloc_fn(ptr, new_size, a.ctx);
    }
    void *new_ptr = a.alloc_fn(new_size, a.ctx);
    if (new_ptr == NULL) {
        return NULL;
    }
    if (ptr != NULL) {
        size_t copy_size = old_size < new_size ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);
        a.free_fn(ptr, a.ctx);
    }
    return new_ptr;
}


