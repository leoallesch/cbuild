#include "core/mem/heap_mem.h"
#include "core/mem/mem.h"

#include <stdlib.h>

static void* heap_mem_reserve(mem_t* interface, size_t* size)
{
  (void)interface;
  return malloc(*size);
}

static void heap_mem_release(mem_t* interface, void* ptr, size_t size)
{
  (void)interface;
  (void)size;
  free(ptr);
}

static void* heap_mem_commit(mem_t* interface, void* ptr, size_t* size)
{
  (void)interface;
  (void)size;
  return ptr;
}

static mem_t interface = {
  .commit = heap_mem_commit,
  .reserve = heap_mem_reserve,
  .release = heap_mem_release,
};

mem_t* heap_mem_init()
{
  return &interface;
}
