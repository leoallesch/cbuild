#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct mem_t {
  void* (*reserve)(struct mem_t* mem, size_t* size);
  void (*release)(struct mem_t* mem, void* ptr, size_t size);
  void* (*commit)(struct mem_t* mem, void* ptr, size_t* size);
} mem_t;

static inline void* mem_reserve(mem_t* mem, size_t* size)
{
  return mem->reserve(mem, size);
}

static inline void mem_release(mem_t* mem, void* ptr, size_t size)
{
  mem->release(mem, ptr, size);
}

static inline void* mem_commit(mem_t* mem, void* ptr, size_t* size)
{
  return mem->commit(mem, ptr, size);
}
