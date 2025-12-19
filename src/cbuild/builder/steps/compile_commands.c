#include "build_step.h"
#include "builder.h"
#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/logger/logger.h"
#include "core/mem/memory_context.h"
#include "core/os/directory.h"
#include "core/os/file.h"
#include "core/string/string.h"

#include "core/os/path.h"
#include "core/string/string_builder.h"

static bool run(builder_t* b, hook_step_t* h)
{
  (void)b;
  scratch_t* scratch = scratch_begin();
  allocator_t* temp_alloc = scratch_allocator(scratch);

  build_step_t** steps = h->ctx;

  log_info_tag("COMPILE_COMMANDS", "Generating " STR_FMT, STR_ARG(h->output));

  fd_t fd = file_open(h->output, FILE_MODE_W);
  string_builder_t contents = string_builder_init(temp_alloc);
  string_t cwd = directory_cwd(temp_alloc);
  string_t entry;

  string_builder_append(&contents, string("[\n"));

  array_list_for_each(steps, build_step_t*, step)
  {
    if(step->kind == STEP_COMPILE) {
      string_builder_append(&contents, string("\t{\n"));

      string_builder_append(&contents, string("\t\t\"directory\": \""));
      string_builder_append(&contents, cwd);
      string_builder_append(&contents, string("\",\n"));

      string_builder_append(&contents, string("\t\t\"command\": \""));
      array_list_for_each(step->command.argv, string_t, arg)
      {
        string_builder_append(&contents, arg);
        string_builder_append(&contents, string(" "));
      }
      string_builder_append(&contents, string("\",\n"));

      string_builder_append(&contents, string("\t\t\"file\": \""));
      string_builder_append(&contents, step->inputs[0]);
      string_builder_append(&contents, string("\",\n"));

      string_builder_append(&contents, string("\t\t\"output\": \""));
      string_builder_append(&contents, step->output);
      string_builder_append(&contents, string("\"\n"));

      string_builder_append(&contents, string("\t}"));

      if(_i < _len - 1) {
        string_builder_append(&contents, string(","));
      }
      string_builder_append(&contents, string("\n"));

      entry = string_builder_to_string(&contents);

      file_append_fd(fd, entry.string, entry.length);

      string_builder_reset(&contents);
    }
  };

  string_builder_append(&contents, string("]\n"));
  entry = string_builder_to_string(&contents);

  file_append_fd(fd, entry.string, entry.length);

  scratch_end(scratch);

  return true;
}

hook_step_t* hook_compile_commands(builder_t* b, target_t* t, build_step_t** compile_steps)
{
  hook_step_t* hook = allocator_alloc(b->allocator, sizeof(hook_step_t), alignof(hook_step_t));

  hook->name = string("compile_commands");
  hook->inputs = array_list(string_t, b->allocator);
  hook->ctx = compile_steps;

  array_list_for_each(compile_steps, build_step_t*, step)
  {
    array_list_push(hook->inputs, step->inputs[0]);
  };

  string_t output = path_join(b->build_dir, t->artifacts_dir, b->allocator);
  output = path_join(output, string("compile_commands.json"), b->allocator);

  hook->output = output;
  hook->run = run;
  hook->on_complete = NULL;

  return hook;
}
