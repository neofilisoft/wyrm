/*
 * wyrm_std_random.h - Wyrm Standard Library: Random Number Generation
 *
 * Provides PRNG (Xoshiro256**), CSPRNG (OS Cryptographic entropy),
 * and TRNG (CPU Hardware RDRAND / RDSEED with OS entropy fallback).
 *
 * Wyrm builtins provided (registered when `use std.random;` is encountered):
 *   rand_seed(val)              -> VAL_NULL (Seeds PRNG; auto-seeds if null)
 *   rand()                      -> VAL_NUMBER [0.0, 1.0)
 *   rand_int(min, max)          -> VAL_NUMBER integer in [min, max] inclusive
 *   rand_range(min, max)        -> VAL_NUMBER integer in [min, max] inclusive
 *   rand_choice(array)          -> Value (Random element from array)
 *   rand_shuffle(array)         -> VAL_ARRAY (New shuffled copy via Fisher-Yates)
 *   rand_secure()               -> VAL_NUMBER [0.0, 1.0) (Cryptographically secure)
 *   rand_secure_int(min, max)   -> VAL_NUMBER unbiased integer in [min, max]
 *   rand_bytes_hex(count)       -> VAL_STRING hex string of random bytes
 *   rand_has_trng()             -> VAL_BOOL true if CPU RDRAND is supported
 *   rand_trng()                 -> VAL_NUMBER [0.0, 1.0) via hardware TRNG
 *   rand_trng_int(min, max)     -> VAL_NUMBER integer in [min, max] via TRNG
 *   rand_reseed_trng()          -> VAL_NULL (Reseeds PRNG state from TRNG)
 */
#ifndef WYRM_STD_RANDOM_H
#define WYRM_STD_RANDOM_H

#include "../wyrm_core.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize random subsystem */
void wyrm_random_init(void);

/* PRNG functions */
Value rand_seed(Value seed_val);
Value rand_val(void);
Value rand_int(Value min_val, Value max_val);
Value rand_range(Value min_val, Value max_val);
Value rand_choice(Value arr_val);
Value rand_shuffle(Value arr_val);

/* CSPRNG functions */
Value rand_secure(void);
Value rand_secure_int(Value min_val, Value max_val);
Value rand_bytes_hex(Value count_val);

/* TRNG functions */
Value rand_has_trng(void);
Value rand_trng(void);
Value rand_trng_int(Value min_val, Value max_val);
Value rand_reseed_trng(void);

#ifdef __cplusplus
}
#endif

#endif /* WYRM_STD_RANDOM_H */
