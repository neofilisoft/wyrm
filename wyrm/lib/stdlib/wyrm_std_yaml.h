/*
 * wyrm_std_yaml.h - Wyrm Standard Library: YAML Serialization
 *
 * Pure C11 YAML subset parser (block style) and encoder.
 * Covers: scalars, sequences (- item), mappings (key: value),
 * multi-level nesting, quoted strings, null/bool/number detection.
 * No external dependencies.
 *
 * YAML objects are stored identically to JSON objects (sentinel-tagged arrays)
 * for unified access through json_get/json_set.
 */
#ifndef WYRM_STD_YAML_H
#define WYRM_STD_YAML_H

#include "../wyrm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Parse YAML text into a Wyrm Value tree.
 * Uses the same object representation as the JSON module.
 * Returns VAL_ERROR on parse failure.
 */
Value yaml_parse(Value yaml_str);

/*
 * Encode a Wyrm Value tree to YAML text.
 * Produces block-style YAML with 2-space indentation.
 */
Value yaml_encode(Value val);

#ifdef __cplusplus
}
#endif

#endif /* WYRM_STD_YAML_H */
