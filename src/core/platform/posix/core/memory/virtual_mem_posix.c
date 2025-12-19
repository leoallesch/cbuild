#include "core/mem/mem.h"
#include "core/mem/mem_utils.h"
#include "core/mem/virtual_mem.h"

#include <sys/mman.h>
#include <unistd.h>

static struct
{
  mem_t interface;
  size_t page_size;
} virtual_mem;

static bool init = false;

static void* virtual_mem_reserve(mem_t* interface, size_t* size)
{
  (void)interface;
  *size = ALIGN_UP(*size, virtual_mem.page_size);

  void* ptr = mmap(NULL, *size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if(ptr == MAP_FAILED) {
    return NULL;
  }

  return ptr;
}

static void* virtual_mem_commit(mem_t* interface, void* ptr, size_t* size)
{
  (void)interface;
  *size = ALIGN_UP(*size, virtual_mem.page_size);

  if(mprotect(ptr, *size, PROT_READ | PROT_WRITE) == -1)
    return NULL;

  return ptr;
}

static void virtual_mem_release(mem_t* interface, void* ptr, size_t size)
{
  (void)interface;
  if(ptr) {
    munmap(ptr, size);
  }
}

mem_t* virtual_mem_init()
{
  virtual_mem.page_size = sysconf(_SC_PAGESIZE);
  if(virtual_mem.page_size <= 0) {
    virtual_mem.page_size = DEFAULT_PAGE_SIZE;
  }

  virtual_mem.interface = (mem_t){
    .reserve = virtual_mem_reserve,
    .commit = virtual_mem_commit,
    .release = virtual_mem_release,
  };

  init = true;

  return &virtual_mem.interface;
}

mem_t* virtual_mem_get()
{
  if(!init) {
    return virtual_mem_init();
  }
  return &virtual_mem.interface;
}
