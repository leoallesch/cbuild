#include "build_step.h"
#include "builder.h"
#include "cbuild/types.h"
#include "core/container/array_list.h"
#include "core/os/process.h"
#include "core/string/string.h"

static process_command_t generate_cmd(builder_t* b, build_step_t* step)
{
  process_command_t cmd = { 0 };
  string_t archiver = !string_is_empty(b->toolchain.archiver)
    ? b->toolchain.archiver
    : string("ar");
  string_t* args = array_list(string_t, b->allocator);

  // [archiver] rcs [output] [inputs...]
  array_list_push(args, archiver);
  array_list_push(args, string("rcs"));
  array_list_push(args, step->output);

  array_list_for_each(step->inputs, string_t, input)
  {
    array_list_push(args, input);
  }

  cmd.program = archiver;
  cmd.argv = args;
  cmd.cwd = NULL;
  cmd.stdin = PROCESS_FD_INVALID;
  cmd.stdout = PROCESS_FD_INVALID;
  cmd.stderror = PROCESS_FD_INVALID;

  return cmd;
}

static bool needs_rebuild(build_step_t* step)
{
  if(!file_exists(step->output)) {
    return true;
  }

  array_list_for_each(step->inputs, string_t, input)
  {
    if(file_is_newer(input, step->output)) {
      return true;
    }
  }

  return false;
}

static void on_complete(build_step_t* step, builder_t* builder)
{
  (void)step;
  (void)builder;
}

static const char* kind_str(build_step_t* step)
{
  (void)step;
  return "ARCHIVE";
}

static const build_step_api_t api = {
  .needs_rebuild = needs_rebuild,
  .on_complete = on_complete,
  .kind_str = kind_str
};

build_step_t* build_step_archive(builder_t* b, target_t* t, build_step_t** compile_steps)
{
  build_step_t* step = build_step_create(b, t, STEP_ARCHIVE, t->name, &api);

  step->target = t;
  step->output = compute_output_path(b, t);
  step->language = LANG_C; // archiver doesn't care about language

  size_t count = array_list_length(compile_steps);
  for(size_t i = 0; i < count; i++) {
    build_step_t* compile_step = compile_steps[i];
    array_list_push(step->inputs, compile_step->output);
  }

  step->command = generate_cmd(b, step);

  return step;
}
