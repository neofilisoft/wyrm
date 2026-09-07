/*
 * wyrm_std_time.h - Wyrm Standard Library: High-Resolution Time Subsystem
 *
 * Provides high-resolution monotonic timers (Windows QPC / POSIX CLOCK_MONOTONIC),
 * Unix epoch wall-clock timestamps, millisecond thread sleep, and date/time formatting.
 *
 * Wyrm builtins provided (registered when `use std.time;` is encountered):
 *   time_now()                  -> VAL_NUMBER (floating-point seconds since Unix epoch)
 *   time_unix()                 -> VAL_NUMBER (integer seconds since Unix epoch)
 *   time_unix_ms()              -> VAL_NUMBER (integer milliseconds since Unix epoch)
 *   time_monotonic()            -> VAL_NUMBER (high-precision monotonic seconds)
 *   time_monotonic_ms()         -> VAL_NUMBER (high-precision monotonic milliseconds)
 *   time_monotonic_ns()         -> VAL_NUMBER (high-precision monotonic nanoseconds)
 *   time_sleep(ms)              -> VAL_NULL (sleeps current thread for ms milliseconds)
 *   time_diff(start, end)       -> VAL_NUMBER (end - start in seconds)
 *   time_format(ts, fmt)        -> VAL_STRING (formatted UTC date/time string)
 *   time_format_local(ts, fmt)  -> VAL_STRING (formatted local date/time string)
 */
#ifndef WYRM_STD_TIME_H
#define WYRM_STD_TIME_H

#include "../wyrm_core.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize time subsystem (e.g. QPC frequency caching) */
void wyrm_time_init(void);

/* Wall-clock timestamps */
Value time_now(void);
Value time_unix(void);
Value time_unix_ms(void);

/* High-resolution monotonic timers */
Value time_monotonic(void);
Value time_monotonic_ms(void);
Value time_monotonic_ns(void);

/* Execution control & utility */
Value time_sleep_ms(Value ms_val);
Value time_diff(Value start_val, Value end_val);

/* Formatting */
Value time_format_utc(Value ts_val, Value fmt_val);
Value time_format_local(Value ts_val, Value fmt_val);

#ifdef __cplusplus
}
#endif

#endif /* WYRM_STD_TIME_H */
