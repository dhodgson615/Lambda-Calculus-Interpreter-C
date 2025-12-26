#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief              Arena allocator for efficient memory allocation.
 * 
 * An arena allocator allocates memory in large blocks and distributes
 * smaller chunks from those blocks. This reduces the overhead of many
 * small allocations and improves cache locality.
 */
typedef struct ArenaBlock {
    struct ArenaBlock *next;
    size_t used;
    size_t capacity;
    char data[];
} ArenaBlock;

typedef struct Arena {
    ArenaBlock *current;
    ArenaBlock *first;
    size_t default_block_size;
} Arena;

/**
 * @brief              Initialize an arena allocator.
 * @param  arena       the arena to initialize
 * @param  block_size  the default size for arena blocks
 */
void arena_init(Arena *arena, size_t block_size);

/**
 * @brief              Allocate memory from the arena.
 * @param  arena       the arena to allocate from
 * @param  size        the number of bytes to allocate
 * @param  align       the alignment requirement (must be power of 2)
 * @return             pointer to allocated memory, or NULL on failure
 */
void *arena_alloc(Arena *arena, size_t size, size_t align);

/**
 * @brief              Free all memory in the arena.
 * @param  arena       the arena to free
 */
void arena_free(Arena *arena);

/**
 * @brief              Reset the arena without freeing memory.
 * @param  arena       the arena to reset
 * 
 * This allows reusing the allocated blocks for new allocations.
 */
void arena_reset(Arena *arena);

#endif /* ARENA_H */
