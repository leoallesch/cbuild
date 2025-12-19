#pragma once

#include "core/alloc/allocator.h"
#include "core/string/string.h"

typedef struct
{
  allocator_t* allocator;
  char* string;
} string_builder_t;

string_builder_t string_builder_init(allocator_t* allocator);
string_builder_t string_builder_from_string(allocator_t* allocator, string_t string);
string_builder_t string_builder_from_list(allocator_t* allocator, string_t* string_list);
string_builder_t string_builder_from_buffer(allocator_t* allocator, char* buffer, size_t len);

string_t string_builder_to_string(string_builder_t* sb);
string_t string_builder_to_string_copy(string_builder_t* sb);
const char* string_builder_to_cstr(string_builder_t* sb);

void string_builder_append(string_builder_t* sb, string_t str);

void string_builder_reset(string_builder_t* sb);
size_t string_builder_length(string_builder_t sb);
size_t string_builder_capacity(string_builder_t sb);
