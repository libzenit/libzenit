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

#include <libzenit/base64.h>
#include <stdlib.h>
#include <string.h>

/* Base64 alphabet — RFC 4648 */
static const char enc_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Decode lookup: 0xFF marks invalid characters */
static const unsigned char dec_table[256] = {
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3E,0xFF,0xFF,0xFF,0x3F,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
    0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

size_t zenit_base64_encode_len(size_t input_len) {
    /* 4 output chars per 3 input bytes, rounded up, plus null */
    return ((input_len + 2) / 3) * 4 + 1;
}

size_t zenit_base64_decode_len(const char *encoded) {
    if (encoded == NULL) return 0;
    size_t len = strlen(encoded);
    /* 3 bytes per 4 input chars, ignoring padding */
    return (len / 4) * 3;
}

char *zenit_base64_encode(const unsigned char *data, size_t len) {
    if (data == NULL && len > 0) return NULL;

    size_t out_len = zenit_base64_encode_len(len);
    char *out = malloc(out_len);
    if (out == NULL) return NULL;

    size_t i = 0, o = 0;
    /* Process full 3-byte groups */
    while (i + 3 <= len) {
        unsigned int a = data[i++];
        unsigned int b = data[i++];
        unsigned int c = data[i++];
        out[o++] = enc_table[(a >> 2) & 0x3F];
        out[o++] = enc_table[((a << 4) | (b >> 4)) & 0x3F];
        out[o++] = enc_table[((b << 2) | (c >> 6)) & 0x3F];
        out[o++] = enc_table[c & 0x3F];
    }

    /* Handle remaining bytes with padding */
    size_t remaining = len - i;
    if (remaining == 1) {
        unsigned int a = data[i];
        out[o++] = enc_table[(a >> 2) & 0x3F];
        out[o++] = enc_table[(a << 4) & 0x3F];
        out[o++] = '=';
        out[o++] = '=';
    } else if (remaining == 2) {
        unsigned int a = data[i];
        unsigned int b = data[i + 1];
        out[o++] = enc_table[(a >> 2) & 0x3F];
        out[o++] = enc_table[((a << 4) | (b >> 4)) & 0x3F];
        out[o++] = enc_table[(b << 2) & 0x3F];
        out[o++] = '=';
    }

    out[o] = '\0';
    return out;
}

unsigned char *zenit_base64_decode(const char *encoded, size_t *out_len) {
    if (encoded == NULL) return NULL;

    size_t len = strlen(encoded);
    /* Minimum valid length is 4 chars, must be multiple of 4 */
    if (len < 4 || (len % 4) != 0) return NULL;

    /* Count padding characters */
    size_t padding = 0;
    if (encoded[len - 1] == '=') padding++;
    if (len > 1 && encoded[len - 2] == '=') padding++;

    size_t max_out = zenit_base64_decode_len(encoded);
    unsigned char *out = malloc(max_out);
    if (out == NULL) return NULL;

    size_t o = 0;
    size_t i;
    for (i = 0; i < len; i += 4) {
        unsigned char d0 = dec_table[(unsigned char)encoded[i]];
        unsigned char d1 = dec_table[(unsigned char)encoded[i + 1]];
        unsigned char d2 = dec_table[(unsigned char)encoded[i + 2]];
        unsigned char d3 = dec_table[(unsigned char)encoded[i + 3]];

        /* Reject invalid characters */
        if (d0 == 0xFF || d1 == 0xFF) {
            free(out);
            return NULL;
        }

        unsigned int triple = (d0 << 18) | (d1 << 12);

        if (encoded[i + 2] != '=') {
            if (d2 == 0xFF) {
                free(out);
                return NULL;
            }
            triple |= (d2 << 6);
        }

        if (encoded[i + 3] != '=') {
            if (d3 == 0xFF) {
                free(out);
                return NULL;
            }
            triple |= d3;
        }

        out[o++] = (triple >> 16) & 0xFF;
        if (encoded[i + 2] != '=') {
            out[o++] = (triple >> 8) & 0xFF;
        }
        if (encoded[i + 3] != '=') {
            out[o++] = triple & 0xFF;
        }
    }

    if (out_len != NULL) {
        *out_len = o;
    }
    return out;
}
