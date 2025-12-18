#include "array_list.h"
#include "i_allocator.h"
#include "string_helper.h"

#include <string.h>

string_t string_from_buffer(const char* buffer, size_t len)
{
  return (string_t){
    .string = buffer,
    .length = len
  };
}

string_t string_copy_buffer(i_allocator_t* allocator, const char* buffer, size_t len)
{
  char* b = allocator_alloc(allocator, len, alignof(char));
  memcpy(b, buffer, len);
  return (string_t){
    .string = b,
    .length = len
  };
}

string_t string_copy(i_allocator_t* allocator, string_t str)
{
  char* b = allocator_alloc(allocator, str.length, 0);
  memcpy(b, str.string, str.length);
  return (string_t){
    .string = b,
    .length = str.length
  };
}

const char* string_to_cstr(i_allocator_t* allocator, string_t str)
{
  char* b = allocator_alloc(allocator, str.length + 1, 0);
  memcpy(b, str.string, str.length);
  b[str.length] = '\0';
  return b;
}

const char** string_arr_to_cstr_arr(i_allocator_t* allocator, string_t* str_list)
{
  const char** res = array_list(const char*, allocator);
  array_list_for_each(str_list, string_t, str)
  {
    array_list_push(res, string_to_cstr(allocator, str));
  }

  return res;
}

string_t string_concat(i_allocator_t* allocator, string_t a, string_t b)
{
  char* buffer = allocator_alloc(allocator, a.length + b.length, 0);
  memcpy(buffer, a.string, a.length);
  memcpy(buffer + a.length, b.string, b.length);
  return (string_t){
    .string = buffer,
    .length = a.length + b.length
  };
}

string_t string_substr(string_t str, size_t start, size_t len)
{
  if(start + len > string_length(str)) {
    len = string_length(str) - start;
  }
  return (string_t){
    .string = str.string + start,
    .length = len
  };
}

string_t string_slice(string_t str, size_t start, size_t end)
{
  return string_substr(str, start, end - start);
}

bool string_equals(string_t a, string_t b)
{
  if(a.length != b.length) {
    return false;
  }
  return memcmp(a.string, b.string, a.length) == 0;
}

int string_compare(string_t a, string_t b)
{
  size_t min_len = (a.length < b.length) ? a.length : b.length;

  for(size_t i = 0; i < min_len; i++) {
    unsigned char ac = (unsigned char)a.string[i];
    unsigned char bc = (unsigned char)b.string[i];

    if(ac != bc)
      return (ac < bc) ? -1 : 1;
  }

  if(a.length == b.length)
    return 0;

  return (a.length < b.length) ? -1 : 1;
}

string_t* string_split(i_allocator_t* allocator, string_t str, char delim, bool keep_delim)
{
  string_t* segments = array_list(string_t, allocator);

  size_t start = 0;
  size_t stop = 0;

  while(stop <= str.length) {
    if(stop == str.length || str.string[stop] == delim) {
      string_t segment = string_slice(str, start, stop);
      array_list_push(segments, segment);
      if(keep_delim) {
        segment = string_slice(str, stop, stop + 1);
        array_list_push(segments, segment);
      }
      start = stop + 1;
    }
    stop++;
  }

  return segments;
}

string_t string_remove_prefix(string_t str, size_t n)
{
  if(n > str.length) {
    n = str.length;
  }

  return (string_t){
    .string = str.string + n,
    .length = str.length - n
  };
}

string_t string_remove_suffix(string_t str, size_t n)
{
  if(n > str.length) {
    n = str.length;
  }
  return (string_t){
    .string = str.string,
    .length = str.length - n
  };
}

bool string_contains(string_t str, string_t target)
{
  size_t str_len = string_length(str);
  size_t target_len = string_length(target);

  bool contains = false;

  for(size_t i = 0; i < str_len; i++) {
    if(str.string[i] == target.string[0]) {
      contains = true;
      for(size_t j = 1; j < target_len; j++) {
        if(str.string[i + j] != target.string[j]) {
          contains = false;
        }
      }
      if(contains) {
        break;
      }
    }
  }
  return contains;
}

bool string_starts_with(string_t str, string_t target);
bool string_ends_with(string_t str, string_t target)
{
  size_t str_len = string_length(str);
  size_t target_len = string_length(target);
  if(str_len < target_len) {
    return false;
  }

  string_t suffix = string_substr(str, str_len - target_len, target_len);

  return string_equals(suffix, target);
}

int string_indexof(string_t str, char c)
{
  int index = -1;

  for(size_t i = 0; i < string_length(str); i++) {
    if(string_at(str, i) == c) {
      index = i;
      break;
    }
  }

  return index;
}

int string_lindexof(string_t str, char c)
{
  int index = -1;

  for(int i = string_length(str); i >= 0; i--) {
    if(string_at(str, i) == c) {
      index = i;
      break;
    }
  }

  return index;
}

string_t string_chop_delim_left(string_t str, char delim)
{
  int index = string_indexof(str, delim);
  if(index >= 0) {
    return string_slice(str, 0, index);
  }
  return string_empty();
}

string_t string_chop_delim_right(string_t str, char delim)
{
  int index = string_lindexof(str, delim);
  if(index >= 0) {
    return string_slice(str, index, string_length(str));
  }
  return string_empty();
}

string_t string_chop_string(string_t str, string_t tool)
{
  string_t window = string_from_buffer(str.string, string_length(tool));
  size_t i = 0;
  bool found = false;
  while(i + string_length(tool) < string_length(str)) {
    if(string_equals(tool, window)) {
      found = true;
      break;
    }
    i++;
    window.string++;
  }

  if(found) {
    return string_slice(str, string_length(tool), string_length(str));
  }

  return str;
}
