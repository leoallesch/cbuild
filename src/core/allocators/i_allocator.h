#pragma once

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct i_allocator_t {
  void* (*alloc)(struct i_allocator_t* alloc, size_t size, size_t alignment);
  void (*free)(struct i_allocator_t* alloc, void* ptr);
  void (*free_all)(struct i_allocator_t* alloc);
  void* (*realloc)(struct i_allocator_t* alloc, void* ptr, size_t old_size, size_t size, size_t alignment);
  void (*destroy)(struct i_allocator_t* alloc);
} i_allocator_t;

static inline void* allocator_alloc(i_allocator_t* allocator, size_t size, size_t alignment)
{
  return allocator->alloc(allocator, size, alignment);
}

static inline void allocator_free(i_allocator_t* allocator, void* ptr)
{
  allocator->free(allocator, ptr);
}

static inline void allocator_free_all(i_allocator_t* allocator)
{
  allocator->free_all(allocator);
}

static inline void* allocator_realloc(i_allocator_t* allocator, void* ptr, size_t old_size, size_t size, size_t alignment)
{
  return allocator->realloc(allocator, ptr, old_size, size, alignment);
}

static inline void allocator_destroy(i_allocator_t* allocator)
{
  allocator->destroy(allocator);
}

#define allocator_alloc_arr(allocator, type, num_elements) \
  (type*)allocator_alloc(allocator, sizeof(type) * num_elements, alignof(type))
