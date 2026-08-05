#include "wyrm_arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Arena Allocator Implementation ----

WyrmArena *arena_create(size_t cap) {
    WyrmArena *a = (WyrmArena*)malloc(sizeof(WyrmArena));
    if (!a) { fprintf(stderr, "Out of memory\n"); exit(1); }
    a->buf   = (char*)malloc(cap);
    if (!a->buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
    a->cap   = cap;
    a->used  = 0;
    a->freed = 0;
    return a;
}

void *arena_alloc(WyrmArena *a, size_t sz) {
    if (!a || a->freed) {
        fprintf(stderr, "Runtime Error: arena_alloc() on freed/null arena\n");
        exit(1);
    }
    // Align to 8 bytes
    size_t aligned = (sz + 7u) & ~7u;
    if (a->used + aligned > a->cap) {
        fprintf(stderr, "Runtime Error: arena out of space (cap=%zu used=%zu requested=%zu)\n",
                a->cap, a->used, aligned);
        exit(1);
    }
    void *ptr = a->buf + a->used;
    a->used += aligned;
    return ptr;
}

void arena_reset(WyrmArena *a) {
    if (!a) return;
    a->used  = 0;
    a->freed = 1;  // prevent further alloc() without reinit
}

void arena_destroy(WyrmArena *a) {
    if (!a) return;
    free(a->buf);
    free(a);
}

