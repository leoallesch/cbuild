#include "build_step.h"
#include "builder.h"
#include "cbuild/types.h"
#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/mem/memory_context.h"
#include "core/string/string.h"
#include "depfile.h"

#include "core/os/path.h"
#include "core/os/process.h"
#include "target.h"

static process_command_t generate_cmd(build_step_t* step, builder_t* b)
{
  process_command_t cmd = { 0 };
  string_t compiler = get_compiler_for_lang(b, step->language);
  string_t* args = array_list(string_t, b->allocator);

  // [compiler] [flags...] -c [input] -o [output]
  array_list_push(args, compiler);

  // Preprocessor flags (apply to both C and C++)
  array_list_for_each(step->target->cpp_flags, string_t, flag)
  {
    array_list_push(args, flag);
  }

  // Language-specific compiler flags
  if(step->language == LANG_CXX) {
    array_list_for_each(step->target->cxx_flags, string_t, flag)
    {
      array_list_push(args, flag);
    }
  }
  else {
    array_list_for_each(step->target->c_flags, string_t, flag)
    {
      array_list_push(args, flag);
    }
  }

  array_list_push(args, optimize_to_flag(step->target->optimize));

  array_list_for_each(step->target->include_dirs, include_dir_t, inc)
  {
    array_list_push(args, get_include_flag(b->allocator, inc));
  }

  array_list_for_each(step->target->defines, string_t, def)
  {
    array_list_push(args, string_concat(b->allocator, string("-D"), def));
  }

  if(step->target->kind == TARGET_SHARED_LIB) {
    array_list_push(args, string("-fPIC"));
  }

  if(step->target->emit_deps) {
    array_list_push(args, string("-MMD"));
  }

  array_list_push(args, string("-c"));
  array_list_push(args, step->inputs[0]);
  array_list_push(args, string("-o"));
  array_list_push(args, step->output);

  cmd.program = compiler;
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

  array_list_for_each(step->header_deps, string_t, dep)
  {
    if(file_is_newer(dep, step->output)) {
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
  return "COMPILE";
}

static const build_step_api_t api = {
  .needs_rebuild = needs_rebuild,
  .on_complete = on_complete,
  .kind_str = kind_str
};

build_step_t* build_step_compile(builder_t* b, target_t* t, source_file_t src)
{
  build_step_t* step = build_step_create(b, t, STEP_COMPILE, path_basename(src.path), &api);

  if(src.language != LANG_AUTO && src.language != LANG_UNKNOWN) {
    step->language = src.language;
  }
  else {
    step->language = detect_language(src.path);
  }

  array_list_push(step->inputs, src.path);
  step->output = compute_compile_output_path(b, t, src.path, string(".o"));
  step->dep_path = compute_compile_output_path(b, t, src.path, string(".d"));
  step->command = generate_cmd(step, b);

  string_t depfile = compute_compile_output_path(b, t, src.path, string(".d"));
  step->header_deps = depfile_parse(depfile, b->allocator);

  return step;
}
