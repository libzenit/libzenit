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
#include <stdlib.h>
#include <string.h>

/* Convert a nibble (0-15) to a lowercase hex character */
static char nibble_to_hex(unsigned char n) {
    n &= 0x0F;
    return n < 10 ? (char)('0' + n) : (char)('a' + n - 10);
}

/* Convert a hex character to a nibble (0-15), or 0xFF on invalid input */
static unsigned char hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') return (unsigned char)(c - '0');
    if (c >= 'a' && c <= 'f') return (unsigned char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (unsigned char)(c - 'A' + 10);
    return 0xFF;
}

size_t zenit_hex_encode_len(size_t data_len) {
    /* 2 hex chars per byte + null terminator */
    return data_len * 2 + 1;
}

size_t zenit_hex_decode_len(const char *hex) {
    if (hex == NULL) return 0;
    size_t len = strlen(hex);
    /* If odd length, treat as if a leading '0' were present */
    return (len + 1) / 2;
}

char *zenit_hex_encode(const unsigned char *data, size_t len) {
    if (data == NULL && len > 0) return NULL;

    size_t out_len = zenit_hex_encode_len(len);
    char *out = malloc(out_len);
    if (out == NULL) return NULL;

    for (size_t i = 0; i < len; i++) {
        out[i * 2]     = nibble_to_hex((unsigned char)(data[i] >> 4));
        out[i * 2 + 1] = nibble_to_hex((unsigned char)(data[i] & 0x0F));
    }
    out[len * 2] = '\0';

    return out;
}

unsigned char *zenit_hex_decode(const char *hex, size_t *out_len) {
    if (hex == NULL || out_len == NULL) return NULL;

    size_t len = strlen(hex);
    if (len == 0) {
        *out_len = 0;
        unsigned char *out = malloc(1);
        if (out != NULL) out[0] = 0;
        return out;
    }

    /* If odd length, pre-pend a '0' in our iteration */
    size_t decoded_len = (len + 1) / 2;
    unsigned char *out = malloc(decoded_len);
    if (out == NULL) return NULL;

    size_t out_idx = 0;
    size_t i = 0;

    /* Handle odd length: first character alone with implied leading 0 */
    if (len % 2 != 0) {
        unsigned char n = hex_to_nibble(hex[0]);
        if (n == 0xFF) {
            free(out);
            return NULL;
        }
        out[out_idx++] = n;
        i = 1;
    }

    /* Process pairs */
    for (; i < len; i += 2) {
        unsigned char hi = hex_to_nibble(hex[i]);
        unsigned char lo = hex_to_nibble(hex[i + 1]);
        if (hi == 0xFF || lo == 0xFF) {
            free(out);
            return NULL;
        }
        out[out_idx++] = (hi << 4) | lo;
    }

    *out_len = decoded_len;
    return out;
}
