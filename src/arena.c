#include "../include/arena.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void arena_init(Arena *arena, size_t block_size) {
    arena->first = NULL;
    arena->current = NULL;
    arena->default_block_size = block_size > 0 ? block_size : 4096;
}

static ArenaBlock *arena_new_block(size_t size) {
    ArenaBlock *block = malloc(sizeof(ArenaBlock) + size);
    if (!block) {
        perror("malloc");
        exit(1);
    }
    block->next = NULL;
    block->used = 0;
    block->capacity = size;
    return block;
}

void *arena_alloc(Arena *arena, size_t size, size_t align) {
    if (size == 0) return NULL;
    
    /* Ensure alignment is valid */
    if (align == 0) align = sizeof(void *);
    
    /* Try to allocate from current block */
    if (arena->current) {
        /* Calculate aligned position */
        size_t current_addr = (size_t)arena->current->data + arena->current->used;
        size_t aligned_addr = (current_addr + align - 1) & ~(align - 1);
        size_t padding = aligned_addr - current_addr;
        size_t total_needed = padding + size;
        
        if (arena->current->used + total_needed <= arena->current->capacity) {
            void *ptr = (void *)aligned_addr;
            arena->current->used += total_needed;
            return ptr;
        }
    }
    
    /* Need a new block */
    size_t block_size = arena->default_block_size;
    if (size > block_size) {
        /* Allocate a larger block for this single allocation */
        block_size = size + align;
    }
    
    ArenaBlock *new_block = arena_new_block(block_size);
    
    /* Link into list */
    if (!arena->first) {
        arena->first = new_block;
    }
    if (arena->current) {
        arena->current->next = new_block;
    }
    arena->current = new_block;
    
    /* Allocate from the new block */
    size_t current_addr = (size_t)new_block->data;
    size_t aligned_addr = (current_addr + align - 1) & ~(align - 1);
    size_t padding = aligned_addr - current_addr;
    
    new_block->used = padding + size;
    return (void *)aligned_addr;
}

void arena_free(Arena *arena) {
    ArenaBlock *block = arena->first;
    while (block) {
        ArenaBlock *next = block->next;
        free(block);
        block = next;
    }
    arena->first = NULL;
    arena->current = NULL;
}

void arena_reset(Arena *arena) {
    ArenaBlock *block = arena->first;
    while (block) {
        block->used = 0;
        block = block->next;
    }
    arena->current = arena->first;
}
