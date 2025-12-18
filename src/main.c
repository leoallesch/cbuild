#include <stddef.h>

#include "arena_allocator.h"
#include "array_list.h"
#include "build_step.h"
#include "builder.h"
#include "directory.h"
#include "i_allocator.h"
#include "i_mem.h"
#include "logger.h"
#include "mem_utils.h"
#include "memory_context.h"
#include "path.h"
#include "process.h"
#include "string_helper.h"
#include "virtual_mem.h"

extern void build(builder_t* builder);

int main()
{
  i_mem_t* vmem = virtual_mem_init();
  arena_t arena = arena_init(vmem, MB(10));

  memory_context_t* context = memory_context_create(vmem, &arena.interface, KB(10));

  logger_init();
  logger_set_level(LOG_DEBUG);

  builder_t* builder = builder_create(context->allocator);

  build(builder);

  logger_shutdown();
  memory_context_destroy(context);

  return 0;
}
