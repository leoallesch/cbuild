#pragma once

// ============================================================================
// CBuild Target API
// ============================================================================
//
// Targets represent build outputs: executables, static libraries, shared
// libraries, or object files. The target_t type is opaque - use the
// provided functions to configure targets.
//

#include "cbuild/types.h"
#include "core/alloc/allocator.h"
#include "core/string/string.h"

// ============================================================================
// Target Creation
// ============================================================================

target_t* target_create(allocator_t* alloc, string_t name, target_kind_t kind);
target_t* target_executable(allocator_t* alloc, string_t name);
target_t* target_static_lib(allocator_t* alloc, string_t name);
target_t* target_shared_lib(allocator_t* alloc, string_t name);

// ============================================================================
// Source Files
// ============================================================================

void target_add_source(target_t* t, string_t path);
void target_add_source_ex(target_t* t, string_t path, source_lang_t lang);
void target_add_source_dir(target_t* t, string_t dir);

// Variadic helpers - add multiple sources/dirs at once
#define target_add_sources(target, ...)                             \
  do {                                                              \
    string_t _srcs[] = { __VA_ARGS__ };                             \
    for(size_t _i = 0; _i < sizeof(_srcs) / sizeof(_srcs[0]); _i++) \
      target_add_source((target), _srcs[_i]);                       \
  } while(0)

#define target_add_source_dirs(target, ...)                         \
  do {                                                              \
    string_t _dirs[] = { __VA_ARGS__ };                             \
    for(size_t _i = 0; _i < sizeof(_dirs) / sizeof(_dirs[0]); _i++) \
      target_add_source_dir((target), _dirs[_i]);                   \
  } while(0)

// ============================================================================
// Include Paths
// ============================================================================

void target_add_include(target_t* t, string_t path);
void target_add_include_ex(target_t* t, string_t path, include_kind_t kind);

// Convenience for system includes
static inline void target_add_include_system(target_t* t, string_t path)
{
  target_add_include_ex(t, path, INCLUDE_SYSTEM);
}

// ============================================================================
// Preprocessor Defines
// ============================================================================

void target_add_define(target_t* t, string_t name);
void target_add_define_value(target_t* t, string_t name, string_t value);

// ============================================================================
// Compiler Flags
// ============================================================================

void target_add_c_flag(target_t* t, string_t flag);
void target_add_cxx_flag(target_t* t, string_t flag);
void target_add_cpp_flag(target_t* t, string_t flag);
void target_add_link_flag(target_t* t, string_t flag);

// ============================================================================
// Linking
// ============================================================================

void target_link_target(target_t* t, target_t* dependency);
void target_link_system_lib(target_t* t, string_t name);
void target_link_static(target_t* t, string_t path);
void target_link_shared(target_t* t, string_t path);
void target_add_lib_path(target_t* t, string_t path);

// ============================================================================
// Configuration
// ============================================================================

void target_set_optimize(target_t* t, optimize_mode_t mode);
void target_set_language(target_t* t, source_lang_t lang);
void target_set_artifacts_dir(target_t* t, string_t dir);
void target_set_bin_dir(target_t* t, string_t dir);
void target_set_output_name(target_t* t, string_t name);

// Build options
void target_set_pie(target_t* t, bool enable);
void target_set_lto(target_t* t, bool enable);
void target_set_strip(target_t* t, bool enable);

// ============================================================================
// Query
// ============================================================================

string_t target_get_name(target_t* t);
target_kind_t target_get_kind(target_t* t);

// ============================================================================
// Debug
// ============================================================================

void target_log_config(target_t* t);

// ============================================================================
// Cross-target references (for multi-file builds)
// ============================================================================

#define import_target(name) \
  extern target_t* name##_target(builder_t* b)

#define export_target(name) \
  target_t* name##_target(builder_t* b)
