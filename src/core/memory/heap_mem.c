#include "heap_mem.h"
#include "i_mem.h"

#include <stdlib.h>

static void* heap_mem_reserve(i_mem_t* interface, size_t *size)
{
  (void)interface;
  return malloc(*size);
}

static void heap_mem_release(i_mem_t* interface, void* ptr, size_t size)
{
  (void)interface;
  (void)size;
  free(ptr);
}

static void* heap_mem_commit(i_mem_t* interface, void* ptr, size_t *size)
{
  (void)interface;
  (void)size;
  return ptr;
}

static i_mem_t interface = {
  .commit = heap_mem_commit,
  .reserve = heap_mem_reserve,
  .release = heap_mem_release,
};

i_mem_t* heap_mem_init()
{
  return &interface;
}
