/*
 * wyrm_std_json.h - Wyrm Standard Library: JSON Serialization
 *
 * Pure C11 recursive-descent JSON parser and encoder.
 * No external dependencies.
 *
 * Wyrm builtins provided (registered when `use std.json` is encountered):
 *   json_parse(str)             -> Value (array/map represented as nested arrays)
 *   json_encode(val)            -> string
 *   json_pretty(val, indent)    -> string (pretty-printed with indent spaces)
 */
#ifndef WYRM_STD_JSON_H
#define WYRM_STD_JSON_H

#include "../wyrm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * JSON Value Representation in Wyrm:
 *   JSON null      -> VAL_NULL
 *   JSON bool      -> VAL_BOOL
 *   JSON number    -> VAL_NUMBER
 *   JSON string    -> VAL_STRING
 *   JSON array     -> VAL_ARRAY of Values
 *   JSON object    -> VAL_ARRAY of alternating key-value pairs:
 *                     [key0, val0, key1, val1, ...]
 *                     tagged with a sentinel string "__json_obj__" at index 0
 *
 * Object representation (array with sentinel):
 *   arr[0] = "__json_obj__"
 *   arr[1] = key0 (VAL_STRING)
 *   arr[2] = val0 (any JSON value)
 *   arr[3] = key1 (VAL_STRING)
 *   ...
 *
 * Wyrm code accesses object fields via json_get(obj, key).
 */

/* Parse a JSON string. Returns the parsed Value tree or VAL_ERROR on failure. */
Value json_parse(Value json_str);

/* Encode a Value tree to a compact JSON string. */
Value json_encode(Value val);

/* Encode a Value tree to a pretty-printed JSON string with the given indent size. */
Value json_pretty(Value val, Value indent_size);

/* Object field access: json_get(obj, "key") */
Value json_get(Value obj, Value key);

/* Check if an object has a key: json_has(obj, "key") -> bool */
Value json_has(Value obj, Value key);

/* Set a field on a JSON object: json_set(obj, "key", value) */
Value json_set(Value obj, Value key, Value val);

/* Create an empty JSON object */
Value json_object(void);

/* Check if a Value is a JSON object (has the sentinel) */
int json_is_object(Value v);

#ifdef __cplusplus
}
#endif

#endif /* WYRM_STD_JSON_H */
