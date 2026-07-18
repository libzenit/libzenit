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

#include <libzenit/hex.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

static char nibble_to_hex(unsigned char n) {
    n &= 0x0F;
    return n < 10 ? (char)('0' + n) : (char)('a' + n - 10);
}

static unsigned char hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') return (unsigned char)(c - '0');
    if (c >= 'a' && c <= 'f') return (unsigned char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (unsigned char)(c - 'A' + 10);
    return 0xFF;
}

size_t zenit_hex_encode_len(size_t data_len) {
    return data_len * 2 + 1;
}

size_t zenit_hex_decode_len(const char *hex) {
    if (hex == NULL) return 0;
    size_t len = strlen(hex);
    return (len + 1) / 2;
}

static char *encode_impl(const unsigned char *data, size_t len, zenit_allocator_t a) {
    if (data == NULL && len > 0) return NULL;

    size_t out_len = zenit_hex_encode_len(len);
    char *out = a.alloc_fn(out_len, a.ctx);
    if (out == NULL) return NULL;

    for (size_t i = 0; i < len; i++) {
        out[i * 2]     = nibble_to_hex((unsigned char)(data[i] >> 4));
        out[i * 2 + 1] = nibble_to_hex((unsigned char)(data[i] & 0x0F));
    }
    out[len * 2] = '\0';

    return out;
}

static unsigned char *decode_impl(const char *hex, size_t *out_len, zenit_allocator_t a) {
    if (hex == NULL || out_len == NULL) return NULL;

    size_t len = strlen(hex);
    if (len == 0) {
        *out_len = 0;
        unsigned char *out = a.alloc_fn(1, a.ctx);
        if (out != NULL) out[0] = 0;
        return out;
    }

    size_t decoded_len = (len + 1) / 2;
    unsigned char *out = a.alloc_fn(decoded_len, a.ctx);
    if (out == NULL) return NULL;

    size_t out_idx = 0;
    size_t i = 0;

    if (len % 2 != 0) {
        unsigned char n = hex_to_nibble(hex[0]);
        if (n == 0xFF) {
            a.free_fn(out, a.ctx);
            return NULL;
        }
        out[out_idx++] = n;
        i = 1;
    }

    for (; i < len; i += 2) {
        unsigned char hi = hex_to_nibble(hex[i]);
        unsigned char lo = hex_to_nibble(hex[i + 1]);
        if (hi == 0xFF || lo == 0xFF) {
            a.free_fn(out, a.ctx);
            return NULL;
        }
        out[out_idx++] = (hi << 4) | lo;
    }

    *out_len = decoded_len;
    return out;
}

char *zenit_hex_encode(const unsigned char *data, size_t len) {
    return zenit_hex_encode_with_allocator(data, len, ZENIT_ALLOCATOR_DEFAULT);
}

char *zenit_hex_encode_with_allocator(const unsigned char *data, size_t len, zenit_allocator_t allocator) {
    return encode_impl(data, len, allocator);
}

unsigned char *zenit_hex_decode(const char *hex, size_t *out_len) {
    return zenit_hex_decode_with_allocator(hex, out_len, ZENIT_ALLOCATOR_DEFAULT);
}

unsigned char *zenit_hex_decode_with_allocator(const char *hex, size_t *out_len, zenit_allocator_t allocator) {
    return decode_impl(hex, out_len, allocator);
}
