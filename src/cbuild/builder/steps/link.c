#include "build_step.h"
#include "builder.h"
#include "cbuild/types.h"
#include "core/container/array_list.h"
#include "core/string/string.h"

// Recursively add target and its transitive dependencies to the link args
static void add_target_libs(builder_t* b, target_t* target, string_t* args)
{
  // Add this target's library
  string_t lib_path = compute_output_path(b, target);
  array_list_push(args, lib_path);

  // Recursively add its LINK_TARGET dependencies
  array_list_for_each(target->link_objects, link_object_t, obj)
  {
    if(obj.kind == LINK_TARGET && obj.target) {
      add_target_libs(b, obj.target, args);
    }
  }
}

static process_command_t generate_cmd(builder_t* b, build_step_t* step)
{
  process_command_t cmd = { 0 };
  string_t linker = !string_is_empty(b->toolchain.linker)
    ? b->toolchain.linker
    : get_compiler_for_lang(b, step->language);
  string_t* args = array_list(string_t, b->allocator);

  // [linker] [inputs...] -o [output] [lib_paths...] [link_objects...] [flags...]
  array_list_push(args, linker);

  // Shared lib flag must come early
  if(step->target->kind == TARGET_SHARED_LIB) {
    array_list_push(args, string("-shared"));
  }

  // Input object files
  array_list_for_each(step->inputs, string_t, input)
  {
    array_list_push(args, input);
  }

  array_list_push(args, string("-o"));
  array_list_push(args, step->output);

  // Library search paths
  array_list_for_each(step->target->lib_paths, string_t, path)
  {
    array_list_push(args, string_concat(b->allocator, string("-L"), path));
  }

  // Link objects (libraries, frameworks, etc.)
  array_list_for_each(step->target->link_objects, link_object_t, obj)
  {
    switch(obj.kind) {
      case LINK_SYSTEM_LIB:
        array_list_push(args, string_concat(b->allocator, string("-l"), obj.name));
        break;
      case LINK_FRAMEWORK:
        array_list_push(args, string("-framework"));
        array_list_push(args, obj.name);
        break;
      case LINK_STATIC_PATH:
      case LINK_SHARED_PATH:
      case LINK_OBJECT_FILE:
        array_list_push(args, obj.path);
        break;
      case LINK_TARGET:
        // DAG handles build ordering; here we add the library and its transitive deps
        if(obj.target) {
          add_target_libs(b, obj.target, args);
        }
        break;
    }
  }

  // Linker options
  if(step->target->pie) {
    array_list_push(args, string("-pie"));
  }

  if(step->target->lto) {
    array_list_push(args, string("-flto"));
  }

  if(step->target->strip) {
    array_list_push(args, string("-s"));
  }

  // User link flags
  array_list_for_each(step->target->link_flags, string_t, flag)
  {
    array_list_push(args, flag);
  }

  cmd.program = linker;
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
  return "LINK";
}

static const build_step_api_t api = {
  .needs_rebuild = needs_rebuild,
  .on_complete = on_complete,
  .kind_str = kind_str
};

build_step_t* build_step_link(builder_t* b, target_t* t, build_step_t** compile_steps)
{
  build_step_t* step = build_step_create(b, t, STEP_COMPILE, t->name, &api);

  step->output = compute_output_path(b, t);
  step->language = LANG_C;

  size_t count = array_list_length(compile_steps);
  for(size_t i = 0; i < count; i++) {
    build_step_t* compile_step = compile_steps[i];
    array_list_push(step->inputs, compile_step->output);

    if(compile_step->language == LANG_CXX) {
      step->language = LANG_CXX;
    }
  }

  step->command = generate_cmd(b, step);

  return step;
}
