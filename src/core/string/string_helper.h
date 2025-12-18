#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "i_allocator.h"

typedef struct
{
  const char* string;
  size_t length;
} string_t;

#define string(str)                      \
  (string_t)                             \
  {                                      \
    .string = str, .length = strlen(str) \
  }

#define string_empty() string("")

#define STR_FMT "%.*s"
#define STR_ARG(str) (int)(str).length, (str).string

string_t string_from_buffer(const char* buffer, size_t length);
string_t string_copy_buffer(i_allocator_t* allocator, const char* buffer, size_t length);
string_t string_copy(i_allocator_t* allocator, string_t str);
const char* string_to_cstr(i_allocator_t* allocator, string_t str);
const char** string_arr_to_cstr_arr(i_allocator_t* allocator, string_t *str_list);
string_t string_concat(i_allocator_t* allocator, string_t a, string_t b);
string_t string_substr(string_t str, size_t start, size_t len);
string_t string_slice(string_t str, size_t start, size_t end);
bool string_equals(string_t a, string_t b);
int string_compare(string_t a, string_t b);
string_t* string_split(i_allocator_t* allocator, string_t str, char delim, bool keep_delim);
string_t string_remove_prefix(string_t str, size_t n);
string_t string_remove_suffix(string_t str, size_t n);
bool string_starts_with(string_t str, string_t target);
bool string_ends_with(string_t str, string_t target);
bool string_contains(string_t str, string_t target);
size_t string_find(string_t str, string_t target);
int string_indexof(string_t str, char c);
int string_lindexof(string_t str, char c);
string_t string_chop_delim_left(string_t str, char delim);
string_t string_chop_delim_right(string_t str, char delim);
string_t string_chop_string(string_t str, string_t tool);

static inline char string_at(string_t str, size_t index)
{
  return index <= str.length ? str.string[index] : '\0';
}

static inline char string_front(string_t str)
{
  return str.length > 0 ? str.string[0] : '\0';
}

static inline char string_back(string_t str)
{
  return str.length > 0 ? str.string[str.length - 1] : '\0';
}

static inline size_t string_length(string_t str)
{
  return str.length;
}

static inline bool string_isempty(string_t str)
{
  return str.length == 0;
}
