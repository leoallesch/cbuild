#include "array_list.h"
#include "build_step.h"
#include "builder.h"
#include "dag.h"
#include "directory.h"
#include "hash_table.h"
#include "logger.h"
#include "path.h"
#include "process.h"
#include "string_helper.h"
#include "target.h"

#include <stdio.h>

builder_t* builder_create(i_allocator_t* alloc)
{
  builder_t* b = allocator_alloc(alloc, sizeof(builder_t), alignof(builder_t));
  dag_init(&b->dag, alloc);
  b->allocator = alloc;
  b->targets = array_list(target_t*, alloc);
  b->steps = array_list(build_step_t*, alloc);
  b->output_map = hash_table_init(alloc);
  b->build_dir = string("build");
  b->toolchain = (toolchain_t){
    .compiler_c = string("gcc"),
    .compiler_cxx = string("g++"),
    .assembler = string("as"),
    .archiver = string("gcc-ar"),
    .linker = string_empty(),
    .objcopy = string("objcopy"),
    .size = string("size")
  };
  b->stop_on_error = true;
  b->verbose = true;

  return b;
}

void builder_set_build_dir(builder_t* b, string_t dir)
{
  b->build_dir = dir;
}

void builder_set_toolchain(builder_t* b, toolchain_t tc)
{
  b->toolchain = tc;
}

void builder_set_c_compiler(builder_t* b, string_t cc)
{
  b->toolchain.compiler_c = cc;
}

void builder_set_cxx_compiler(builder_t* b, string_t cxx)
{
  b->toolchain.compiler_cxx = cxx;
}

void builder_set_assembler(builder_t* b, string_t assembler)
{
  b->toolchain.assembler = assembler;
}

void builder_set_archiver(builder_t* b, string_t archiver)
{
  b->toolchain.archiver = archiver;
}

void builder_set_linker(builder_t* b, string_t linker)
{
  b->toolchain.linker = linker;
}

void builder_set_objcopy(builder_t* b, string_t objcopy)
{
  b->toolchain.objcopy = objcopy;
}

void builder_set_size(builder_t* b, string_t size)
{
  b->toolchain.size = size;
}

void builder_set_stop_on_error(builder_t* b, bool stop)
{
  b->stop_on_error = stop;
}

void builder_set_verbose(builder_t* b, bool verbose)
{
  b->verbose = verbose;
}

void get_sources_from_source_dirs(target_t* target)
{
  array_list_for_each(target->source_dirs, string_t, dir)
  {
    directory_entry_t* entries = directory_read(dir, target->allocator);
    if(!entries) {
      continue;
    }
    array_list_for_each(entries, directory_entry_t, entry)
    {
      source_lang_t lang = detect_language(entry.path);
      if(lang != LANG_UNKNOWN) {
        source_file_t src = {
          .path = entry.path,
          .language = lang,
          .flags = NULL
        };
        array_list_push(target->sources, src);
      }
    }
  }
}

void builder_add_target(builder_t* b, target_t* target)
{
  get_sources_from_source_dirs(target);
  array_list_push(b->targets, target);

  // Create compile steps for this target's sources
  build_step_t** compile_steps = array_list(build_step_t*, b->allocator);

  array_list_for_each(target->sources, source_file_t, src)
  {
    build_step_t* compile_step = build_step_compile(b, target, src);
    dag_add_node(&b->dag, &compile_step->node);
    array_list_push(b->steps, compile_step);
    array_list_push(compile_steps, compile_step);
    hash_table_set(&b->output_map, compile_step->output, compile_step);
  }

  // Create output step based on target kind
  build_step_t* output_step = NULL;

  switch(target->kind) {
    case TARGET_EXECUTABLE:
    case TARGET_SHARED_LIB:
      output_step = build_step_link(b, target, compile_steps);
      break;
    case TARGET_STATIC_LIB:
      output_step = build_step_archive(b, target, compile_steps);
      break;
    case TARGET_OBJECT:
      // No link/archive step needed for object-only targets
      break;
  }

  if(output_step) {
    // Must add node to DAG first (initializes dependencies/dependents arrays)
    dag_add_node(&b->dag, &output_step->node);

    // Add DAG edges: output step depends on compile steps
    size_t count = array_list_length(compile_steps);
    for(size_t i = 0; i < count; i++) {
      dag_add_edge(&output_step->node, &compile_steps[i]->node);
    }

    array_list_push(b->steps, output_step);
    hash_table_set(&b->output_map, output_step->output, output_step);

    // Handle LINK_TARGET dependencies (output_step depends on dep_step)
    array_list_for_each(target->link_objects, link_object_t, obj)
    {
      if(obj.kind == LINK_TARGET) {
        build_step_t* dep_step = hash_table_get(&b->output_map, obj.path);
        if(dep_step) {
          dag_add_edge(&output_step->node, &dep_step->node);
        }
      }
    }
  }
}

build_step_t* builder_get_step_for_output(builder_t* b, string_t path)
{
  return hash_table_get(&b->output_map, path);
}

build_step_t** builder_get_ready_steps(builder_t* b)
{
  build_step_t** list = array_list(build_step_t*, b->allocator);
  array_list_for_each(b->steps, build_step_t*, step)
  {
    if(!step->completed) {
      array_list_push(list, step);
    }
  }

  return list;
}

build_step_t** builder_get_root_steps(builder_t* b)
{
  return (build_step_t**)dag_get_roots(&b->dag, b->allocator);
}

void builder_mark_complete(build_step_t* step)
{
  step->completed = true;
}

bool builder_is_complete(builder_t* b)
{
  bool complete = true;
  array_list_for_each(b->steps, build_step_t*, step)
  {
    if(!step->completed) {
      complete = false;
      break;
    }
  }
  return complete;
}

void builder_reset(builder_t* b)
{
  array_list_for_each(b->steps, build_step_t*, step)
  {
    step->dirty = true;
    step->completed = false;
  }
}

// ============================================================================
// Execution
// ============================================================================

build_result_t builder_run(builder_t* b)
{
  build_result_t result = { 0 };
  dag_node_t** sorted = dag_topo_sort(&b->dag, b->allocator);

  if(!sorted) {
    log_error_tag("BUILD", "Dependency cycle detected");
    result.success = false;
    return result;
  }

  result.total = array_list_length(sorted);
  log_info_tag("BUILD", "--------------------------------------------------");

  array_list_for_each(sorted, dag_node_t*, node)
  {
    build_step_t* step = (build_step_t*)node;

    if(step->completed || !step->dirty) {
      result.skipped++;
      continue;
    }

    if(builder_run_step(b, step)) {
      result.completed++;
    }
    else {
      result.failed++;
      if(b->stop_on_error) {
        break;
      }
    }
  }

  result.success = (result.failed == 0);
  return result;
}

bool builder_run_step(builder_t* b, build_step_t* step)
{
  string_t dir = path_dirname(step->output);
  directory_create_recursive(dir);

  if(b->verbose) {
    build_step_log(step);
  }

  step->result = process_run(step->command, b->allocator);

  if(step->result.error != PROCESS_OK || step->result.output.exit_code != 0) {
    log_error_tag("BUILD", STR_FMT, STR_ARG(step->result.output.stderr));
    return false;
  }

  step->completed = true;
  step->dirty = false;
  return true;
}

void builder_log_result(build_result_t result)
{
  log_info_tag("BUILD", "--------------------------------------------------");
  if(result.success) {
    log_info_tag("BUILD", "Build succeeded: %zu completed, %zu skipped",
      result.completed, result.skipped);
  }
  else {
    log_error_tag("BUILD", "Build failed: %zu completed, %zu failed, %zu skipped",
      result.completed, result.failed, result.skipped);
  }
}

void builder_log_steps(builder_t* b)
{
  array_list_for_each(b->steps, build_step_t*, step)
  {
    build_step_log(step);
  }
}
