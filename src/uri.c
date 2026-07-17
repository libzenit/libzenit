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

#include <libzenit/uri.h>
#include <stdlib.h>
#include <string.h>

/* Test whether a character is "unreserved" per RFC 3986 and does not
 * need percent-encoding: ALPHA, DIGIT, '-', '_', '.', '~'. */
static int is_unreserved(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '-' || c == '_' || c == '.' || c == '~';
}

/* Hex digit for percent-encoding */
static char hex_digit(unsigned char v) {
    v &= 0x0F;
    return v < 10 ? (char)('0' + v) : (char)('A' + v - 10);
}

/* Convert a hex character to a value (0-15), or 0xFF on invalid input */
static unsigned char hex_val(char c) {
    if (c >= '0' && c <= '9') return (unsigned char)(c - '0');
    if (c >= 'a' && c <= 'f') return (unsigned char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (unsigned char)(c - 'A' + 10);
    return 0xFF;
}

char *zenit_uri_encode(const char *input) {
    if (input == NULL) return NULL;

    size_t len = strlen(input);
    /* Worst case: every byte becomes %XX — 3x + null */
    size_t max_out = len * 3 + 1;
    char *out = malloc(max_out);
    if (out == NULL) return NULL;

    size_t o = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (is_unreserved((char)c)) {
            out[o++] = (char)c;
        } else {
            out[o++] = '%';
            out[o++] = hex_digit(c >> 4);
            out[o++] = hex_digit(c & 0x0F);
        }
    }
    out[o] = '\0';

    return out;
}

char *zenit_uri_decode(const char *encoded) {
    if (encoded == NULL) return NULL;

    size_t len = strlen(encoded);
    /* Decoded string is never longer than the encoded one */
    char *out = malloc(len + 1);
    if (out == NULL) return NULL;

    size_t o = 0;
    for (size_t i = 0; i < len; i++) {
        char c = encoded[i];
        if (c == '+') {
            out[o++] = ' ';
        } else if (c == '%') {
            /* Need at least two more characters for a valid %XX sequence */
            if (i + 2 >= len) {
                free(out);
                return NULL;
            }
            unsigned char hi = hex_val(encoded[i + 1]);
            unsigned char lo = hex_val(encoded[i + 2]);
            if (hi == 0xFF || lo == 0xFF) {
                free(out);
                return NULL;
            }
            out[o++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            out[o++] = c;
        }
    }
    out[o] = '\0';

    return out;
}
