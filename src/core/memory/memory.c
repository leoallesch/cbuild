#include "arena_allocator.h"
#include "memory.h"

memory_context_t memory_context_create(size_t global_size, size_t scratch_size)
{
  return (memory_context_t){
    .global = arena_init(global_size),
    .scratch_pool[0] = arena_init(scratch_size),
    .scratch_pool[1] = arena_init(scratch_size),
    .scratch_pool_index = 0
  };
}

void memory_context_destroy(memory_context_t* context)
{
  arena_free(&context->global);
  arena_free(&context->scratch_pool[0]);
  arena_free(&context->scratch_pool[1]);
}

scratch_t scratch_begin(memory_context_t* context)
{
  arena_t* arena = &context->scratch_pool[context->scratch_pool_index];
  context->scratch_pool_index ^= 1;
  return (scratch_t){ .arena = arena, .reset_offset = arena->offset };
}

void scratch_end(scratch_t *s)
{
    s->arena->offset = s->reset_offset;
};
