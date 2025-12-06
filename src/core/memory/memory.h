#pragma once

#include "arena_allocator.h"

typedef struct
{
  arena_t global;
  arena_t scratch_pool[2];
  uint32_t scratch_pool_index;
} memory_context_t;

typedef struct
{
  arena_t* arena;
  size_t reset_offset;
} scratch_t;

memory_context_t memory_context_create(size_t global_size, size_t scratch_size);
void memory_context_destroy(memory_context_t* context);

scratch_t scratch_begin(memory_context_t* context);
void scratch_end(scratch_t* s);

#define context_alloc_global(ctx, size) arena_alloc(&(ctx)->global, size)
#define context_alloc_global_aligned(ctx, size, alignment) arena_alloc_aligned(&(ctx)->global, size, alignment)
#define context_alloc_global_type(ctx, type, count) ((type*)arena_alloc(&(ctx)->global, sizeof(type) * count))

#define scratch_scope(mem_ctx, scratch_name) \
  for(scratch_t scratch_name = scratch_begin(mem_ctx); scratch_name.arena; scratch_end(&scratch_name), scratch_name.arena = NULL)
