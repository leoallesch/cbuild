#pragma once

// ============================================================================
// CBuild - C Build System
// ============================================================================
//
// Include this single header in your build.c file to access the full API.
//
// Example build.c:
//
//   #include "cbuild/cbuild.h"
//
//   void build(builder_t* b)
//   {
//       allocator_t* alloc = builder_allocator(b);
//
//       target_t* app = target_executable(alloc, string("myapp"));
//       target_add_source(app, string("main.c"));
//       target_set_optimize(app, OPT_RELEASE);
//
//       builder_add_target(b, app);
//   }
//

#include "cbuild/builder.h"
#include "cbuild/target.h"
#include "cbuild/types.h"

// ============================================================================
// User Entry Point
// ============================================================================

// Implement this function in your build.c file.
// The builder is already initialized when this is called.
void build(builder_t* builder);
