#ifndef WYRM_ARENA_H
#define WYRM_ARENA_H

#include <stddef.h>

// Arena Allocator structure
typedef struct {
    char   *buf;        // raw memory block
    size_t  cap;        // total capacity in bytes
    size_t  used;       // bytes allocated so far
    int     freed;      // 1 after arena_reset(), prevents alloc after free
} WyrmArena;

// Arena operations
WyrmArena *arena_create(size_t cap);
void *arena_alloc(WyrmArena *a, size_t sz);
void arena_reset(WyrmArena *a);
void arena_destroy(WyrmArena *a);

#endif // WYRM_ARENA_H
