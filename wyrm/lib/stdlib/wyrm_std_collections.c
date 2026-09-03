/*
 * wyrm_std_collections.c - Wyrm Standard Library: Collections Implementation
 *
 * HashMap: open-addressing hash table with FNV-1a string hashing,
 *          load-factor 0.75, power-of-2 capacity doubling.
 * Set:     thin wrapper over WyrmMap (stores VAL_BOOL true as values).
 */
#include "wyrm_std_collections.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 * HashMap internals
 * -------------------------------------------------------------------------- */
typedef struct MapEntry {
    char  *key;     /* heap-allocated key string; NULL = empty slot */
    Value  val;     /* stored value (owned by map) */
    int    deleted; /* tombstone marker for open addressing */
} MapEntry;

typedef struct WyrmMap {
    MapEntry *slots;
    size_t    capacity; /* must be power of 2 */
    size_t    count;    /* number of live entries */
} WyrmMap;

/* FNV-1a 64-bit hash for string keys */
static uint64_t fnv1a(const char *s) {
    uint64_t h = 14695981039346656037ULL;
    while (*s) { h ^= (uint8_t)*s++; h *= 1099511628211ULL; }
    return h;
}

static WyrmMap *map_alloc(size_t cap) {
    WyrmMap *m = calloc(1, sizeof(WyrmMap));
    if (!m) return NULL;
    m->slots    = calloc(cap, sizeof(MapEntry));
    if (!m->slots) { free(m); return NULL; }
    m->capacity = cap;
    m->count    = 0;
    return m;
}

static void map_free_internal(WyrmMap *m) {
    for (size_t i = 0; i < m->capacity; i++) {
        if (m->slots[i].key && !m->slots[i].deleted) {
            free(m->slots[i].key);
            val_drop(m->slots[i].val);
        } else if (m->slots[i].key) {
            free(m->slots[i].key); /* tombstone: only free key */
        }
    }
    free(m->slots);
    free(m);
}

static int map_insert_internal(WyrmMap *m, const char *key, Value val);

static int map_grow(WyrmMap *m) {
    size_t   new_cap   = m->capacity * 2;
    WyrmMap *nm        = map_alloc(new_cap);
    if (!nm) return 0;
    for (size_t i = 0; i < m->capacity; i++) {
        MapEntry *e = &m->slots[i];
        if (e->key && !e->deleted) {
            map_insert_internal(nm, e->key, e->val);
            /* val ownership transferred to nm - zero out to avoid double-drop */
            e->val = val_null();
        }
    }
    /* Free old slot array (keys were transferred, vals zeroed) */
    for (size_t i = 0; i < m->capacity; i++) {
        if (m->slots[i].key) free(m->slots[i].key);
    }
    free(m->slots);
    m->slots    = nm->slots;
    m->capacity = nm->capacity;
    m->count    = nm->count;
    free(nm);
    return 1;
}

static int map_insert_internal(WyrmMap *m, const char *key, Value val) {
    if (m->count * 4 >= m->capacity * 3) { /* 75% load factor */
        if (!map_grow(m)) return 0;
    }
    uint64_t h    = fnv1a(key);
    size_t   mask = m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    size_t   first_tomb = (size_t)-1;

    for (size_t probe = 0; probe < m->capacity; probe++) {
        MapEntry *e = &m->slots[(idx + probe) & mask];
        if (!e->key) {
            /* Empty slot */
            size_t target = (first_tomb != (size_t)-1) ? first_tomb : (idx + probe) & mask;
            m->slots[target].key     = strdup(key);
            m->slots[target].val     = val;
            m->slots[target].deleted = 0;
            m->count++;
            return 1;
        }
        if (e->deleted) {
            if (first_tomb == (size_t)-1) first_tomb = (idx + probe) & mask;
            continue;
        }
        if (strcmp(e->key, key) == 0) {
            val_drop(e->val);
            e->val = val;
            return 1;
        }
    }
    return 0;
}

static MapEntry *map_find(WyrmMap *m, const char *key) {
    uint64_t h    = fnv1a(key);
    size_t   mask = m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    for (size_t probe = 0; probe < m->capacity; probe++) {
        MapEntry *e = &m->slots[(idx + probe) & mask];
        if (!e->key) return NULL;
        if (!e->deleted && strcmp(e->key, key) == 0) return e;
    }
    return NULL;
}

/* --------------------------------------------------------------------------
 * Public: HashMap functions
 * -------------------------------------------------------------------------- */
Value map_new(void) {
    WyrmMap *m = map_alloc(16);
    if (!m) { fprintf(stderr, "Runtime Error [map_new]: out of memory\n"); exit(1); }
    return val_raw_ptr(m);
}

static WyrmMap *get_map(const char *fn, Value v) {
    if (v.type != VAL_RAW_PTR || !v.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [%s]: argument must be a map handle from map_new()\n", fn);
        exit(1);
    }
    return (WyrmMap *)v.as.raw_ptr;
}

Value map_set(Value map_val, Value key, Value val) {
    WyrmMap *m = get_map("map_set", map_val);
    if (key.type != VAL_STRING || !key.as.string) {
        fprintf(stderr, "Runtime Error [map_set]: key must be a string\n"); exit(1);
    }
    map_insert_internal(m, key.as.string, val_copy(val));
    return map_val;
}

Value map_get(Value map_val, Value key) {
    WyrmMap *m = get_map("map_get", map_val);
    if (key.type != VAL_STRING || !key.as.string) return val_null();
    MapEntry *e = map_find(m, key.as.string);
    if (!e) return val_null();
    return val_copy(e->val);
}

Value map_has(Value map_val, Value key) {
    WyrmMap *m = get_map("map_has", map_val);
    if (key.type != VAL_STRING || !key.as.string) return val_bool(false);
    return val_bool(map_find(m, key.as.string) != NULL);
}

Value map_del(Value map_val, Value key) {
    WyrmMap *m = get_map("map_del", map_val);
    if (key.type != VAL_STRING || !key.as.string) return val_null();
    uint64_t h    = fnv1a(key.as.string);
    size_t   mask = m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    for (size_t probe = 0; probe < m->capacity; probe++) {
        MapEntry *e = &m->slots[(idx + probe) & mask];
        if (!e->key) return val_null();
        if (!e->deleted && strcmp(e->key, key.as.string) == 0) {
            val_drop(e->val);
            e->val     = val_null();
            e->deleted = 1;
            m->count--;
            return val_null();
        }
    }
    return val_null();
}

Value map_keys(Value map_val) {
    WyrmMap *m   = get_map("map_keys", map_val);
    Value    arr = val_array_create((int)m->count);
    arr.as.array->size = 0;
    for (size_t i = 0; i < m->capacity; i++) {
        MapEntry *e = &m->slots[i];
        if (e->key && !e->deleted) {
            arr = val_array_append(arr, val_string(e->key));
        }
    }
    return arr;
}

Value map_values(Value map_val) {
    WyrmMap *m   = get_map("map_values", map_val);
    Value    arr = val_array_create((int)m->count);
    arr.as.array->size = 0;
    for (size_t i = 0; i < m->capacity; i++) {
        MapEntry *e = &m->slots[i];
        if (e->key && !e->deleted) {
            arr = val_array_append(arr, val_copy(e->val));
        }
    }
    return arr;
}

Value map_len(Value map_val) {
    WyrmMap *m = get_map("map_len", map_val);
    return val_number((double)m->count);
}

Value map_free(Value map_val) {
    if (map_val.type != VAL_RAW_PTR || !map_val.as.raw_ptr) return val_null();
    map_free_internal((WyrmMap *)map_val.as.raw_ptr);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Set internals: thin wrapper over WyrmMap, stores val_bool(true)
 * -------------------------------------------------------------------------- */
typedef struct WyrmSet { WyrmMap *m; } WyrmSet;

static WyrmSet *get_set(const char *fn, Value v) {
    if (v.type != VAL_RAW_PTR || !v.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [%s]: argument must be a set handle from set_new()\n", fn);
        exit(1);
    }
    return (WyrmSet *)v.as.raw_ptr;
}

static const char *val_to_set_key(Value v, char *buf, size_t bufsz) {
    switch (v.type) {
        case VAL_STRING:  return v.as.string;
        case VAL_NUMBER:  snprintf(buf, bufsz, "%.17g", v.as.number);  return buf;
        case VAL_BOOL:    return v.as.boolean ? "true" : "false";
        case VAL_NULL:    return "null";
        default:          return NULL;
    }
}

Value set_new(void) {
    WyrmSet *s = malloc(sizeof(WyrmSet));
    if (!s) { fprintf(stderr, "Runtime Error [set_new]: out of memory\n"); exit(1); }
    s->m = (WyrmMap *)map_new().as.raw_ptr;
    return val_raw_ptr(s);
}

Value set_add(Value set_val, Value item) {
    WyrmSet *s = get_set("set_add", set_val);
    char buf[64];
    const char *k = val_to_set_key(item, buf, sizeof(buf));
    if (!k) { fprintf(stderr, "Runtime Error [set_add]: unsupported item type\n"); exit(1); }
    map_insert_internal(s->m, k, val_bool(true));
    return set_val;
}

Value set_has(Value set_val, Value item) {
    WyrmSet *s = get_set("set_has", set_val);
    char buf[64];
    const char *k = val_to_set_key(item, buf, sizeof(buf));
    if (!k) return val_bool(false);
    return val_bool(map_find(s->m, k) != NULL);
}

Value set_del(Value set_val, Value item) {
    WyrmSet *s = get_set("set_del", set_val);
    char buf[64];
    const char *k = val_to_set_key(item, buf, sizeof(buf));
    if (!k) return val_null();
    Value sv = val_raw_ptr(s->m);
    Value kv = k == buf ? val_string(buf) : val_null(); /* reuse key string */
    (void)kv;
    uint64_t h    = fnv1a(k);
    size_t   mask = s->m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    for (size_t probe = 0; probe < s->m->capacity; probe++) {
        MapEntry *e = &s->m->slots[(idx + probe) & mask];
        if (!e->key) return val_null();
        if (!e->deleted && strcmp(e->key, k) == 0) {
            val_drop(e->val); e->val = val_null(); e->deleted = 1; s->m->count--; return val_null();
        }
    }
    (void)sv;
    return val_null();
}

Value set_union_fn(Value a, Value b) {
    WyrmSet *sa = get_set("set_union", a);
    WyrmSet *sb = get_set("set_union", b);
    Value result = set_new();
    WyrmSet *sr  = (WyrmSet *)result.as.raw_ptr;
    for (size_t i = 0; i < sa->m->capacity; i++) {
        MapEntry *e = &sa->m->slots[i];
        if (e->key && !e->deleted) map_insert_internal(sr->m, e->key, val_bool(true));
    }
    for (size_t i = 0; i < sb->m->capacity; i++) {
        MapEntry *e = &sb->m->slots[i];
        if (e->key && !e->deleted) map_insert_internal(sr->m, e->key, val_bool(true));
    }
    return result;
}

Value set_intersect(Value a, Value b) {
    WyrmSet *sa = get_set("set_intersect", a);
    WyrmSet *sb = get_set("set_intersect", b);
    Value result = set_new();
    WyrmSet *sr  = (WyrmSet *)result.as.raw_ptr;
    for (size_t i = 0; i < sa->m->capacity; i++) {
        MapEntry *e = &sa->m->slots[i];
        if (e->key && !e->deleted && map_find(sb->m, e->key)) {
            map_insert_internal(sr->m, e->key, val_bool(true));
        }
    }
    return result;
}

Value set_to_array(Value set_val) {
    WyrmSet *s   = get_set("set_to_array", set_val);
    Value    arr = val_array_create((int)s->m->count);
    arr.as.array->size = 0;
    for (size_t i = 0; i < s->m->capacity; i++) {
        MapEntry *e = &s->m->slots[i];
        if (e->key && !e->deleted) arr = val_array_append(arr, val_string(e->key));
    }
    return arr;
}

Value set_free(Value set_val) {
    if (set_val.type != VAL_RAW_PTR || !set_val.as.raw_ptr) return val_null();
    WyrmSet *s = (WyrmSet *)set_val.as.raw_ptr;
    map_free_internal(s->m);
    free(s);
    return val_null();
}
