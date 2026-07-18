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

#include <libzenit/timer.h>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <time.h>
#endif

zenit_time_t zenit_time_now(void) {
    zenit_time_t t;
#if defined(_WIN32)
    LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    long long total_ns = (counter.QuadPart * 1000000000LL) / freq.QuadPart;
    t.sec = total_ns / 1000000000LL;
    t.nsec = total_ns % 1000000000LL;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t.sec = (long long)ts.tv_sec;
    t.nsec = (long long)ts.tv_nsec;
#endif
    return t;
}

static long long to_ns(zenit_time_t t) {
    return t.sec * 1000000000LL + t.nsec;
}

double zenit_time_elapsed_s(zenit_time_t start, zenit_time_t end) {
    return (double)(to_ns(end) - to_ns(start)) / 1.0e9;
}

double zenit_time_elapsed_ms(zenit_time_t start, zenit_time_t end) {
    return (double)(to_ns(end) - to_ns(start)) / 1.0e6;
}

double zenit_time_elapsed_us(zenit_time_t start, zenit_time_t end) {
    return (double)(to_ns(end) - to_ns(start)) / 1.0e3;
}

double zenit_time_elapsed_ns(zenit_time_t start, zenit_time_t end) {
    return (double)(to_ns(end) - to_ns(start));
}

zenit_time_t zenit_time_add(zenit_time_t a, zenit_time_t b) {
    zenit_time_t r;
    r.sec = a.sec + b.sec;
    r.nsec = a.nsec + b.nsec;
    if (r.nsec >= 1000000000LL) {
        r.sec += 1;
        r.nsec -= 1000000000LL;
    }
    return r;
}

zenit_time_t zenit_time_sub(zenit_time_t a, zenit_time_t b) {
    zenit_time_t r;
    r.sec = a.sec - b.sec;
    r.nsec = a.nsec - b.nsec;
    if (r.nsec < 0) {
        r.sec -= 1;
        r.nsec += 1000000000LL;
    }
    return r;
}

int zenit_time_cmp(zenit_time_t a, zenit_time_t b) {
    if (a.sec < b.sec) return -1;
    if (a.sec > b.sec) return 1;
    if (a.nsec < b.nsec) return -1;
    if (a.nsec > b.nsec) return 1;
    return 0;
}
