#include <string.h>
#include "core/container/array_list.h"
#include "core/string/string_builder.h"

string_builder_t string_builder_init(allocator_t* allocator)
{
  string_builder_t sb;
  sb.allocator = allocator;
  sb.string = array_list(char, allocator);

  return sb;
}

string_builder_t string_builder_from_string(allocator_t* allocator, string_t string)
{
  string_builder_t sb = string_builder_init(allocator);
  string_builder_append(&sb, string);
  return sb;
}

string_builder_t string_builder_from_list(allocator_t* allocator, string_t* string_list)
{
  string_builder_t sb = string_builder_init(allocator);
  array_list_for_each(string_list, string_t, str)
  {
    string_builder_append(&sb, str);
  }

  return sb;
}

string_builder_t string_builder_from_buffer(allocator_t* allocator, char* buffer, size_t len)
{
  string_builder_t sb = string_builder_init(allocator);
  string_t str = string_from_buffer(buffer, len);
  string_builder_append(&sb, str);
  return sb;
}

string_t string_builder_to_string(string_builder_t* sb)
{
  return (string_t){
    .string = sb->string,
    .length = string_builder_length(*sb),
  };
}

string_t string_builder_to_string_copy(string_builder_t* sb)
{
  size_t len = string_builder_length(*sb);
  char* buffer = allocator_alloc(sb->allocator, len, alignof(char));
  memcpy(buffer, sb->string, len);
  return (string_t){
    .string = buffer,
    .length = len
  };
}

const char* string_builder_to_cstr(string_builder_t* sb)
{
  string_t str = string_builder_to_string(sb);
  return string_to_cstr(sb->allocator, str);
}

void string_builder_append(string_builder_t* sb, string_t str)
{
  size_t len = string_length(str);
  if(len == 0)
    return;

  sb->string = array_list_ensure_capacity(sb->string, len, sizeof(char));
  memcpy(sb->string + array_list_length(sb->string), str.string, len);
  array_list_header(sb->string)->length += len;
}

void string_builder_reset(string_builder_t* sb)
{
  array_list_clear(sb->string);
}

size_t string_builder_length(string_builder_t sb)
{
  return array_list_length(sb.string);
}

size_t string_builder_capacity(string_builder_t sb)
{
  return array_list_capacity(sb.string);
}
