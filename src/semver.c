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

#include <libzenit/semver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Advances *p past digits and returns the parsed integer. Sets *ok to 0 on overflow. */
static int parse_num(const char **p, int *ok) {
    int n = 0;
    int started = 0;
    while (**p >= '0' && **p <= '9') {
        started = 1;
        int d = **p - '0';
        if (n > (2147483647 - d) / 10) {
            *ok = 0;
            return 0;
        }
        n = n * 10 + d;
        (*p)++;
    }
    if (!started) *ok = 0;
    return n;
}

zenit_result_t zenit_semver_parse(const char *str, zenit_semver_t *out) {
    if (str == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Zero out the output struct */
    memset(out, 0, sizeof(zenit_semver_t));

    const char *p = str;
    int ok = 1;

    /* Parse major */
    out->major = parse_num(&p, &ok);
    if (!ok || *p != '.') return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    p++;

    /* Parse minor */
    out->minor = parse_num(&p, &ok);
    if (!ok || *p != '.') return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    p++;

    /* Parse patch */
    out->patch = parse_num(&p, &ok);
    if (!ok) return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);

    /* Leading zeros not allowed (starts with '0' and has more digits) */
    if (str[0] == '0' && str[1] >= '0' && str[1] <= '9') {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Parse pre-release (-) */
    if (*p == '-') {
        p++;
        size_t i = 0;
        while (*p != '\0' && *p != '+') {
            if (i >= ZENIT_SEMVER_MAX_PRERELEASE - 1) {
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
            }
            out->pre_release[i++] = *p;
            p++;
        }
        out->pre_release[i] = '\0';
        if (i == 0) return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    /* Parse build metadata (+) */
    if (*p == '+') {
        p++;
        size_t i = 0;
        while (*p != '\0') {
            if (i >= ZENIT_SEMVER_MAX_BUILD - 1) {
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
            }
            out->build[i++] = *p;
            p++;
        }
        out->build[i] = '\0';
        if (i == 0) return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    if (*p != '\0') {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_PARAM);
    }

    return ZENIT_RESULT_OK;
}

static char* format_impl(const zenit_semver_t *v, zenit_allocator_t a) {
    if (v == NULL) return NULL;

    /* Calculate required length */
    /* major.minor.patch = up to 3×10 digits + 2 dots = 32, plus prerelease and build */
    size_t base_len = 32;
    size_t pre_len = strlen(v->pre_release);
    size_t build_len = strlen(v->build);
    size_t total = base_len + (pre_len > 0 ? 1 + pre_len : 0) + (build_len > 0 ? 1 + build_len : 0) + 1;

    char *out = a.alloc_fn(total, a.ctx);
    if (out == NULL) return NULL;

    int written;
    if (pre_len > 0 && build_len > 0) {
        written = snprintf(out, total, "%d.%d.%d-%s+%s", v->major, v->minor, v->patch, v->pre_release, v->build);
    } else if (pre_len > 0) {
        written = snprintf(out, total, "%d.%d.%d-%s", v->major, v->minor, v->patch, v->pre_release);
    } else if (build_len > 0) {
        written = snprintf(out, total, "%d.%d.%d+%s", v->major, v->minor, v->patch, v->build);
    } else {
        written = snprintf(out, total, "%d.%d.%d", v->major, v->minor, v->patch);
    }

    if (written < 0 || (size_t)written >= total) {
        a.free_fn(out, a.ctx);
        return NULL;
    }

    return out;
}

zenit_result_t zenit_semver_format(const zenit_semver_t *v, char **out) {
    return zenit_semver_format_with_allocator(v, out, ZENIT_ALLOCATOR_DEFAULT);
}

zenit_result_t zenit_semver_format_with_allocator(const zenit_semver_t *v, char **out, zenit_allocator_t allocator) {
    if (v == NULL || out == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    char *s = format_impl(v, allocator);
    if (s == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    *out = s;
    return ZENIT_RESULT_OK;
}

/* Compare two pre-release identifiers.  Returns negative, zero, or positive. */
static int cmp_pre_id(const char *a_start, size_t a_len, const char *b_start, size_t b_len) {
    /* Check if both are numeric */
    int a_num = 1;
    int b_num = 1;
    for (size_t i = 0; i < a_len; i++) {
        if (a_start[i] < '0' || a_start[i] > '9') { a_num = 0; break; }
    }
    for (size_t i = 0; i < b_len; i++) {
        if (b_start[i] < '0' || b_start[i] > '9') { b_num = 0; break; }
    }

    if (a_num && b_num) {
        /* Numeric comparison */
        int a_val = 0;
        int b_val = 0;
        for (size_t i = 0; i < a_len; i++) a_val = a_val * 10 + (a_start[i] - '0');
        for (size_t i = 0; i < b_len; i++) b_val = b_val * 10 + (b_start[i] - '0');
        if (a_val < b_val) return -1;
        if (a_val > b_val) return 1;
        return 0;
    }

    /* Lexicographic ASCII comparison */
    size_t min_len = a_len < b_len ? a_len : b_len;
    for (size_t i = 0; i < min_len; i++) {
        if (a_start[i] < b_start[i]) return -1;
        if (a_start[i] > b_start[i]) return 1;
    }
    if (a_len < b_len) return -1;
    if (a_len > b_len) return 1;
    return 0;
}

/* Extract the next dot-separated identifier from *sp.
   Sets *out_start, *out_len and advances *sp past the dot. */
static int next_ident(const char **sp, const char **out_start, size_t *out_len) {
    if (**sp == '\0') return 0;
    const char *dot = NULL;
    for (const char *s = *sp; *s; s++) {
        if (*s == '.') { dot = s; break; }
    }
    *out_start = *sp;
    *out_len = dot ? (size_t)(dot - *sp) : strlen(*sp);
    *sp = dot ? dot + 1 : *sp + *out_len;
    return 1;
}

/* Compare pre-release strings per SemVer 2.0.0 precedence rules. */
static int cmp_prerelease(const char *a, const char *b) {
    for (;;) {
        const char *a_start;
        const char *b_start;
        size_t a_len;
        size_t b_len;
        int has_a = next_ident(&a, &a_start, &a_len);
        int has_b = next_ident(&b, &b_start, &b_len);
        if (has_a != has_b) return has_a ? 1 : -1;
        if (!has_a) return 0;
        int c = cmp_pre_id(a_start, a_len, b_start, b_len);
        if (c != 0) return c;
    }
}

int zenit_semver_compare(const zenit_semver_t *a, const zenit_semver_t *b) {
    if (a == NULL && b == NULL) return 0;
    if (a == NULL) return -1;
    if (b == NULL) return 1;

    if (a->major != b->major) return a->major - b->major;
    if (a->minor != b->minor) return a->minor - b->minor;
    if (a->patch != b->patch) return a->patch - b->patch;

    return cmp_prerelease(a->pre_release, b->pre_release);
}
