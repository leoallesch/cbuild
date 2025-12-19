#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "core/alloc/allocator.h"
#include "core/container/stack.h"
#include "core/os/file.h"
#include "core/string/string.h"

typedef struct {
  string_t path;
  string_t name;
  file_type_t type;
  size_t depth;
} directory_entry_t;

typedef struct {
  cb_stack_t stack;
  allocator_t* allocator;
  int depth;
  directory_entry_t current_entry;
  string_t root_path;
  string_t current_path;
} directory_walker_t;

// Directory listing
directory_entry_t* directory_read(string_t path, allocator_t* allocator);

// Directory walking
directory_walker_t directory_walk_begin(string_t path, allocator_t* allocator);
bool directory_walk_next(directory_walker_t* walker);
directory_entry_t directory_walk_current(directory_walker_t* walker);
void directory_walk_end(directory_walker_t* walker);

#define directory_walk(path, entry, allocator, ...)                    \
  do {                                                                 \
    directory_walker_t walker = directory_walk_begin(path, allocator); \
    while(directory_walk_next(&walker)) {                              \
      directory_entry_t entry = directory_walk_current(&walker);       \
      __VA_ARGS__                                                      \
    }                                                                  \
    directory_walk_end(&walker);                                       \
  } while(0)

// Directory manipulation
bool directory_exists(string_t path);
bool directory_create(string_t path);
bool directory_create_recursive(string_t path);
bool directory_delete(string_t path);
bool directory_delete_recursive(string_t path);
bool directory_rename(string_t old_path, string_t new_path);

// Working directory
string_t directory_cwd(allocator_t* allocator);
bool directory_change(string_t path);
