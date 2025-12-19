# CBuild - C Build System

A self-hosting C build system where builds are defined in C code rather than configuration files.

## Quick Reference

### Building the Project

```bash
# Option 1: Bootstrap from scratch
make -f lib.mk        # Build libcbuild.a
./bootstrap.sh        # Create cbuild executable

# Option 2: If cbuild already exists
./cbuild              # Self-build
```

### Project Structure

```
cbuild/
├── include/cbuild/          # PUBLIC API (users include these)
│   ├── cbuild.h            # Single include for users
│   ├── types.h             # Opaque types (builder_t, target_t, allocator_t)
│   ├── string.h            # string_t type and operations
│   ├── target.h            # Target configuration API
│   └── builder.h           # Builder API + toolchain_t
│
├── src/
│   ├── internal/            # INTERNAL headers (implementation details)
│   │   ├── allocator.h     # allocator_t struct definition
│   │   ├── target_internal.h # target_t struct + internal types
│   │   ├── string_internal.h # Internal string functions + compat macros
│   │   └── utils.h         # CONTAINER_OF, OFFSET_OF macros
│   │
│   ├── main.c              # Entry point
│   ├── builder/            # Build orchestration
│   │   ├── builder.c/h     # Builder core - manages targets and DAG
│   │   ├── build_step.c/h  # Compile/link/archive step abstraction
│   │   ├── target.c        # Target configuration implementation
│   │   └── depfile.c/h     # .d file parser for incremental builds
│   ├── cli/                # Command-line argument parsing
│   ├── core/               # Core utilities
│   │   ├── allocators/     # Arena allocator
│   │   ├── containers/     # array_list, hash_table, stack
│   │   ├── logger/         # Leveled logging system
│   │   ├── memory/         # Memory context, scratch allocators
│   │   ├── os/             # File, directory, process, path abstractions
│   │   └── string/         # string_t implementation, string_builder
│   ├── graph/              # DAG for dependency ordering
│   └── platform/           # Platform-specific implementations
│       ├── posix/          # Linux/macOS
│       └── win32/          # Windows (stubs)
│
├── examples/               # Usage examples
├── build.c                 # Self-build definition
├── bootstrap.sh            # Bootstrap script
└── lib.mk                  # Static library makefile
```

## API Design Philosophy

### Public vs Internal Headers

**Public API** (`include/cbuild/`):
- Users include `#include "cbuild/cbuild.h"` in their `build.c`
- Types are **opaque** - users work with pointers, not struct internals
- Clean, stable API surface

**Internal Headers** (`src/internal/`):
- Only for cbuild implementation code
- Contains struct definitions, internal functions
- Can change without breaking user code

### Opaque Types

Users see these as opaque pointers:
- `builder_t*` - Build orchestrator
- `target_t*` - Build target
- `allocator_t*` - Memory allocator

They get an allocator from `builder_allocator(b)` and pass it to target creation.

## Core Concepts

### Memory Management (3-tier system)

1. **Virtual Memory Backend** (`mem_t`) - Platform mmap/VirtualAlloc
2. **Arena Allocator** - Bump allocation, no individual frees
3. **Scratch Allocators** - Temporary allocations with `scratch_begin()`/`scratch_end()`

```c
// Scratch allocator usage
scratch_scope(scratch) {
    allocator_t* alloc = scratch_allocator(scratch);
    string_t temp = string_copy(alloc, some_string);
    // ... use temp ...
} // automatically freed
```

### String Handling

Non-owning string views (`string_t`) with pointer + length:

```c
string_t s = string("hello");           // From literal
printf(STR_FMT "\n", STR_ARG(s));       // Print with format
string_t sub = string_substr(s, 0, 3);  // "hel" - zero-copy
```

### Containers

- `array_list` - Dynamic array with header-based metadata
- `hash_table` - String-keyed, chaining collision

```c
string_t* list = array_list(string_t, alloc);
array_list_push(list, string("item"));
array_list_for_each(list, string_t, item) {
    // use item
}
```

### Build Graph (DAG)

- Nodes = build steps (compile, link, archive)
- Edges = dependencies
- Topological sort for execution order
- Cycle detection

## Build Definition API

### Target Types

```c
target_t* exe = target_executable(alloc, string("myapp"));
target_t* lib = target_static_lib(alloc, string("mylib"));
target_t* so  = target_shared_lib(alloc, string("myshared"));
```

### Target Configuration

```c
// Sources
target_add_source(t, string("main.c"));
target_add_source_dir(t, string("src"));
target_add_sources(t, string("a.c"), string("b.c"));

// Includes
target_add_include(t, string("include"));
target_add_include_system(t, string("/usr/include"));

// Defines
target_add_define(t, string("DEBUG"));
target_add_define_value(t, string("VERSION"), string("\"1.0\""));

// Flags
target_add_c_flag(t, string("-Wall"));
target_add_cxx_flag(t, string("-std=c++17"));

// Optimization: OPT_NONE, OPT_DEBUG, OPT_RELEASE, OPT_FAST, OPT_SIZE
target_set_optimize(t, OPT_DEBUG);

// Linking
target_link_target(t, other_target);        // Link another target
target_link_system_lib(t, string("pthread")); // -lpthread
```

### Builder Usage

```c
#include "cbuild/cbuild.h"

void build(builder_t* b)
{
    allocator_t* alloc = builder_allocator(b);
    builder_set_build_dir(b, string("build"));

    target_t* t = target_executable(alloc, string("myapp"));
    // ... configure target ...

    builder_add_target(b, t);
}
```

## CLI Commands

```bash
./cbuild              # Build (default)
./cbuild clean        # Remove build artifacts
./cbuild rebuild      # Clean and build
./cbuild -v           # Verbose output
./cbuild -f FILE      # Use alternate build.c
```

## Build Output Structure

```
build/
├── artifacts/{target}/   # Object files (.o) and deps (.d)
└── bin/                  # Final executables and libraries
```

## Key Files Reference

| File | Purpose |
|------|---------|
| `include/cbuild/cbuild.h` | Single-include public API |
| `include/cbuild/target.h` | Target API (opaque) |
| `include/cbuild/builder.h` | Builder API + toolchain_t |
| `src/internal/target_internal.h` | target_t struct definition |
| `src/internal/allocator.h` | allocator_t struct definition |
| `src/builder/builder.c` | Build orchestration, runs DAG |
| `src/builder/build_step.c` | Compile/link command generation |
| `src/builder/target.c` | Target configuration implementation |
| `src/graph/dag.c` | Topological sort, cycle detection |

## Incremental Build

- Checks output file timestamps vs inputs
- Parses `.d` files for header dependencies
- Skips up-to-date files automatically

## Platform Support

- **POSIX** (Linux, macOS): Full implementation
- **Windows**: Stubs present in `src/platform/win32/`

## Design Patterns

1. **Opaque types** - Public API uses forward declarations, internals hidden
2. **Interface-based abstractions** - `allocator_t`, `mem_t`
3. **Result types** - Functions return structs with error + value
4. **Intrusive data structures** - DAG nodes embedded in build steps
5. **Header-based array metadata** - Length/capacity stored before data pointer

## Notes for Development

### Include Guidelines

**In user's build.c:**
```c
#include "cbuild/cbuild.h"  // All you need
```

**In cbuild source files:**
```c
#include "internal/allocator.h"       // For allocator_t definition
#include "internal/target_internal.h" // For target_t definition
#include "internal/string_internal.h" // For string funcs + compat macros
#include "cbuild/string.h"            // If only need string_t type
```

### Key Points

- All allocations go through `allocator_t` interface
- Use scratch allocators for temporary work
- String literals: wrap with `string("...")` macro
- Printf strings: use `STR_FMT` and `STR_ARG(s)`
- Build steps are DAG nodes (first member is `dag_node_t`)
- Hash table is string-keyed, no resize - set capacity appropriately
