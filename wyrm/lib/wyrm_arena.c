#include "wyrm_arena.h"
#include "wyrm_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Arena Allocator Implementation ----

WyrmArena *arena_create(size_t cap) {
    WyrmArena *a = (WyrmArena*)malloc(sizeof(WyrmArena));
    wyrm_check_oom(a, "arena_create (WyrmArena struct)");
    a->buf   = (char*)malloc(cap);
    wyrm_check_oom(a->buf, "arena_create (arena buffer)");
    a->cap   = cap;
    a->used  = 0;
    a->freed = 0;
    return a;
}

void *arena_alloc(WyrmArena *a, size_t sz) {
    if (!a) {
        fprintf(stderr, "Runtime Error: arena_alloc() on null arena\n");
        exit(1);
    }
    if (a->freed) {
        fprintf(stderr, "Runtime Error: arena_alloc() on destroyed arena\n");
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
    a->freed = 0;  // allow reuse after reset
}

void arena_destroy(WyrmArena *a) {
    if (!a) return;
    free(a->buf);
    a->buf   = NULL;
    a->used  = 0;
    a->freed = 1;  // mark destroyed before freeing struct
    free(a);
}

