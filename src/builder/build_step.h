#pragma once

#include <stdbool.h>
#include "dag.h"
#include "process.h"
#include "string_helper.h"
#include "target.h"

typedef enum {
  STEP_COMPILE,
  STEP_LINK,
  STEP_ARCHIVE,
  STEP_CUSTOM,
} step_kind_t;

typedef struct build_step_t {
  dag_node_t node; // MUST be first for casting
  step_kind_t kind;
  string_t name;

  target_t* target; // owning target
  source_lang_t language; // for compile steps

  string_t* inputs; // array_list
  string_t output;
  string_t dep_path;

  process_command_t command;

  bool dirty;
  bool completed;
  process_result_t result;
} build_step_t;

// Forward declare builder_t to avoid circular include
typedef struct builder_t builder_t;

// ============================================================================
// Step Creation
// ============================================================================

build_step_t* build_step_create(builder_t* b, step_kind_t kind, string_t name);
build_step_t* build_step_compile(builder_t* b, target_t* t, source_file_t src);
build_step_t* build_step_link(builder_t* b, target_t* t, build_step_t** compile_steps);
build_step_t* build_step_archive(builder_t* b, target_t* t, build_step_t** compile_steps);

// ============================================================================
// Command Generation
// ============================================================================

process_command_t build_step_generate_compile_cmd(builder_t* b, build_step_t* step);
process_command_t build_step_generate_link_cmd(builder_t* b, build_step_t* step);
process_command_t build_step_generate_archive_cmd(builder_t* b, build_step_t* step);

// ============================================================================
// Helpers
// ============================================================================

string_t compute_object_path(builder_t* b, string_t source_path);
string_t compute_dep_path(builder_t* b, string_t source_path);
source_lang_t detect_language(string_t path);
string_t get_compiler_for_lang(builder_t* b, source_lang_t lang);
string_t optimize_to_flag(optimize_mode_t mode);

// ============================================================================
// Debug
// ============================================================================

void build_step_log(build_step_t* step);
