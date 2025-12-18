#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct i_mem_t {
  void* (*reserve)(struct i_mem_t* mem, size_t* size);
  void (*release)(struct i_mem_t* mem, void* ptr, size_t size);
  void* (*commit)(struct i_mem_t* mem, void* ptr, size_t* size);
} i_mem_t;

static inline void* mem_reserve(i_mem_t* mem, size_t* size)
{
  return mem->reserve(mem, size);
}

static inline void mem_release(i_mem_t* mem, void* ptr, size_t size)
{
  mem->release(mem, ptr, size);
}

static inline void* mem_commit(i_mem_t* mem, void* ptr, size_t* size)
{
  return mem->commit(mem, ptr, size);
}
