#include "build_step.h"
#include "builder.h"
#include "i_allocator.h"
#include "target.h"

void build(builder_t* builder)
{
  i_allocator_t* allocator = builder->allocator;

  builder_set_build_dir(builder, string("tmp_build"));

  target_t* target = target_executable(allocator, string("hello_world"));

  target_add_source_dir(target, string("examples/hello_world"));
  builder_add_target(builder, target);

  build_result_t res = builder_run(builder);
  builder_log_result(res);
}
