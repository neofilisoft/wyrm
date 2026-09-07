/*
 * wyrm_std_time.c - Wyrm Standard Library: High-Resolution Time Subsystem
 *
 * Implements high-resolution monotonic timers using Win32 QueryPerformanceCounter
 * on Windows and clock_gettime(CLOCK_MONOTONIC) on POSIX platforms.
 * Provides microsecond-precision wall-clock time, thread sleeping, and strftime formatting.
 */

#include "wyrm_std_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

/* ==========================================================================
 * Monotonic Clock Internal State
 * ========================================================================== */

#if defined(_WIN32) || defined(_WIN64)
static LARGE_INTEGER g_qpc_frequency = { 0 };
static LARGE_INTEGER g_qpc_start = { 0 };
static bool g_time_initialized = false;

static void ensure_time_init(void) {
    if (!g_time_initialized) {
        QueryPerformanceFrequency(&g_qpc_frequency);
        QueryPerformanceCounter(&g_qpc_start);
        g_time_initialized = true;
    }
}
#else
static struct timespec g_posix_start = { 0, 0 };
static bool g_time_initialized = false;

static void ensure_time_init(void) {
    if (!g_time_initialized) {
        clock_gettime(CLOCK_MONOTONIC, &g_posix_start);
        g_time_initialized = true;
    }
}
#endif

void wyrm_time_init(void) {
    ensure_time_init();
}

/* ==========================================================================
 * Wall-Clock Timestamp Implementations
 * ========================================================================== */

/* Returns fractional seconds since Unix epoch with high resolution */
Value time_now(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    /* 100-nanosecond intervals between 1601-01-01 and 1970-01-01 */
    const uint64_t epoch_diff = 116444736000000000ULL;
    if (uli.QuadPart >= epoch_diff) {
        uint64_t unix_100ns = uli.QuadPart - epoch_diff;
        double secs = (double)unix_100ns / 10000000.0;
        return val_number(secs);
    }
    return val_number((double)time(NULL));
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        double secs = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
        return val_number(secs);
    }
    return val_number((double)time(NULL));
#endif
}

/* Returns integer Unix epoch timestamp in seconds */
Value time_unix(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    const uint64_t epoch_diff = 116444736000000000ULL;
    if (uli.QuadPart >= epoch_diff) {
        uint64_t unix_sec = (uli.QuadPart - epoch_diff) / 10000000ULL;
        return val_number((double)unix_sec);
    }
    return val_number((double)time(NULL));
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        return val_number((double)ts.tv_sec);
    }
    return val_number((double)time(NULL));
#endif
}

/* Returns integer Unix epoch timestamp in milliseconds */
Value time_unix_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    const uint64_t epoch_diff = 116444736000000000ULL;
    if (uli.QuadPart >= epoch_diff) {
        uint64_t unix_ms = (uli.QuadPart - epoch_diff) / 10000ULL;
        return val_number((double)unix_ms);
    }
    return val_number((double)time(NULL) * 1000.0);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        double ms = ((double)ts.tv_sec * 1000.0) + ((double)ts.tv_nsec / 1000000.0);
        return val_number(ms);
    }
    return val_number((double)time(NULL) * 1000.0);
#endif
}

/* ==========================================================================
 * Monotonic High-Resolution Timers
 * ========================================================================== */

/* Returns monotonic elapsed seconds as double */
Value time_monotonic(void) {
    ensure_time_init();
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double elapsed = (double)(now.QuadPart - g_qpc_start.QuadPart) / (double)g_qpc_frequency.QuadPart;
    return val_number(elapsed);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (double)(now.tv_sec - g_posix_start.tv_sec) +
                     ((double)(now.tv_nsec - g_posix_start.tv_nsec) / 1000000000.0);
    return val_number(elapsed);
#endif
}

/* Returns monotonic elapsed milliseconds as double */
Value time_monotonic_ms(void) {
    ensure_time_init();
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double ms = ((double)(now.QuadPart - g_qpc_start.QuadPart) * 1000.0) / (double)g_qpc_frequency.QuadPart;
    return val_number(ms);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double ms = ((double)(now.tv_sec - g_posix_start.tv_sec) * 1000.0) +
                ((double)(now.tv_nsec - g_posix_start.tv_nsec) / 1000000.0);
    return val_number(ms);
#endif
}

/* Returns monotonic elapsed nanoseconds as double */
Value time_monotonic_ns(void) {
    ensure_time_init();
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double ns = ((double)(now.QuadPart - g_qpc_start.QuadPart) * 1000000000.0) / (double)g_qpc_frequency.QuadPart;
    return val_number(ns);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double ns = ((double)(now.tv_sec - g_posix_start.tv_sec) * 1000000000.0) +
                (double)(now.tv_nsec - g_posix_start.tv_nsec);
    return val_number(ns);
#endif
}

/* ==========================================================================
 * Sleep and Time Arithmetic
 * ========================================================================== */

/* Suspends current thread execution for specified milliseconds */
Value time_sleep_ms(Value ms_val) {
    double ms = 0.0;
    if (ms_val.type == VAL_NUMBER) {
        ms = ms_val.as.number;
    }
    if (ms > 0.0) {
#if defined(_WIN32) || defined(_WIN64)
        DWORD sleep_ms = (DWORD)ms;
        Sleep(sleep_ms);
#else
        struct timespec req;
        req.tv_sec = (time_t)(ms / 1000.0);
        req.tv_nsec = (long)((ms - (double)req.tv_sec * 1000.0) * 1000000.0);
        nanosleep(&req, NULL);
#endif
    }
    return val_null();
}

/* Calculates difference in seconds: (end - start) */
Value time_diff(Value start_val, Value end_val) {
    double start = 0.0;
    double end = 0.0;
    if (start_val.type == VAL_NUMBER) { start = start_val.as.number; }
    if (end_val.type == VAL_NUMBER) { end = end_val.as.number; }
    return val_number(end - start);
}

/* ==========================================================================
 * Date/Time Formatting
 * ========================================================================== */

/* Formats timestamp (seconds) into UTC string using strftime */
Value time_format_utc(Value ts_val, Value fmt_val) {
    time_t raw_time;
    if (ts_val.type == VAL_NUMBER) {
        raw_time = (time_t)ts_val.as.number;
    } else {
        raw_time = time(NULL);
    }

    const char *format = "%Y-%m-%d %H:%M:%S";
    if (fmt_val.type == VAL_STRING && fmt_val.as.string && strlen(fmt_val.as.string) > 0) {
        format = fmt_val.as.string;
    }

    struct tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_buf, &raw_time);
#else
    gmtime_r(&raw_time, &tm_buf);
#endif

    char out_buf[256];
    size_t written = strftime(out_buf, sizeof(out_buf), format, &tm_buf);
    if (written == 0) {
        out_buf[0] = '\0';
    }
    return val_string(out_buf);
}

/* Formats timestamp (seconds) into local timezone string using strftime */
Value time_format_local(Value ts_val, Value fmt_val) {
    time_t raw_time;
    if (ts_val.type == VAL_NUMBER) {
        raw_time = (time_t)ts_val.as.number;
    } else {
        raw_time = time(NULL);
    }

    const char *format = "%Y-%m-%d %H:%M:%S";
    if (fmt_val.type == VAL_STRING && fmt_val.as.string && strlen(fmt_val.as.string) > 0) {
        format = fmt_val.as.string;
    }

    struct tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &raw_time);
#else
    localtime_r(&raw_time, &tm_buf);
#endif

    char out_buf[256];
    size_t written = strftime(out_buf, sizeof(out_buf), format, &tm_buf);
    if (written == 0) {
        out_buf[0] = '\0';
    }
    return val_string(out_buf);
}
