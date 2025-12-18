#include <assert.h>
#include <string.h>

#include "arena_allocator.h"
#include "i_allocator.h"
#include "mem_utils.h"

void arena_destroy(i_allocator_t* allocator)
{
  arena_t* arena = (arena_t*)allocator;
  mem_release(arena->mem, arena->buffer, arena->capacity);
  arena->buffer = NULL;
  arena->capacity = 0;
  arena->offset = 0;
}

void arena_free_all(i_allocator_t* allocator)
{
  arena_t* arena = (arena_t*)allocator;
  arena->offset = 0;
}

static void* arena_allocate(i_allocator_t* allocator, size_t size, size_t alignment)
{
  arena_t* arena = (arena_t*)allocator;
  if(size == 0) {
    return NULL;
  }

  if(alignment == 0) {
    alignment = DEFAULT_ALIGNMENT;
  }

  size_t aligned_offset = ALIGN_UP(arena->offset, alignment);
  size_t new_offset = aligned_offset + size;

  if(new_offset > arena->capacity) {
    return NULL;
  }

  if(new_offset > arena->committed) {
    size_t commit_size = new_offset - arena->committed;

    if(arena->committed + commit_size > arena->capacity) {
      commit_size = arena->capacity - arena->committed;
    }

    if(!mem_commit(arena->mem, arena->buffer + arena->committed, &commit_size)) {
      return NULL;
    }
    arena->committed += commit_size;
  }

  void* ptr = arena->buffer + aligned_offset;
  arena->offset = new_offset;

  memset(ptr, 0, size);

  return ptr;
}

static void* arena_realloc(i_allocator_t* allocator, void* ptr, size_t old_size, size_t size, size_t alignment)
{
  void* buffer = arena_allocate(allocator, size, alignment);
  if(!buffer) {
    return NULL;
  }
  memcpy(buffer, ptr, old_size);
  return buffer;
}

static void arena_free(i_allocator_t* allocator, void* ptr)
{
  (void)allocator;
  (void)ptr;
  assert(!"Arena allocator does not support individual frees!");
}

static i_allocator_t interface = {
  .alloc = arena_allocate,
  .free = arena_free,
  .free_all = arena_free_all,
  .realloc = arena_realloc,
  .destroy = arena_destroy,
};

arena_t arena_init(i_mem_t* mem, size_t size)
{
  uint8_t* buffer = mem_reserve(mem, &size);

  if(!buffer) {
    return (arena_t){ 0 };
  }

  arena_t arena = {
    .interface = interface,
    .mem = mem,
    .buffer = buffer,
    .capacity = size,
    .offset = 0,
    .committed = 0
  };

  return arena;
}
