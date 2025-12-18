#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "i_allocator.h"
#include "i_mem.h"

typedef struct
{
  i_allocator_t interface;
  i_mem_t* mem;
  uint8_t* buffer;
  size_t capacity;
  size_t offset;
  size_t committed;
} arena_t;

arena_t arena_init(i_mem_t* mem, size_t size);