#include "i_allocator.h"
#include "memory_context.h"

#include <assert.h>
#include <string.h>

/* ---------------- Scratch Internal ---------------- */

struct scratch_t {
  arena_t* arena;
  size_t reset_offset;
  int arena_index;
  int depth;
};

/* ---------------- Global Context ---------------- */

static memory_context_t g_memory_context;

/* ---------------- API ---------------- */

memory_context_t* memory_context_create(
  i_mem_t* mem,
  i_allocator_t* allocator,
  size_t scratch_allocator_size)
{
  memory_context_t* ctx = &g_memory_context;
  memset(ctx, 0, sizeof(*ctx));

  ctx->allocator = allocator;

  for(int i = 0; i < SCRATCH_ARENA_COUNT; ++i) {
    ctx->scratch_arenas[i] = arena_init(mem, scratch_allocator_size);
    ctx->scratch_depth[i] = 0;
  }

  ctx->next_scratch = 0;
  ctx->initialized = true;

  return ctx;
}

void memory_context_destroy(memory_context_t* ctx)
{
  assert(ctx->initialized);

  allocator_destroy(ctx->allocator);

  for(int i = 0; i < SCRATCH_ARENA_COUNT; ++i) {
    allocator_destroy(&ctx->scratch_arenas[i].interface);
  }

  ctx->initialized = false;
}

scratch_t* scratch_begin(void)
{
  memory_context_t* ctx = &g_memory_context;
  assert(ctx->initialized);

  int arena_index = ctx->next_scratch;
  ctx->next_scratch = (ctx->next_scratch + 1) % SCRATCH_ARENA_COUNT;

  arena_t* arena = &ctx->scratch_arenas[arena_index];
  int depth = ctx->scratch_depth[arena_index];

  assert(depth < MAX_SCRATCH_DEPTH);

  size_t offset = arena->offset;
  ctx->scratch_stack[arena_index][depth] = offset;
  ctx->scratch_depth[arena_index]++;

  scratch_t* scratch = allocator_alloc(&arena->interface, sizeof(scratch_t), _Alignof(scratch_t));

  *scratch = (scratch_t){
    .arena = arena,
    .reset_offset = offset,
    .arena_index = arena_index,
    .depth = depth
  };

  return scratch;
}

void scratch_end(scratch_t* scratch)
{
  assert(scratch);

  memory_context_t* ctx = &g_memory_context;
  int arena_index = scratch->arena_index;

  assert(ctx->scratch_depth[arena_index] > 0);

  ctx->scratch_depth[arena_index]--;

  size_t expected_offset =
    ctx->scratch_stack[arena_index][ctx->scratch_depth[arena_index]];

  /* Enforce strict stack discipline */
  assert(expected_offset == scratch->reset_offset);

  scratch->arena->offset = expected_offset;
}

i_allocator_t* scratch_allocator(scratch_t* scratch)
{
  assert(scratch);
  return &scratch->arena->interface;
}

memory_context_t* memory_context_get(void)
{
  return &g_memory_context;
}
