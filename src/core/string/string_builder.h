#pragma once

#include "array_list.h"
#include "i_allocator.h"
#include "string_helper.h"

typedef struct
{
  i_allocator_t* allocator;
  char* string;
} string_builder_t;

string_builder_t string_builder_init(i_allocator_t* allocator);
string_builder_t string_builder_from_string(i_allocator_t* allocator, string_t string);
string_builder_t string_builder_from_list(i_allocator_t* allocator, string_t* string_list);
string_builder_t string_builder_from_buffer(i_allocator_t* allocator, char* buffer, size_t len);

string_t string_builder_to_string(string_builder_t* sb);
string_t string_builder_to_string_copy(string_builder_t* sb);
const char* string_builder_to_cstr(string_builder_t* sb);

void string_builder_append(string_builder_t* sb, string_t str);

static inline void string_builder_reset(string_builder_t* sb)
{
  array_list_clear(sb->string);
}

static inline size_t string_builder_length(string_builder_t sb)
{
  return array_list_length(sb.string);
}

static inline size_t string_builder_capacity(string_builder_t sb)
{
  return array_list_capacity(sb.string);
}
