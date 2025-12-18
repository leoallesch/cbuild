#pragma once

#include "build_step.h"
#include "dag.h"
#include "hash_table.h"
#include "i_allocator.h"
#include "target.h"
#include "toolchain.h"

typedef struct {
  size_t total;
  size_t completed;
  size_t failed;
  size_t skipped;
  bool success;
} build_result_t;

typedef struct builder_t {
  i_allocator_t* allocator;
  dag_t dag;

  target_t** targets; // array_list
  build_step_t** steps; // array_list

  hash_table_t output_map; // output path -> build_step_t*

  string_t build_dir;
  toolchain_t toolchain;

  bool stop_on_error;
  bool verbose;
} builder_t;

// ============================================================================
// Creation & Configuration
// ============================================================================

builder_t* builder_create(i_allocator_t* alloc);

void builder_set_build_dir(builder_t* b, string_t dir);
void builder_set_toolchain(builder_t* b, toolchain_t tc);
void builder_set_c_compiler(builder_t* b, string_t cc);
void builder_set_cxx_compiler(builder_t* b, string_t cxx);
void builder_set_assembler(builder_t* b, string_t assembler);
void builder_set_archiver(builder_t* b, string_t archiver);
void builder_set_linker(builder_t* b, string_t linker);
void builder_set_objcopy(builder_t* b, string_t objcopy);
void builder_set_size(builder_t* b, string_t size);
void builder_set_stop_on_error(builder_t* b, bool stop);
void builder_set_verbose(builder_t* b, bool verbose);

// ============================================================================
// Target Processing
// ============================================================================

void builder_add_target(builder_t* b, target_t* target);

// ============================================================================
// Query
// ============================================================================

build_step_t* builder_get_step_for_output(builder_t* b, string_t path);
build_step_t** builder_get_ready_steps(builder_t* b);
build_step_t** builder_get_root_steps(builder_t* b);

// ============================================================================
// Execution
// ============================================================================

build_result_t builder_run(builder_t* b);
bool builder_run_step(builder_t* b, build_step_t* step);
void builder_log_result(build_result_t result);

void builder_mark_complete(build_step_t* step);
bool builder_is_complete(builder_t* b);
void builder_reset(builder_t* b);

// ============================================================================
// Debug
// ============================================================================

void builder_log_steps(builder_t* b);
