#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/alloc/allocator.h"
#include "core/mem/mem.h"

typedef struct arena_t {
  allocator_t interface;
  mem_t* mem;
  uint8_t* buffer;
  size_t capacity;
  size_t offset;
  size_t committed;
} arena_t;

arena_t arena_init(mem_t* mem, size_t size);
