#ifndef WYRM_STR_H
#define WYRM_STR_H

#include "wyrm_core.h"

// String & Data helper declarations
Value val_split(Value s, Value sep);
Value val_join(Value sep, Value lst);
Value val_trim(Value s);
Value val_upper(Value s);
Value val_lower(Value s);
Value val_contains(Value s, Value sub);
Value val_replace(Value s, Value old, Value sub_new);
Value val_starts_with(Value s, Value prefix);
Value val_ends_with(Value s, Value suffix);
Value val_char_at(Value s, Value idx);
Value val_ord_val(Value c);
Value val_chr_val(Value n);
Value val_to_bytes(Value s);
Value val_from_bytes(Value lst);

#endif // WYRM_STR_H
