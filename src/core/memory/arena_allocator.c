#include <assert.h>
#include <string.h>
#include "arena_allocator.h"

static inline size_t align_up(size_t n, size_t align)
{
  return (n + align - 1) & ~(align - 1);
}

arena_t arena_init(size_t size)
{
  void* buffer = malloc(size);
  assert(buffer && "arena_init: malloc failed");

  return (arena_t){
    .buffer = buffer,
    .capacity = size,
    .offset = 0
  };
}

void arena_free(arena_t* arena)
{
  free(arena->buffer);
  arena->buffer = NULL;
  arena->capacity = 0;
  arena->offset = 0;
}

void* arena_alloc(arena_t* arena, size_t size)
{
  if(arena->offset + size > arena->capacity) {
    return NULL;
  }

  void* ptr = arena->buffer + arena->offset;
  arena->offset += size;
  return ptr;
}

void* arena_alloc_aligned(arena_t* arena, size_t size, size_t align)
{
  size_t offset = align_up(arena->offset, align);

  if(offset + size > arena->capacity) {
    return NULL;
  }

  void* ptr = arena->buffer + offset;
  arena->offset = offset + size;
  return ptr;
}

void arena_reset(arena_t* arena)
{
  arena->offset = 0;
}
