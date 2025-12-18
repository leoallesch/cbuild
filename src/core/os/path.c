#include "array_list.h"
#include "directory.h"
#include "filesystem.h"
#include "i_allocator.h"
#include "logger.h"
#include "path.h"
#include "string_builder.h"
#include "string_helper.h"

#include <stdio.h>

#ifdef _WIN32
#define PATH_SEP '\\'
#define PATH_SEP_STR "\\"
#else
#define PATH_SEP '/'
#define PATH_SEP_STR "/"
#endif

typedef enum {
  PATH_TOK_INVALID,
  PATH_TOK_EMPTY,
  PATH_TOK_SEP,
  PATH_TOK_DOT,
  PATH_TOK_DDOT,
  PATH_TOK_NAME,
  PATH_TOK_DSTAR
} path_token_t;

static path_token_t classify_segment(string_t seg)
{
  path_token_t tok = PATH_TOK_INVALID;

  if(string_isempty(seg)) {
    tok = PATH_TOK_EMPTY;
  }
  else if(string_equals(seg, string("."))) {
    tok = PATH_TOK_DOT;
  }
  else if(string_equals(seg, string(".."))) {
    tok = PATH_TOK_DDOT;
  }
  else if(string_equals(seg, string(PATH_SEP_STR))) {
    tok = PATH_TOK_SEP;
  }
  else if(string_equals(seg, string("**"))) {
    tok = PATH_TOK_DSTAR;
  }
  else {
    tok = PATH_TOK_NAME;
  }

  return tok;
}

static string_t build_path_from_segments(string_t* str_list, bool absolute, i_allocator_t* alloc)
{
  string_builder_t final_sb = string_builder_init(alloc);

  if(absolute) {
    string_builder_append(&final_sb, string(PATH_SEP_STR));
  }

  array_list_for_each(str_list, string_t, seg)
  {
    string_builder_append(&final_sb, seg);
    if(_i < array_list_length(str_list) - 1) {
      string_builder_append(&final_sb, string(PATH_SEP_STR));
    }
  }

  return string_builder_to_string(&final_sb);
}

bool path_is_absolute(string_t path)
{
  if(string_isempty(path)) {
    return false;
  }
  return (string_front(path) == PATH_SEP);
}

string_t path_normalize(string_t path, i_allocator_t* allocator)
{
  if(string_isempty(path) || !allocator) {
    return string(".");
  }

  bool is_absolute = path_is_absolute(path);
  string_t* path_segments = string_split(allocator, path, PATH_SEP, false);
  string_t* temp_list = array_list(string_t, allocator);

  size_t seg_count = array_list_length(path_segments);
  for(size_t i = 0; i < seg_count; i++) {
    string_t seg = path_segments[i];
    path_token_t tok = classify_segment(seg);
    size_t len = array_list_length(temp_list);

    switch(tok) {
      case PATH_TOK_EMPTY:
      case PATH_TOK_DOT:
        break;

      case PATH_TOK_DDOT:
        if(len > 0 && classify_segment(temp_list[len - 1]) != PATH_TOK_DDOT) {
          array_list_pop(temp_list);
        }
        else if(!is_absolute) {
          array_list_push(temp_list, seg);
        }
        break;

      case PATH_TOK_SEP:
      case PATH_TOK_NAME:
        array_list_push(temp_list, seg);
        break;

      default:
        break;
    }
  }

  return build_path_from_segments(temp_list, is_absolute, allocator);
}

string_t path_join(string_t base, string_t relative, i_allocator_t* alloc)
{
  if(!alloc) {
    return string(".");
  }

  string_builder_t sb = string_builder_init(alloc);

  string_builder_append(&sb, base);
  if(!string_isempty(base) && !string_isempty(relative)) {
    string_builder_append(&sb, string(PATH_SEP_STR));
  }
  string_builder_append(&sb, relative);

  return path_normalize(string_builder_to_string(&sb), alloc);
}

string_t path_dirname(string_t path)
{
  size_t len = string_length(path);

  if(len == 0) {
    return string(".");
  }

  for(int i = len - 1; i >= 0; i--) {
    if(string_at(path, i) == PATH_SEP) {
      if(i == 0) {
        return string(PATH_SEP_STR);
      }
      return string_substr(path, 0, i);
    }
  }

  return string(".");
}

string_t path_basename(string_t path)
{
  size_t base_start = 0;
  size_t len = string_length(path);

  if(len == 0) {
    return string(".");
  }

  for(int i = len; i >= 0; i--) {
    if(string_at(path, i) == PATH_SEP) {
      base_start = i + 1;
      break;
    }
  }

  if(base_start > 0) {
    return string_substr(path, base_start, len - base_start);
  }

  return path;
}

string_t path_extension(string_t path)
{
  size_t base_start = 0;
  size_t len = string_length(path);

  for(int i = len; i >= 0; i--) {
    if(string_at(path, i) == '.') {
      base_start = i;
      break;
    }

    if(string_at(path, i) == PATH_SEP) {
      base_start = 0;
      break;
    }
  }

  if(base_start > 0) {
    return string_substr(path, base_start, len - base_start);
  }

  return string_empty();
}

string_t path_stem(string_t path)
{
  string_t base = path_basename(path);
  return string_chop_delim_left(base, '.');
}

string_t path_absolute(string_t path, i_allocator_t* alloc)
{
  if(!alloc) {
    return string(".");
  }

  if(path_is_absolute(path)) {
    return path_normalize(path, alloc);
  }

  return path_join(directory_cwd(alloc), path, alloc);
}

string_t path_relative(string_t path, i_allocator_t* alloc)
{
  if(!alloc) {
    return string(".");
  }

  if(!path_is_absolute(path)) {
    return path;
  }

  string_t rel = string_chop_string(path, directory_cwd(alloc));
  if(string_front(rel) == PATH_SEP) {
    rel = string_remove_prefix(rel, 1);
  }
  return rel;
}

string_t* path_glob(string_t path, i_allocator_t* alloc)
{
  string_t dir = path_dirname(path);
  string_t base = path_basename(path);
  string_t pattern = string_chop_string(base, string("**"));

  string_t* paths = array_list(string_t, alloc);

  directory_walk(dir, entry, alloc, {
    if(string_ends_with(entry.path, pattern)) {
      array_list_push(paths, string_copy(alloc, entry.path));
    }
  });

  return paths;
}
