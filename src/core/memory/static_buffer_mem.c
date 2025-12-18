#include "i_mem.h"
#include "static_buffer_mem.h"

#include <assert.h>
#include <stdio.h>

static void* static_buffer_mem_reserve(i_mem_t* interface, size_t* size)
{
  static_buffer_mem_t* mem = (static_buffer_mem_t*)interface;
  assert(mem->committed + *size <= mem->capacity);
  void *ptr = mem->buffer + mem->committed;
  mem->committed += *size;
  return ptr;
}

static void static_buffer_mem_release(i_mem_t* interface, void* ptr, size_t size)
{
  (void)size;
  (void)ptr;
  static_buffer_mem_t* mem = (static_buffer_mem_t*)interface;
  mem->committed = 0;
}

static void* static_buffer_mem_commit(i_mem_t* interface, void* ptr, size_t* size)
{
  (void)interface;
  (void)size;
  return ptr;
}

void static_buffer_mem_init(static_buffer_mem_t* mem, void* buffer, size_t size)
{
  if(buffer == NULL || size == 0) {
    return;
  }

  i_mem_t interface = {
    .reserve = static_buffer_mem_reserve,
    .release = static_buffer_mem_release,
    .commit = static_buffer_mem_commit,
  };

  mem->interface = interface;
  mem->buffer = buffer;
  mem->capacity = size;
  mem->committed = 0;
}
