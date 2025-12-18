#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "array_list.h"
#include "build_step.h"
#include "builder.h"
#include "dag.h"
#include "hash_table.h"
#include "i_allocator.h"
#include "logger.h"
#include "memory_context.h"
#include "path.h"
#include "process.h"
#include "string_builder.h"
#include "string_helper.h"
#include "target.h"

// ============================================================================
// Helpers
// ============================================================================

string_t compute_object_path(builder_t* b, string_t source_path)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  string_t path = path_join(b->build_dir, source_path, temp_alloc);
  path = string_concat(b->allocator, path, string(".o"));

  scratch_end(scratch);

  return path;
}

string_t compute_dep_path(builder_t* b, string_t source_path)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  string_t path = path_join(b->build_dir, source_path, temp_alloc);
  path = string_concat(b->allocator, path, string(".d"));

  scratch_end(scratch);

  return path;
}

string_t compute_output_path(builder_t* b, target_t* t)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  string_t output_name;
  switch(t->kind) {
    case TARGET_SHARED_LIB:
      output_name = string_concat(temp_alloc, string("lib"), t->output_name);
      output_name = string_concat(temp_alloc, output_name, string(".so"));
      break;
    case TARGET_STATIC_LIB:
      output_name = string_concat(temp_alloc, string("lib"), t->output_name);
      output_name = string_concat(temp_alloc, output_name, string(".a"));
      break;
    case TARGET_EXECUTABLE:
    case TARGET_OBJECT:
    default:
      output_name = t->output_name;
      break;
  }

  string_t path = path_join(b->build_dir, t->output_dir, temp_alloc);
  path = path_join(path, output_name, b->allocator);

  scratch_end(scratch);

  return path;
}

source_lang_t detect_language(string_t path)
{
  string_t ext = path_extension(path);

  if(string_equals(ext, string(".c"))) {
    return LANG_C;
  }
  else if(
    string_equals(ext, string(".cpp")) ||
    string_equals(ext, string(".cxx")) ||
    string_equals(ext, string(".cc"))) {
    return LANG_CXX;
  }
  else if(string_equals(ext, string(".s")) || string_equals(ext, string(".S"))) {
    return LANG_ASM;
  }

  return LANG_UNKNOWN;
}

string_t get_compiler_for_lang(builder_t* b, source_lang_t lang)
{
  switch(lang) {
    case LANG_CXX:
      return b->toolchain.compiler_cxx;
    case LANG_ASM:
    case LANG_C:
    default:
      return b->toolchain.compiler_c;
  }
}

string_t optimize_to_flag(optimize_mode_t mode)
{
  switch(mode) {
    case OPT_DEBUG:
      return string("-Og");
    case OPT_RELEASE:
      return string("-O2");
    case OPT_FAST:
      return string("-O3");
    case OPT_SIZE:
      return string("-Os");
    case OPT_SIZE_MIN:
      return string("-Oz");
    case OPT_NONE:
    default:
      return string("-O0");
  }
}

static string_t get_include_flag(i_allocator_t* alloc, include_dir_t inc)
{
  switch(inc.kind) {
    case INCLUDE_SYSTEM:
      return string_concat(alloc, string("-isystem"), inc.path);
    case INCLUDE_AFTER:
      return string_concat(alloc, string("-idirafter"), inc.path);
    case INCLUDE_FRAMEWORK:
      return string_concat(alloc, string("-F"), inc.path);
    case INCLUDE_PATH:
    default:
      return string_concat(alloc, string("-I"), inc.path);
  }
}

static const char* step_kind_to_str(step_kind_t kind)
{
  switch(kind) {
    case STEP_COMPILE:
      return "COMPILE";
    case STEP_LINK:
      return "LINK";
    case STEP_ARCHIVE:
      return "ARCHIVE";
    case STEP_CUSTOM:
      return "CUSTOM";
    default:
      return "UNKNOWN";
  }
}

// ============================================================================
// Step Creation
// ============================================================================

build_step_t* build_step_create(builder_t* b, step_kind_t kind, string_t name)
{
  build_step_t* step = allocator_alloc(b->allocator, sizeof(build_step_t), alignof(build_step_t));

  step->kind = kind;
  step->name = name;
  step->target = NULL;
  step->language = LANG_UNKNOWN;
  step->inputs = array_list(string_t, b->allocator);
  step->output = string_empty();
  step->dep_path = string_empty();
  step->command = (process_command_t){ 0 };
  step->dirty = true;
  step->completed = false;
  step->result = (process_result_t){ 0 };

  return step;
}

build_step_t* build_step_compile(builder_t* b, target_t* t, source_file_t src)
{
  string_t file_name = path_basename(src.path);
  build_step_t* step = build_step_create(b, STEP_COMPILE, file_name);

  step->target = t;
  array_list_push(step->inputs, src.path);
  step->output = compute_object_path(b, src.path);
  step->dep_path = compute_dep_path(b, src.path);

  // Use explicit language if provided, otherwise detect
  if(src.language != LANG_AUTO && src.language != LANG_UNKNOWN) {
    step->language = src.language;
  }
  else {
    step->language = detect_language(src.path);
  }

  step->command = build_step_generate_compile_cmd(b, step);

  return step;
}

build_step_t* build_step_link(builder_t* b, target_t* t, build_step_t** compile_steps)
{
  build_step_t* step = build_step_create(b, STEP_LINK, t->name);

  step->target = t;
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

  step->command = build_step_generate_link_cmd(b, step);

  return step;
}

build_step_t* build_step_archive(builder_t* b, target_t* t, build_step_t** compile_steps)
{
  build_step_t* step = build_step_create(b, STEP_ARCHIVE, t->name);

  step->target = t;
  step->output = compute_output_path(b, t);
  step->language = LANG_C; // archiver doesn't care about language

  // Collect inputs from compile steps and add DAG edges
  size_t count = array_list_length(compile_steps);
  for(size_t i = 0; i < count; i++) {
    build_step_t* compile_step = compile_steps[i];
    array_list_push(step->inputs, compile_step->output);
  }

  step->command = build_step_generate_archive_cmd(b, step);

  return step;
}

// ============================================================================
// Command Generation
// ============================================================================

process_command_t build_step_generate_compile_cmd(builder_t* b, build_step_t* step)
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
    array_list_push(args, string("-MF"));
    array_list_push(args, step->dep_path);
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

process_command_t build_step_generate_link_cmd(builder_t* b, build_step_t* step)
{
  process_command_t cmd = { 0 };
  string_t linker = !string_isempty(b->toolchain.linker)
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
        // Target dependencies handled at DAG level
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

process_command_t build_step_generate_archive_cmd(builder_t* b, build_step_t* step)
{
  process_command_t cmd = { 0 };
  string_t archiver = !string_isempty(b->toolchain.archiver)
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

// ============================================================================
// Debug
// ============================================================================

void build_step_log(build_step_t* step)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  if(!step) {
    log_error("Cannot log step: step is NULL");
    return;
  }

  log_info("[%s] " STR_FMT " -> " STR_FMT,
    step_kind_to_str(step->kind),
    STR_ARG(step->name),
    STR_ARG(step->output));

  // Log command
  if(step->command.argv && array_list_length(step->command.argv) > 0) {
    string_builder_t sb = string_builder_init(temp_alloc);
    array_list_for_each(step->command.argv, string_t, arg)
    {
      string_builder_append(&sb, arg);
      string_builder_append(&sb, string(" "));
    }
    log_debug("  cmd: " STR_FMT, STR_ARG(string_builder_to_string(&sb)));
  }

  scratch_end(scratch);
}
