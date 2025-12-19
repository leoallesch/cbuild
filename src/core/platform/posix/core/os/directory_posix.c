#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/logger/logger.h"
#include "core/mem/memory_context.h"
#include "core/os/directory.h"
#include "core/string/string.h"

#include "core/os/path.h"
#include "core/string/string_builder.h"

#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define walker_level_current(walker) stack_peek(&walker->stack, walker_level_t, element)

typedef struct
{
  stack_element_t element;
  DIR* dir_handle;
  string_t path;
} walker_level_t;

// Defined in file_posix.c
extern file_type_t file_type_from_mode(__mode_t mode);

static file_type_t file_type_from_dirent(unsigned char type)
{
  return file_type_from_mode(DTTOIF(type));
}

static DIR* directory_open(string_t path)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp_alloc = scratch_allocator(scratch);
  string_t normalized = path_normalize(path, temp_alloc);
  const char* cstr = string_to_cstr(temp_alloc, normalized);

  DIR* dir = opendir(cstr);

  scratch_end(scratch);

  return dir;
}

directory_entry_t* directory_read(string_t path, allocator_t* allocator)
{
  DIR* dir = directory_open(path);
  if(!dir) {
    log_error("%s", strerror(errno));
    return NULL;
  }

  directory_entry_t* entries = array_list(directory_entry_t, allocator);

  struct dirent* d;

  while((d = readdir(dir)) != NULL) {
    string_t name = string_from_cstr(d->d_name);
    if(!string_equals(name, string("..")) && !string_equals(name, string("."))) {
      directory_entry_t e = {
        .path = path_join(path, name, allocator),
        .name = name,
        .type = file_type_from_dirent(d->d_type),
        .depth = 1
      };
      array_list_push(entries, e);
    }
  }

  closedir(dir);
  return entries;
}

static walker_level_t* new_walker_level(string_t path, allocator_t* allocator)
{
  DIR* dir = directory_open(path);
  if(!dir) {
    log_error("%s", strerror(errno));
    return NULL;
  }

  walker_level_t* level = allocator_alloc(allocator, sizeof(walker_level_t), alignof(walker_level_t));

  level->dir_handle = dir;
  level->path = path;

  return level;
}

directory_walker_t directory_walk_begin(string_t path, allocator_t* allocator)
{
  walker_level_t* level = new_walker_level(path, allocator);
  if(!level) {
    log_error("Directory walker not initalized correctly!");
    return (directory_walker_t){ 0 };
  }

  directory_walker_t walker;
  stack_init(&walker.stack);
  stack_push(&walker.stack, &level->element);

  walker.depth = 0;
  walker.root_path = string_copy(allocator, path);
  walker.allocator = allocator;

  return walker;
}

bool directory_walk_next(directory_walker_t* walker)
{
  while(walker->depth >= 0) {
    walker_level_t* curr = walker_level_current(walker);

    struct dirent* entry = readdir(curr->dir_handle);

    if(entry == NULL) {
      closedir(curr->dir_handle);
      _stack_pop(&walker->stack);
      walker->depth--;
      continue;
    }

    string_t name = string_from_cstr(entry->d_name);

    if(!string_equals(name, string("..")) && !string_equals(name, string("."))) {
      string_t new_path = path_join(curr->path, name, walker->allocator);
      walker->current_path = new_path;

      directory_entry_t new_entry = {
        .path = new_path,
        .name = name,
        .type = file_type_from_dirent(entry->d_type),
        .depth = walker->depth
      };

      walker->current_entry = new_entry;

      if(new_entry.type == FILE_TYPE_DIRECTORY) {
        walker->depth++;
        walker_level_t* new_level = new_walker_level(new_path, walker->allocator);
        stack_push(&walker->stack, &new_level->element);
      }
      return true;
    }
  }
  return false;
}

directory_entry_t directory_walk_current(directory_walker_t* walker)
{
  return walker->current_entry;
}

void directory_walk_end(directory_walker_t* walker)
{
  while(walker->depth >= 0) {
    walker_level_t* level = stack_pop(&walker->stack, walker_level_t, element);
    closedir(level->dir_handle);
    walker->depth--;
  }
}

bool directory_create(string_t path)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp = scratch_allocator(scratch);
  string_t normalized = path_normalize(path, temp);
  const char* cstr = string_to_cstr(temp, normalized);

  int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

  int err = mkdir(cstr, mode);
  bool res = true;

  scratch_end(scratch);

  if(err) {
    switch(errno) {
      case EEXIST:
        break;
      default:
        res = false;
        break;
    }
  }

  return res;
}

bool directory_create_recursive(string_t path)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp = scratch_allocator(scratch);
  string_builder_t sb = string_builder_init(temp);

  string_t normalized = path_normalize(path, temp);
  string_t* parts = string_split(temp, normalized, '/', false);
  bool res = true;
  array_list_for_each(parts, string_t, item)
  {
    string_builder_append(&sb, item);
    if(!directory_create(string_builder_to_string(&sb))) {
      res = false;
      goto end;
    }
    string_builder_append(&sb, string("/"));
  }

end:
  scratch_end(scratch);
  return res;
}

bool directory_delete(string_t path)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp = scratch_allocator(scratch);
  string_t normalized = path_normalize(path, temp);
  const char* cstr = string_to_cstr(temp, normalized);

  int err = rmdir(cstr);

  if(err) {
    printf("path: " STR_FMT "\nerror: %s\n", STR_ARG(path), strerror(errno));
  }

  scratch_end(scratch);

  return (err == 0);
}

bool directory_delete_recursive(string_t path)
{
  if(!directory_exists(path)) {
    return false;
  }

  scratch_t* scratch = scratch_begin();
  allocator_t* temp = scratch_allocator(scratch);

  DIR* dir = directory_open(path);
  if(!dir) {
    scratch_end(scratch);
    return false;
  }

  bool success = true;
  struct dirent* entry;

  while((entry = readdir(dir)) != NULL && success) {
    string_t name = string_from_cstr(entry->d_name);
    if(string_equals(name, string(".")) || string_equals(name, string(".."))) {
      continue;
    }

    string_t full_path = path_join(path, name, temp);

    if(entry->d_type == DT_DIR) {
      success = directory_delete_recursive(full_path); // Recursive call gets new scratch
    }
    else {
      success = file_delete(full_path);
    }
  }

  closedir(dir);
  scratch_end(scratch);

  if(success) {
    success = directory_delete(path);
  }

  return success;
}

bool directory_exists(string_t path)
{
  DIR* dir = directory_open(path);

  if(dir) {
    closedir(dir);
    return true;
  }
  return false;
}

bool directory_rename(string_t old_path, string_t new_path)
{
  return file_rename(old_path, new_path);
}

string_t directory_cwd(allocator_t* allocator)
{
  char* buffer = allocator_alloc(allocator, PATH_MAX, 0);
  if(getcwd(buffer, PATH_MAX) != NULL) {
    return string_from_buffer(buffer, strlen(buffer));
  }
  return (string_t){ .string = NULL, .length = 0 };
}

bool directory_change(string_t path)
{
  scratch_t* scratch = scratch_begin();
  allocator_t* temp = scratch_allocator(scratch);
  const char* cstr = string_to_cstr(temp, path);

  bool res = chdir(cstr) == 0;

  scratch_end(scratch);
  return res;
}
