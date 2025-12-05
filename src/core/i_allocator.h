#pragma once

#include <stdlib.h>

typedef struct i_allocator_t {
  void* (*alloc)(struct i_allocator_t* alloc, size_t size);
  void (*free)(struct i_allocator_t* alloc, void* ptr);
  void* (*realloc)(struct i_allocator_t* alloc, void* ptr, size_t size);
  void (*reset)(struct i_allocator_t* alloc);
  void (*destroy)(struct i_allocator_t* alloc);
} i_allocator_t;

static inline void* alloc_alloc(i_allocator_t* alloc, size_t size)
{
  return alloc->alloc(alloc, size);
}

static inline void alloc_free(i_allocator_t* alloc, void* ptr)
{
  alloc->free(alloc, ptr);
}

static inline void* alloc_realloc(i_allocator_t* alloc, void* ptr, size_t size)
{
  return alloc->realloc(alloc, ptr, size);
}

static inline void alloc_reset(i_allocator_t* alloc)
{
  alloc->reset(alloc);
}

static inline void alloc_destroy(i_allocator_t* alloc)
{
  alloc->destroy(alloc);
}
