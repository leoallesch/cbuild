#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "arena_allocator.h"
#include "i_allocator.h"
#include "i_mem.h"

/* ---------------- Configuration ---------------- */

#define SCRATCH_ARENA_COUNT 2
#define MAX_SCRATCH_DEPTH 32

/* ---------------- Opaque Scratch ---------------- */

typedef struct scratch_t scratch_t;

/* ---------------- Memory Context ---------------- */

typedef struct
{
  i_allocator_t* allocator;

  arena_t scratch_arenas[SCRATCH_ARENA_COUNT];
  size_t scratch_stack[SCRATCH_ARENA_COUNT][MAX_SCRATCH_DEPTH];
  int scratch_depth[SCRATCH_ARENA_COUNT];

  int next_scratch;
  bool initialized;
} memory_context_t;

/* ---------------- API ---------------- */

memory_context_t* memory_context_create(
  i_mem_t* mem,
  i_allocator_t* allocator,
  size_t scratch_allocator_size);

void memory_context_destroy(memory_context_t* context);

scratch_t* scratch_begin(void);
void scratch_end(scratch_t* scratch);

i_allocator_t* scratch_allocator(scratch_t* scratch);

#define scratch_scope(name) \
  for(scratch_t* name = scratch_begin(); name; scratch_end(name), name = NULL)

memory_context_t* memory_context_get(void);
