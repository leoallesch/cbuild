#pragma once

#include <stdint.h>
#include <stdlib.h>

#define KB(x) ((x)*1024)
#define MB(x) ((x)*1024 * 1024)
#define GB(x) ((x)*1024 * 1024 * 1024)

typedef struct
{
  uint8_t* buffer;
  size_t capacity;
  size_t offset;
} arena_t;

arena_t arena_init(size_t size);
void arena_free(arena_t* allocator);
void* arena_alloc(arena_t* allocator, size_t size);
void* arena_alloc_aligned(arena_t* arena, size_t size, size_t align);
void arena_reset(arena_t* allocator);

#define arena_alloc_type(arena, type, count) ((type*)arena_alloc(arena, sizeof(type) * count))
