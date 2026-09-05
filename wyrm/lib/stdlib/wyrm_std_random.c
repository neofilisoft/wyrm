/*
 * wyrm_std_random.c - Wyrm Standard Library: Random Number Generation
 *
 * Implements PRNG (Xoshiro256**), CSPRNG (OS Cryptographic API),
 * and TRNG (CPU Hardware RDRAND with OS entropy fallback).
 */

#include "wyrm_std_random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define WYRM_ARCH_X86 1
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#include <immintrin.h>
#endif
#endif

/* ==========================================================================
 * PRNG: Xoshiro256** Generator & SplitMix64 Seeder
 * ========================================================================== */

static uint64_t prng_state[4] = {
    0x853c49e6748fea9bULL,
    0xda3e39cb94b95bdbULL,
    0x9e3779b97f4a7c15ULL,
    0xbf58476d1ce4e5b9ULL
};
static bool prng_initialized = false;

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64_next(uint64_t *x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static void xoshiro256starstar_seed(uint64_t seed) {
    uint64_t sm_state = seed;
    prng_state[0] = splitmix64_next(&sm_state);
    prng_state[1] = splitmix64_next(&sm_state);
    prng_state[2] = splitmix64_next(&sm_state);
    prng_state[3] = splitmix64_next(&sm_state);
    /* Ensure non-zero state */
    if (prng_state[0] == 0 && prng_state[1] == 0 &&
        prng_state[2] == 0 && prng_state[3] == 0) {
        prng_state[0] = 0x853c49e6748fea9bULL;
    }
    prng_initialized = true;
}

static uint64_t xoshiro256starstar_next(void) {
    const uint64_t result = rotl(prng_state[1] * 5, 7) * 9;
    const uint64_t t = prng_state[1] << 17;

    prng_state[2] ^= prng_state[0];
    prng_state[3] ^= prng_state[1];
    prng_state[1] ^= prng_state[2];
    prng_state[0] ^= prng_state[3];

    prng_state[2] ^= t;
    prng_state[3] = rotl(prng_state[3], 45);

    return result;
}

/* ==========================================================================
 * CSPRNG: OS Cryptographic Entropy
 * ========================================================================== */

#if defined(_WIN32) || defined(_WIN64)
typedef LONG (WINAPI *pfn_BCryptGenRandom)(void*, unsigned char*, unsigned long, unsigned long);
static pfn_BCryptGenRandom g_bcrypt_gen_random = NULL;
static bool g_bcrypt_loaded = false;

static bool os_crypto_random_bytes(void *buf, size_t len) {
    if (!g_bcrypt_loaded) {
        HMODULE hBcrypt = LoadLibraryA("bcrypt.dll");
        if (hBcrypt) {
            g_bcrypt_gen_random = (pfn_BCryptGenRandom)(void*)GetProcAddress(hBcrypt, "BCryptGenRandom");
        }
        g_bcrypt_loaded = true;
    }
    if (g_bcrypt_gen_random) {
        /* BCRYPT_USE_SYSTEM_PREFERRED_RNG = 0x00000002 */
        LONG status = g_bcrypt_gen_random(NULL, (unsigned char*)buf, (unsigned long)len, 0x00000002);
        if (status >= 0) return true;
    }

    /* Fallback to Advapi32 CryptGenRandom */
    HMODULE hAdvapi = LoadLibraryA("advapi32.dll");
    if (hAdvapi) {
        typedef BOOL (WINAPI *pfn_CryptAcquireContextA)(uintptr_t*, LPCSTR, LPCSTR, DWORD, DWORD);
        typedef BOOL (WINAPI *pfn_CryptGenRandom)(uintptr_t, DWORD, BYTE*);
        typedef BOOL (WINAPI *pfn_CryptReleaseContext)(uintptr_t, DWORD);

        pfn_CryptAcquireContextA pAcquire = (pfn_CryptAcquireContextA)(void*)GetProcAddress(hAdvapi, "CryptAcquireContextA");
        pfn_CryptGenRandom pGen = (pfn_CryptGenRandom)(void*)GetProcAddress(hAdvapi, "CryptGenRandom");
        pfn_CryptReleaseContext pRel = (pfn_CryptReleaseContext)(void*)GetProcAddress(hAdvapi, "CryptReleaseContext");

        if (pAcquire && pGen && pRel) {
            uintptr_t hProv = 0;
            if (pAcquire(&hProv, NULL, NULL, 1 /* PROV_RSA_FULL */, 0xF0000000 /* CRYPT_VERIFYCONTEXT */ | 0x00000040 /* CRYPT_SILENT */)) {
                BOOL ok = pGen(hProv, (DWORD)len, (BYTE*)buf);
                pRel(hProv, 0);
                if (ok) return true;
            }
        }
    }
    return false;
}
#else
static bool os_crypto_random_bytes(void *buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, (char*)buf + total, len - total);
        if (n <= 0) {
            close(fd);
            return false;
        }
        total += (size_t)n;
    }
    close(fd);
    return true;
}
#endif

/* ==========================================================================
 * TRNG: CPU Hardware RDRAND with OS entropy fallback
 * ========================================================================== */

static inline void wyrm_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) && defined(__PIC__)
    __asm__ volatile("xchgl %%ebx, %1\n\t"
                     "cpuid\n\t"
                     "xchgl %%ebx, %1\n\t"
                     : "=a"(*eax), "=r"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(0));
    #else
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(0));
    #endif
#elif defined(_MSC_VER) && !defined(__clang__)
    int info[4];
    __cpuid(info, (int)leaf);
    *eax = (unsigned int)info[0];
    *ebx = (unsigned int)info[1];
    *ecx = (unsigned int)info[2];
    *edx = (unsigned int)info[3];
#else
    *eax = *ebx = *ecx = *edx = 0;
#endif
}

static bool check_cpu_rdrand_support(void) {
#if defined(WYRM_ARCH_X86)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    wyrm_cpuid(1, &eax, &ebx, &ecx, &edx);
    return (ecx & (1U << 30)) != 0;
#else
    return false;
#endif
}

static bool hardware_rdrand64(uint64_t *out_val) {
#if defined(WYRM_ARCH_X86) && (defined(__x86_64__) || defined(_M_X64))
    if (!check_cpu_rdrand_support()) return false;
    #if defined(__GNUC__) || defined(__clang__)
    for (int i = 0; i < 10; i++) {
        unsigned char ok;
        uint64_t v;
        __asm__ volatile(".byte 0x48, 0x0f, 0xc7, 0xf0\n\t"
                         "setc %1"
                         : "=a"(v), "=qm"(ok)
                         :
                         : "cc");
        if (ok) {
            *out_val = v;
            return true;
        }
    }
    return false;
    #elif defined(_MSC_VER) && !defined(__clang__)
    for (int i = 0; i < 10; i++) {
        if (_rdrand64_step(out_val)) return true;
    }
    return false;
    #else
    return false;
    #endif
#else
    (void)out_val;
    return false;
#endif
}

static uint64_t trng_next_u64(void) {
    uint64_t val = 0;
    if (hardware_rdrand64(&val)) {
        return val;
    }
    /* Fallback to OS cryptographic entropy pool */
    if (os_crypto_random_bytes(&val, sizeof(val))) {
        return val;
    }
    /* Final fallback to high-resolution timestamp jitter */
    uint64_t t = (uint64_t)time(NULL);
    return splitmix64_next(&t);
}

/* ==========================================================================
 * Subsystem Initialization
 * ========================================================================== */

void wyrm_random_init(void) {
    if (prng_initialized) return;
    uint64_t seed = trng_next_u64();
    xoshiro256starstar_seed(seed);
}

/* ==========================================================================
 * PRNG Wyrm Builtin Implementations
 * ========================================================================== */

Value rand_seed(Value seed_val) {
    uint64_t seed = 0;
    if (seed_val.type == VAL_NUMBER) {
        seed = (uint64_t)seed_val.as.number;
    } else if (seed_val.type == VAL_NULL) {
        seed = trng_next_u64();
    } else {
        seed = (uint64_t)time(NULL);
    }
    xoshiro256starstar_seed(seed);
    return val_null();
}

Value rand_val(void) {
    if (!prng_initialized) wyrm_random_init();
    uint64_t raw = xoshiro256starstar_next();
    /* 53-bit precision double float in [0.0, 1.0) */
    double d = (double)(raw >> 11) * (1.0 / 9007199254740992.0);
    return val_number(d);
}

static int64_t compute_rand_int(int64_t lo, int64_t hi, uint64_t (*gen_u64)(void)) {
    if (lo > hi) {
        int64_t tmp = lo;
        lo = hi;
        hi = tmp;
    }
    uint64_t range = (uint64_t)(hi - lo + 1);
    if (range == 0) {
        return lo;
    }
    /* Rejection sampling to eliminate modulo bias */
    uint64_t threshold = (0ULL - range) % range;
    uint64_t r;
    do {
        r = gen_u64();
    } while (r < threshold);

    return lo + (int64_t)(r % range);
}

Value rand_int(Value min_val, Value max_val) {
    if (!prng_initialized) wyrm_random_init();
    int64_t lo = (min_val.type == VAL_NUMBER) ? (int64_t)min_val.as.number : 0;
    int64_t hi = (max_val.type == VAL_NUMBER) ? (int64_t)max_val.as.number : 100;
    int64_t result = compute_rand_int(lo, hi, xoshiro256starstar_next);
    return val_number((double)result);
}

Value rand_range(Value min_val, Value max_val) {
    return rand_int(min_val, max_val);
}

Value rand_choice(Value arr_val) {
    if (arr_val.type != VAL_ARRAY || !arr_val.as.array || arr_val.as.array->size == 0) {
        return val_null();
    }
    int size = arr_val.as.array->size;
    int64_t idx = compute_rand_int(0, size - 1, xoshiro256starstar_next);
    return arr_val.as.array->data[idx];
}

Value rand_shuffle(Value arr_val) {
    if (arr_val.type != VAL_ARRAY || !arr_val.as.array) {
        return arr_val;
    }
    int count = arr_val.as.array->size;
    Value result = val_array_create(count);
    if (count == 0) {
        return result;
    }
    for (int i = 0; i < count; i++) {
        result.as.array->data[i] = arr_val.as.array->data[i];
    }
    /* Fisher-Yates shuffle */
    for (int i = count - 1; i > 0; i--) {
        int64_t j = compute_rand_int(0, i, xoshiro256starstar_next);
        Value temp = result.as.array->data[i];
        result.as.array->data[i] = result.as.array->data[j];
        result.as.array->data[j] = temp;
    }
    return result;
}

/* ==========================================================================
 * CSPRNG Wyrm Builtin Implementations
 * ========================================================================== */

static uint64_t crypto_next_u64(void) {
    uint64_t val = 0;
    if (!os_crypto_random_bytes(&val, sizeof(val))) {
        val = trng_next_u64();
    }
    return val;
}

Value rand_secure(void) {
    uint64_t raw = crypto_next_u64();
    double d = (double)(raw >> 11) * (1.0 / 9007199254740992.0);
    return val_number(d);
}

Value rand_secure_int(Value min_val, Value max_val) {
    int64_t lo = (min_val.type == VAL_NUMBER) ? (int64_t)min_val.as.number : 0;
    int64_t hi = (max_val.type == VAL_NUMBER) ? (int64_t)max_val.as.number : 100;
    int64_t result = compute_rand_int(lo, hi, crypto_next_u64);
    return val_number((double)result);
}

Value rand_bytes_hex(Value count_val) {
    int n = (count_val.type == VAL_NUMBER) ? (int)count_val.as.number : 16;
    if (n <= 0) return val_string("");
    if (n > 65536) n = 65536;

    unsigned char *raw = (unsigned char*)malloc((size_t)n);
    if (!raw) return val_error("Out of memory in rand_bytes_hex");

    if (!os_crypto_random_bytes(raw, (size_t)n)) {
        for (int i = 0; i < n; i++) {
            raw[i] = (unsigned char)(trng_next_u64() & 0xFF);
        }
    }

    char *hex = (char*)malloc((size_t)n * 2 + 1);
    if (!hex) {
        free(raw);
        return val_error("Out of memory in rand_bytes_hex");
    }

    static const char hex_digits[] = "0123456789abcdef";
    for (int i = 0; i < n; i++) {
        hex[i * 2]     = hex_digits[(raw[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_digits[raw[i] & 0x0F];
    }
    hex[n * 2] = '\0';
    free(raw);

    Value v = val_string(hex);
    free(hex);
    return v;
}

/* ==========================================================================
 * TRNG Wyrm Builtin Implementations
 * ========================================================================== */

Value rand_has_trng(void) {
    return val_bool(check_cpu_rdrand_support());
}

Value rand_trng(void) {
    uint64_t raw = trng_next_u64();
    double d = (double)(raw >> 11) * (1.0 / 9007199254740992.0);
    return val_number(d);
}

Value rand_trng_int(Value min_val, Value max_val) {
    int64_t lo = (min_val.type == VAL_NUMBER) ? (int64_t)min_val.as.number : 0;
    int64_t hi = (max_val.type == VAL_NUMBER) ? (int64_t)max_val.as.number : 100;
    int64_t result = compute_rand_int(lo, hi, trng_next_u64);
    return val_number((double)result);
}

Value rand_reseed_trng(void) {
    uint64_t seed = trng_next_u64();
    xoshiro256starstar_seed(seed);
    return val_null();
}
