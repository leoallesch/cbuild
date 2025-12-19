#pragma once

// ============================================================================
// Internal Allocator Interface
// ============================================================================
//
// This is the internal definition of allocator_t. Users only see it as
// an opaque pointer - they get one from builder_allocator() and pass it
// to target creation functions.
//

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct allocator_t {
  void* (*alloc)(struct allocator_t* alloc, size_t size, size_t alignment);
  void (*free)(struct allocator_t* alloc, void* ptr);
  void (*free_all)(struct allocator_t* alloc);
  void* (*realloc)(struct allocator_t* alloc, void* ptr, size_t old_size, size_t size, size_t alignment);
  void (*destroy)(struct allocator_t* alloc);
} allocator_t;

// ============================================================================
// Allocator Helpers
// ============================================================================

static inline void* allocator_alloc(allocator_t* allocator, size_t size, size_t alignment)
{
  return allocator->alloc(allocator, size, alignment);
}

static inline void allocator_free(allocator_t* allocator, void* ptr)
{
  allocator->free(allocator, ptr);
}

static inline void allocator_free_all(allocator_t* allocator)
{
  allocator->free_all(allocator);
}

static inline void* allocator_realloc(allocator_t* allocator, void* ptr, size_t old_size, size_t size, size_t alignment)
{
  return allocator->realloc(allocator, ptr, old_size, size, alignment);
}

static inline void allocator_destroy(allocator_t* allocator)
{
  allocator->destroy(allocator);
}

#define allocator_alloc_arr(allocator, type, num_elements) \
  (type*)allocator_alloc(allocator, sizeof(type) * (num_elements), alignof(type))
