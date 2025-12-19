#pragma once

// ============================================================================
// Internal Builder Definition
// ============================================================================

#include "cbuild/builder.h"
#include "core/alloc/allocator.h"

#include "build_step.h"
#include "core/container/hash_table.h"
#include "core/os/datetime.h"
#include "dag.h"

// ============================================================================
// Build Result
// ============================================================================

typedef struct {
  size_t total;
  size_t completed;
  size_t failed;
  size_t skipped;
  double duration;
  bool success;
} build_result_t;

// ============================================================================
// Hook Steps
// ============================================================================

typedef struct builder_t builder_t;

typedef struct hook_step_t {
  string_t name; // Display name for logging
  string_t* inputs; // Files to check for changes (array_list)
  string_t output; // Output file (for timestamp checking)
  void* ctx;

  // Callbacks
  bool (*run)(builder_t* b, struct hook_step_t* h); // Execute the hook, return true on success
  void (*on_complete)(builder_t* b, struct hook_step_t* h); // Called after successful run (e.g., execv for self-rebuild)
} hook_step_t;

// ============================================================================
// Builder Structure (Internal)
// ============================================================================

struct builder_t {
  allocator_t* allocator;
  dag_t dag;

  string_t config_file_path; // (default) ./build.c

  target_t** targets; // array_list
  build_step_t** steps; // array_list

  hook_step_t** pre_hooks; // array_list - run before main build
  hook_step_t** post_hooks; // array_list - run after main build

  hash_table_t output_map; // output path -> build_step_t*

  string_t build_dir;
  toolchain_t toolchain;

  instant_t init_time;

  bool stop_on_error;
  bool verbose;

  // For self-rebuild execv()
  int argc;
  char** argv;
};

// ============================================================================
// Creation & Configuration
// ============================================================================

builder_t* builder_create(allocator_t* alloc);

// Internal: add hooks
void builder_add_pre_hook_internal(builder_t* b, hook_step_t* hook);
void builder_add_post_hook_internal(builder_t* b, hook_step_t* hook);

// Built-in hooks
hook_step_t* hook_self_rebuild(builder_t* b);
hook_step_t* hook_clean(builder_t* b);
hook_step_t* hook_compile_commands(builder_t* b, target_t* t, build_step_t** compile_steps);

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
