#pragma once

// ============================================================================
// CBuild Builder API
// ============================================================================
//
// The builder orchestrates the build process. It manages targets, generates
// build steps, and executes them in dependency order.
//
// Users receive a builder_t* in their build() function and use it to:
// - Get an allocator for creating targets
// - Configure build settings (build dir, toolchain)
// - Add targets to the build
//

#include "cbuild/types.h"
#include "core/string/string.h"

// ============================================================================
// Allocator Access
// ============================================================================

// Get the allocator for creating targets and other build objects.
// This allocator is valid for the duration of the build.
allocator_t* builder_allocator(builder_t* b);

// ============================================================================
// Build Configuration
// ============================================================================

// Set the config file path ./build.c (default) -> ./i_love_cbuild.c
void builder_set_config_file(builder_t* b, string_t path);

// Set the root build directory (default: "build")
void builder_set_build_dir(builder_t* b, string_t dir);

// Stop building on first error (default: true)
void builder_set_stop_on_error(builder_t* b, bool stop);

// Print full commands when building (default: false)
void builder_set_verbose(builder_t* b, bool verbose);

// ============================================================================
// Toolchain Configuration
// ============================================================================

// Set all tools at once
typedef struct {
  string_t c_compiler; // default: "cc"
  string_t cxx_compiler; // default: "c++"
  string_t assembler; // default: "as"
  string_t archiver; // default: "ar"
  string_t linker; // default: "cc"
  string_t objcopy; // default: "objcopy"
  string_t size; // default: "size"
} toolchain_t;

void builder_set_toolchain(builder_t* b, toolchain_t tc);

// Set individual tools
void builder_set_c_compiler(builder_t* b, string_t compiler);
void builder_set_cxx_compiler(builder_t* b, string_t compiler);
void builder_set_assembler(builder_t* b, string_t assembler);
void builder_set_archiver(builder_t* b, string_t archiver);
void builder_set_linker(builder_t* b, string_t linker);

// ============================================================================
// Target Management
// ============================================================================

// Add a target to the build. The builder takes ownership.
void builder_add_target(builder_t* b, target_t* target);
