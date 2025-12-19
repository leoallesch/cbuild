#include "builder.h"
#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/logger/logger.h"
#include "core/os/directory.h"
#include "core/string/string.h"

static bool run(builder_t* b, hook_step_t* h)
{
  (void)h;
  log_info_tag("CLEAN", "Removing " STR_FMT, STR_ARG(b->build_dir));
  return directory_delete_recursive(b->build_dir);
}

hook_step_t* hook_clean(builder_t* b)
{
  hook_step_t* hook = allocator_alloc(b->allocator, sizeof(hook_step_t), alignof(hook_step_t));

  hook->name = string("clean");
  hook->inputs = NULL; // No inputs - always run
  hook->output = string_empty(); // No output - always run
  hook->run = run;
  hook->on_complete = NULL;

  return hook;
}
