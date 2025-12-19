#include "cbuild/builder.h"
#include "cbuild/target.h"

void build(builder_t* b)
{
  allocator_t* alloc = builder_allocator(b);

  target_t* hello_world = target_executable(alloc, string("hello_world"));

  target_add_sources(hello_world, string("main.c"));

  target_add_include(hello_world, string("../../include"));

  target_log_config(hello_world);

  builder_add_target(b, hello_world);
}
