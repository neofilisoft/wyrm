/*
 * wyrm_std_collections.h - Wyrm Standard Library: Collections
 *
 * HashMap and Set implementations over Wyrm Values.
 * Keys are VAL_STRING only for v2.6.0 (covers 95% of real use cases).
 *
 * HashMap is stored as a VAL_RAW_PTR(WyrmMap).
 * Set is stored as a VAL_RAW_PTR(WyrmSet).
 */
#ifndef WYRM_STD_COLLECTIONS_H
#define WYRM_STD_COLLECTIONS_H

#include "../wyrm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
 * HashMap: string -> Value
 * -------------------------------------------------------------------------- */

/* Create an empty map. Returns VAL_RAW_PTR(WyrmMap). */
Value map_new(void);

/* Set key (string) to value. Returns the map handle (same pointer). */
Value map_set(Value map_val, Value key, Value val);

/* Get value for key. Returns VAL_NULL if key not found. */
Value map_get(Value map_val, Value key);

/* Returns VAL_BOOL true if key exists. */
Value map_has(Value map_val, Value key);

/* Delete key. Returns VAL_NULL. */
Value map_del(Value map_val, Value key);

/* Returns VAL_ARRAY of all keys (strings). */
Value map_keys(Value map_val);

/* Returns VAL_ARRAY of all values. */
Value map_values(Value map_val);

/* Returns VAL_NUMBER of entries. */
Value map_len(Value map_val);

/* Free a map created by map_new. */
Value map_free(Value map_val);

/* --------------------------------------------------------------------------
 * Set: string values (unordered, unique)
 * -------------------------------------------------------------------------- */

/* Create an empty set. Returns VAL_RAW_PTR(WyrmSet). */
Value set_new(void);

/* Add value to set. Returns the set handle. */
Value set_add(Value set_val, Value item);

/* Returns VAL_BOOL true if item is in set. */
Value set_has(Value set_val, Value item);

/* Remove item from set. Returns VAL_NULL. */
Value set_del(Value set_val, Value item);

/* Union of two sets. Returns a new set. */
Value set_union_fn(Value a, Value b);

/* Intersection of two sets. Returns a new set. */
Value set_intersect(Value a, Value b);

/* Convert set to a VAL_ARRAY. */
Value set_to_array(Value set_val);

/* Free a set. */
Value set_free(Value set_val);

#ifdef __cplusplus
}
#endif

#endif /* WYRM_STD_COLLECTIONS_H */
