#pragma once

// ============================================================================
// Internal Build Step Definition
// ============================================================================

#include <stdbool.h>

#include "core/os/process.h"
#include "dag.h"
#include "target.h"

typedef enum {
  STEP_COMPILE,
  STEP_LINK,
  STEP_ARCHIVE,
  STEP_CUSTOM,
} step_kind_t;

typedef struct build_step_t build_step_t;

typedef struct {
  // Check if this step needs to run (default: check input/output timestamps)
  bool (*needs_rebuild)(build_step_t* step);

  // Called after successful execution (e.g., for execv on self-rebuild)
  void (*on_complete)(build_step_t* step, struct builder_t* b);

  // Get display string for logging
  const char* (*kind_str)(build_step_t* step);
} build_step_api_t;

typedef struct build_step_t {
  dag_node_t node;

  const build_step_api_t* api;
  step_kind_t kind;
  string_t name;

  target_t* target; // owning target
  string_t* inputs; // array_list
  string_t output;

  source_lang_t language;
  string_t* header_deps;
  string_t dep_path;

  process_command_t command;

  bool dirty;
  bool completed;
  process_result_t result;
} build_step_t;

// Forward declare builder_t to avoid circular include
typedef struct builder_t builder_t;

build_step_t* build_step_create(builder_t* b, target_t* t, step_kind_t kind, string_t name, const build_step_api_t* api);
build_step_t* build_step_compile(builder_t* b, target_t* t, source_file_t src);
build_step_t* build_step_link(builder_t* b, target_t* t, build_step_t** compile_steps);
build_step_t* build_step_archive(builder_t* b, target_t* t, build_step_t** compile_steps);

void build_step_log(build_step_t* step, bool verbose);

// misc helpers

source_lang_t detect_language(string_t path);
string_t compute_compile_output_path(builder_t* b, target_t* t, string_t source_path, string_t ext);
string_t compute_output_path(builder_t* b, target_t* t);
string_t get_compiler_for_lang(builder_t* b, source_lang_t lang);
string_t optimize_to_flag(optimize_mode_t mode);
string_t get_include_flag(allocator_t* alloc, include_dir_t inc);

static inline bool build_step_needs_rebuild(build_step_t* step)
{
  return step->api->needs_rebuild(step);
}

// Called after successful execution (e.g., for execv on self-rebuild)
static inline void build_step_on_complete(build_step_t* step, struct builder_t* b)
{
  return step->api->on_complete(step, b);
}

// Get display string for logging
static inline const char* build_step_kind_str(build_step_t* step)
{
  return step->api->kind_str(step);
}
