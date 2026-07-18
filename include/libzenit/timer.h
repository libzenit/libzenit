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

#ifndef LIBZENIT_TIMER_H
#define LIBZENIT_TIMER_H

#include <stddef.h>

/**
 * @brief A high-resolution time point.
 *
 * Stores a time value as seconds and nanoseconds since an unspecified epoch
 * (typically CLOCK_MONOTONIC).  The epoch is consistent within a single
 * process lifetime — use zenit_time_now() to capture start and end points,
 * then compute differences with zenit_time_elapsed_*().
 */
typedef struct {
    long long sec;  /**< Seconds component. */
    long long nsec; /**< Nanoseconds component (0 … 999999999). */
} zenit_time_t;

/**
 * @brief Capture the current time.
 *
 * Uses CLOCK_MONOTONIC (POSIX) or QueryPerformanceCounter (Windows).
 * The returned value represents a point on a monotonic timeline — it is
 * meaningful only for computing durations with zenit_time_elapsed_*().
 *
 * @return The current time point.
 */
zenit_time_t zenit_time_now(void);

/**
 * @brief Compute the elapsed wall-clock time in seconds.
 *
 * @param start Start time point (captured earlier via zenit_time_now()).
 * @param end   End time point (captured later via zenit_time_now()).
 * @return Elapsed time in seconds (end - start).  May be negative if
 *         @p end is earlier than @p start (should not happen with
 *         monotonic clocks but no check is performed).
 */
double zenit_time_elapsed_s(zenit_time_t start, zenit_time_t end);

/**
 * @brief Compute the elapsed wall-clock time in milliseconds.
 *
 * @param start Start time point.
 * @param end   End time point.
 * @return Elapsed time in milliseconds.
 */
double zenit_time_elapsed_ms(zenit_time_t start, zenit_time_t end);

/**
 * @brief Compute the elapsed wall-clock time in microseconds.
 *
 * @param start Start time point.
 * @param end   End time point.
 * @return Elapsed time in microseconds.
 */
double zenit_time_elapsed_us(zenit_time_t start, zenit_time_t end);

/**
 * @brief Compute the elapsed wall-clock time in nanoseconds.
 *
 * @param start Start time point.
 * @param end   End time point.
 * @return Elapsed time in nanoseconds.
 */
double zenit_time_elapsed_ns(zenit_time_t start, zenit_time_t end);

/**
 * @brief Add two time points.
 *
 * @param a First time point.
 * @param b Second time point.
 * @return a + b with nanosecond carry normalisation.
 */
zenit_time_t zenit_time_add(zenit_time_t a, zenit_time_t b);

/**
 * @brief Subtract two time points.
 *
 * @param a First time point.
 * @param b Second time point.
 * @return a - b with nanosecond borrow normalisation.
 */
zenit_time_t zenit_time_sub(zenit_time_t a, zenit_time_t b);

/**
 * @brief Compare two time points.
 *
 * @param a First time point.
 * @param b Second time point.
 * @return Negative if a < b, zero if a == b, positive if a > b.
 */
int zenit_time_cmp(zenit_time_t a, zenit_time_t b);

#endif
