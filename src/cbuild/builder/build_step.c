#include <stdbool.h>
#include "build_step.h"
#include "builder.h"
#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/logger/logger.h"
#include "core/mem/memory_context.h"
#include "core/string/string.h"

#include "core/os/path.h"
#include "core/os/process.h"
#include "core/string/string_builder.h"

// ============================================================================
// Helpers
// ============================================================================

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

string_t compute_compile_output_path(builder_t* b, target_t* t, string_t source_path, string_t ext)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp_alloc = scratch_allocator(scratch);

  string_t path = path_join(b->build_dir, t->artifacts_dir, temp_alloc);
  path = path_join(path, source_path, temp_alloc);
  path = string_concat(b->allocator, path, ext);

  scratch_end(scratch);

  return path;
}

string_t compute_output_path(builder_t* b, target_t* t)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp_alloc = scratch_allocator(scratch);

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

  string_t path = path_join(b->build_dir, t->bin_dir, temp_alloc);
  path = path_join(path, output_name, b->allocator);

  scratch_end(scratch);

  return path;
}

string_t get_compiler_for_lang(builder_t* b, source_lang_t lang)
{
  switch(lang) {
    case LANG_CXX:
      return b->toolchain.cxx_compiler;
    case LANG_ASM:
    case LANG_C:
    default:
      return b->toolchain.c_compiler;
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

string_t get_include_flag(allocator_t* alloc, include_dir_t inc)
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

build_step_t* build_step_create(builder_t* b, target_t* t, step_kind_t kind, string_t name, const build_step_api_t* api)
{
  build_step_t* step = allocator_alloc(b->allocator, sizeof(build_step_t), alignof(build_step_t));

  step->api = api;
  step->kind = kind;
  step->name = name;
  step->target = t;
  step->inputs = array_list(string_t, b->allocator);
  step->output = string_empty();
  step->command = (process_command_t){ 0 };
  step->dirty = true;
  step->completed = false;
  step->result = (process_result_t){ 0 };

  return step;
}

// ============================================================================
// Debug
// ============================================================================

void build_step_log(build_step_t* step, bool verbose)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp_alloc = scratch_allocator(scratch);

  if(!step) {
    log_error("Cannot log step: step is NULL");
    return;
  }

  log_info_tag(build_step_kind_str(step),
    STR_FMT " -> " STR_FMT,
    STR_ARG(step->name),
    STR_ARG(step->output));

  if(verbose) {
    if(step->command.argv && array_list_length(step->command.argv) > 0) {
      string_builder_t sb = string_builder_init(temp_alloc);
      array_list_for_each(step->command.argv, string_t, arg)
      {
        string_builder_append(&sb, arg);
        string_builder_append(&sb, string(" "));
      }
      log_info_tag(build_step_kind_str(step), STR_FMT, STR_ARG(string_builder_to_string(&sb)));
    }
  }

  scratch_end(scratch);
}
