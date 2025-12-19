#pragma once

// ============================================================================
// Internal Target Definition
// ============================================================================
//
// This header contains the full target_t struct definition and internal
// types. Only used by implementation files - users see target_t as opaque.
//

#include "cbuild/target.h"
#include "core/alloc/allocator.h"

// ============================================================================
// Internal Types
// ============================================================================

typedef struct {
  string_t path;
  include_kind_t kind;
} include_dir_t;

typedef struct {
  string_t path;
  source_lang_t language;
  string_t* flags; // per-file extra flags (array_list)
} source_file_t;

typedef struct {
  link_kind_t kind;
  union {
    struct target_t* target; // LINK_TARGET
    string_t name; // LINK_SYSTEM_LIB, LINK_FRAMEWORK
    string_t path; // LINK_STATIC_PATH, LINK_SHARED_PATH, LINK_OBJECT_FILE
  };
} link_object_t;

// ============================================================================
// Target Structure (Internal)
// ============================================================================

struct target_t {
  string_t name;
  target_kind_t kind;

  // Sources
  source_file_t* sources; // array_list of source files
  string_t* source_dirs; // array_list of source dirs
  include_dir_t* include_dirs; // array_list of include directories

  // Compilation
  source_lang_t default_lang;
  optimize_mode_t optimize;
  string_t* c_flags; // array_list: extra C compiler flags
  string_t* cxx_flags; // array_list: extra C++ compiler flags
  string_t* cpp_flags; // array_list: extra C preprocessor flags
  string_t* defines; // array_list: -DFOO or -DFOO=bar

  // Linking
  link_object_t* link_objects; // array_list of things to link
  string_t* link_flags; // array_list: extra linker flags
  string_t* lib_paths; // array_list: -L paths

  // Output
  string_t bin_dir;
  string_t artifacts_dir;
  string_t output_name; // override output filename (optional)

  // Build options
  bool pie; // position independent executable
  bool lto; // link-time optimization
  bool strip; // strip symbols
  bool emit_deps; // generate .d dependency files

  // Internal
  allocator_t* allocator;
};
