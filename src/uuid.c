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

#include <libzenit/uuid.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <Windows.h>
#include <bcrypt.h>
#elif defined(__APPLE__)
#include <stdlib.h>
#else
#include <sys/random.h>
#include <unistd.h>
#include <errno.h>
#endif

/**
 * @brief Fill a buffer with cryptographically secure random bytes.
 *
 * Uses the platform's preferred CSPRNG:
 *   - Linux:   getrandom() (preferred), then /dev/urandom fallback
 *   - macOS:   arc4random_buf()
 *   - Windows: BCryptGenRandom()
 *
 * @param buf  Output buffer.
 * @param len  Number of bytes to fill.
 * @return ZENIT_RESULT_OK on success, ZENIT_ERROR_ALLOC if the source fails.
 */
static zenit_result_t random_bytes(void *buf, size_t len) {
#if defined(_WIN32)
    if (BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    return ZENIT_RESULT_OK;
#elif defined(__APPLE__)
    arc4random_buf(buf, len);
    return ZENIT_RESULT_OK;
#else
    /* Try getrandom() first (Linux 3.17+, glibc 2.25+) */
    ssize_t n = getrandom(buf, len, 0);
    if (n > 0 && (size_t)n == len) {
        return ZENIT_RESULT_OK;
    }
    /* Fallback: read from /dev/urandom */
    FILE *f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    size_t total = 0;
    while (total < len) {
        size_t r = fread((unsigned char *)buf + total, 1, len - total, f);
        if (r == 0) {
            fclose(f);
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }
        total += r;
    }
    fclose(f);
    return ZENIT_RESULT_OK;
#endif
}

zenit_result_t zenit_uuid_generate(zenit_uuid_t *out) {
    if (out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Fill all 16 bytes with random data */
    zenit_result_t r = random_bytes(out->bytes, 16);
    if (r.error != ZENIT_OK) {
        return r;
    }

    /* Set version (4) in the 7th byte (4 most significant bits) */
    out->bytes[6] = (out->bytes[6] & 0x0f) | 0x40;

    /* Set variant (10xx) in the 9th byte (2 most significant bits) */
    out->bytes[8] = (out->bytes[8] & 0x3f) | 0x80;

    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_uuid_format(const zenit_uuid_t *uuid, char *out) {
    if (uuid == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Format as: 8-4-4-4-12 hex digits with dashes */
    static const char hex[] = "0123456789abcdef";
    size_t pos = 0;
    for (size_t i = 0; i < 16; i++) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            out[pos++] = '-';
        }
        out[pos++] = hex[(uuid->bytes[i] >> 4) & 0x0f];
        out[pos++] = hex[uuid->bytes[i] & 0x0f];
    }
    out[pos] = '\0';

    return ZENIT_RESULT_OK;
}

/* Convert a hex character to its numeric value (0-15).  Returns -1 on invalid. */
static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

zenit_result_t zenit_uuid_parse(const char *str, zenit_uuid_t *out) {
    if (str == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Check length and dash positions */
    if (strlen(str) != 36) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }
    if (str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-') {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Parse hex pairs, skipping dashes */
    size_t si = 0;
    for (size_t bi = 0; bi < 16; bi++) {
        /* Skip dashes at expected positions */
        if (bi == 4 || bi == 6 || bi == 8 || bi == 10) {
            si++;
        }

        int hi = hex_val(str[si]);
        int lo = hex_val(str[si + 1]);
        if (hi < 0 || lo < 0) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
        }

        out->bytes[bi] = (uint8_t)((hi << 4) | lo);
        si += 2;
    }

    return ZENIT_RESULT_OK;
}

int zenit_uuid_equal(const zenit_uuid_t *a, const zenit_uuid_t *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    return memcmp(a->bytes, b->bytes, 16) == 0 ? 1 : 0;
}
