#pragma once

#include "core/mem/mem.h"

typedef struct
{
  mem_t interface;
  void* buffer;
  size_t capacity;
  size_t committed;
} static_buffer_mem_t;

void static_buffer_mem_init(static_buffer_mem_t* mem, void* buffer, size_t size);
