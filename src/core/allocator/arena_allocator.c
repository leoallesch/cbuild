#include "arena_allocator.h"
#include "i_allocator.h"

#include <assert.h>

static void* arena_alloc(i_allocator_t* alloc, size_t size)
{
  arena_allocator_t* a = (arena_allocator_t*)alloc;
  assert(a->offset + size < a->capacity);
  void* ptr = a->buffer + a->offset;
  a->offset += size;
  return ptr;
}

static void arena_free(i_allocator_t* alloc, void* ptr)
{
  (void)alloc;
  (void)ptr;
  assert(!"arena_allocator does not support alloc_free()!");
}

static void* arena_realloc(i_allocator_t* alloc, void* ptr, size_t size)
{
  (void)alloc;
  (void)ptr;
  (void)size;
  assert(!"arena_allocator does not support alloc_realloc()!");
}

static void arena_reset(i_allocator_t* alloc)
{
  arena_allocator_t* a = (arena_allocator_t*)alloc;
  a->offset = 0;
}

static void arena_destroy(i_allocator_t* alloc)
{
  arena_allocator_t* a = (arena_allocator_t*)alloc;
  free(a->buffer);
}

static i_allocator_t interface = {
  .alloc = arena_alloc,
  .free = arena_free,
  .reset = arena_reset,
  .realloc = arena_realloc,
  .destroy = arena_destroy
};

arena_allocator_t arena_init(size_t size)
{
  return (arena_allocator_t){
    .interface = interface,
    .buffer = malloc(size),
    .capacity = size,
    .offset = 0
  };
}
