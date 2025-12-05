#pragma once

#include "i_allocator.h"

#include <stdint.h>

typedef struct
{
  i_allocator_t interface;

  uint8_t* buffer;
  size_t capacity;
  size_t offset;
} arena_allocator_t;

arena_allocator_t arena_init(size_t size);
