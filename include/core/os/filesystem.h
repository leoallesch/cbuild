#pragma once

// Filesystem API - convenience header that includes all components
//
// For granular includes, use:
//   - file.h      - file operations (read, write, stat, copy)
//   - directory.h - directory operations (create, walk, list)
//   - path.h      - path manipulation (join, normalize, basename)

#include "core/os/directory.h"
#include "core/os/file.h"
#include "core/os/path.h"
