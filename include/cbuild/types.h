#pragma once

// ============================================================================
// CBuild Public Types
// ============================================================================
//
// This header contains forward declarations and opaque type definitions.
// Users work with pointers to these types - internal details are hidden.
//

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// Opaque Types
// ============================================================================

// Build system orchestrator - manages targets and executes builds
typedef struct builder_t builder_t;

// Build target - represents an executable, library, or object file
typedef struct target_t target_t;

// ============================================================================
// Target Types
// ============================================================================

typedef enum {
  TARGET_EXECUTABLE,
  TARGET_STATIC_LIB,
  TARGET_SHARED_LIB,
  TARGET_OBJECT,
} target_kind_t;

// ============================================================================
// Language Types
// ============================================================================

typedef enum {
  LANG_UNKNOWN,
  LANG_C,
  LANG_CXX,
  LANG_ASM,
  LANG_AUTO, // detect from file extension
} source_lang_t;

// ============================================================================
// Optimization Levels
// ============================================================================

typedef enum {
  OPT_NONE, // -O0
  OPT_DEBUG, // -Og
  OPT_RELEASE, // -O2
  OPT_FAST, // -O3
  OPT_SIZE, // -Os
  OPT_SIZE_MIN, // -Oz
} optimize_mode_t;

// ============================================================================
// Include Path Types
// ============================================================================

typedef enum {
  INCLUDE_PATH, // -I (normal include path)
  INCLUDE_SYSTEM, // -isystem (system include, suppresses warnings)
  INCLUDE_AFTER, // -idirafter (searched after normal includes)
  INCLUDE_FRAMEWORK, // -F (macOS framework path)
} include_kind_t;

// ============================================================================
// Link Types
// ============================================================================

typedef enum {
  LINK_TARGET, // another target_t (static/shared lib)
  LINK_SYSTEM_LIB, // -lfoo (system library)
  LINK_STATIC_PATH, // direct path to .a file
  LINK_SHARED_PATH, // direct path to .so/.dylib file
  LINK_FRAMEWORK, // -framework Foo (macOS)
  LINK_OBJECT_FILE, // raw .o file
} link_kind_t;
