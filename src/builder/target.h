#pragma once

#include "i_allocator.h"
#include "string_helper.h"
#include "utils.h"

#define import_target(name) extern target_t* CONCAT(name, _get_target)(i_allocator_t * allocator)
#define export_target(name) target_t* CONCAT(name, _get_target)(i_allocator_t * allocator)
#define get_target(name, alloc) CONCAT(name, _get_target(alloc))

typedef enum {
  TARGET_EXECUTABLE,
  TARGET_STATIC_LIB,
  TARGET_SHARED_LIB,
  TARGET_OBJECT,
} target_kind_t;

typedef enum {
  LANG_UNKNOWN,
  LANG_C,
  LANG_CXX,
  LANG_ASM,
  LANG_AUTO, // detect from extension
} source_lang_t;

typedef enum {
  OPT_NONE, // -O0
  OPT_DEBUG, // -Og
  OPT_RELEASE, // -O2
  OPT_FAST, // -O3
  OPT_SIZE, // -Os
  OPT_SIZE_MIN, // -Oz
} optimize_mode_t;

typedef enum {
  INCLUDE_PATH, // -I
  INCLUDE_SYSTEM, // -isystem
  INCLUDE_AFTER, // -idirafter
  INCLUDE_FRAMEWORK, // -F (macOS)
} include_kind_t;

typedef struct {
  string_t path;
  include_kind_t kind;
} include_dir_t;

typedef struct {
  string_t path;
  source_lang_t language;
  string_t* flags; // per-file extra flags (array_list)
} source_file_t;

typedef enum {
  LINK_TARGET, // another target_t (static/shared lib)
  LINK_SYSTEM_LIB, // -lfoo
  LINK_STATIC_PATH, // direct path to .a
  LINK_SHARED_PATH, // direct path to .so/.dylib
  LINK_FRAMEWORK, // -framework Foo (macOS)
  LINK_OBJECT_FILE, // raw .o file
} link_kind_t;

typedef struct target_t target_t;

typedef struct {
  link_kind_t kind;
  union {
    target_t* target; // LINK_TARGET
    string_t name; // LINK_SYSTEM_LIB, LINK_FRAMEWORK
    string_t path; // LINK_STATIC_PATH, LINK_SHARED_PATH, LINK_OBJECT_FILE
  };
} link_object_t;

typedef struct target_t {
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
  string_t* cpp_flags; // array_list: extra C preprocesser flags
  string_t* defines; // array_list: -DFOO or -DFOO=bar

  // Linking
  link_object_t* link_objects; // array_list of things to link
  string_t* link_flags; // array_list: extra linker flags
  string_t* lib_paths; // array_list: -L paths

  // Output
  string_t output_dir; // where to put artifacts
  string_t output_name; // override output filename (optional)

  // Build options
  bool pie; // position independent executable
  bool lto; // link-time optimization
  bool strip; // strip symbols
  bool emit_deps; // generate .d dependency files

  // Internal
  i_allocator_t* allocator;
} target_t;

// Creation
target_t* target_create(i_allocator_t* alloc, string_t name, target_kind_t kind);

// Convenience constructors
target_t* target_executable(i_allocator_t* alloc, string_t name);
target_t* target_static_lib(i_allocator_t* alloc, string_t name);
target_t* target_shared_lib(i_allocator_t* alloc, string_t name);

// Sources
void target_add_source(target_t* t, string_t path);
void target_add_source_with_flags(target_t* t, string_t path, source_lang_t lang, string_t* flags);
void _target_add_sources(target_t* t, string_t* paths, size_t count);
#define target_add_sources(target, ...)                      \
  do {                                                       \
    string_t _tmp[] = { __VA_ARGS__ };                       \
    _target_add_sources((target), _tmp, NUM_ELEMENTS(_tmp)); \
  } while(0)

void target_add_source_dir(target_t* t, string_t path);
void _target_add_source_dirs(target_t* t, string_t* paths, size_t count);
#define target_add_source_dirs(target, ...)                      \
  do {                                                           \
    string_t _tmp[] = { __VA_ARGS__ };                           \
    _target_add_source_dirs((target), _tmp, NUM_ELEMENTS(_tmp)); \
  } while(0)

// Include paths
void target_add_include(target_t* t, string_t path);
void target_add_include_system(target_t* t, string_t path);
void target_add_include_dir(target_t* t, string_t path, include_kind_t kind);

// Defines
void target_add_define(target_t* t, string_t define);
void target_add_define_value(target_t* t, string_t name, string_t value);

// Flags
void target_add_c_flag(target_t* t, string_t flag);
void target_add_cxx_flag(target_t* t, string_t flag);
void target_add_cpp_flag(target_t* t, string_t flag);
void target_add_link_flag(target_t* t, string_t flag);

// Linking
void target_link_target(target_t* t, target_t* dep);
void target_link_system_lib(target_t* t, string_t name);
void target_link_static(target_t* t, string_t path);
void target_link_shared(target_t* t, string_t path);
void target_add_lib_path(target_t* t, string_t path);

// Configuration
void target_set_optimize(target_t* t, optimize_mode_t mode);
void target_set_output_dir(target_t* t, string_t dir);
void target_set_output_name(target_t* t, string_t name);

// Output
void target_log_config(target_t* t);
