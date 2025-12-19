#pragma once

// ============================================================================
// CBuild String Type
// ============================================================================
//
// Non-owning string view type. Stores pointer + length, does not own memory.
// Use string() macro to create from string literals.
//

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "core/alloc/allocator.h"

// ============================================================================
// String Type
// ============================================================================

typedef struct string_t {
  const char* string;
  size_t length;
} string_t;

// ============================================================================
// String Creation
// ============================================================================

// Create string from literal: string("hello")
#define string(str)                        \
  (string_t)                               \
  {                                        \
    .string = (str), .length = strlen(str) \
  }

// Create empty string
#define string_empty() \
  (string_t){ .string = "", .length = 0 }

// Create string from buffer with known length
static inline string_t string_from(const char* data, size_t length)
{
  return (string_t){ .string = data, .length = length };
}

// Create string from null-terminated C string (calculates length)
static inline string_t string_from_cstr(const char* cstr)
{
  return (string_t){ .string = cstr, .length = cstr ? strlen(cstr) : 0 };
}

// ============================================================================
// Printf Support
// ============================================================================

// Usage: printf(STR_FMT "\n", STR_ARG(my_string));
#define STR_FMT "%.*s"
#define STR_ARG(str) (int)(str).length, (str).string

// ============================================================================
// Basic Operations
// ============================================================================

static inline size_t string_length(string_t str)
{
  return str.length;
}

static inline bool string_is_empty(string_t str)
{
  return str.length == 0;
}

static inline char string_at(string_t str, size_t index)
{
  return index < str.length ? str.string[index] : '\0';
}

static inline char string_first(string_t str)
{
  return str.length > 0 ? str.string[0] : '\0';
}

static inline char string_last(string_t str)
{
  return str.length > 0 ? str.string[str.length - 1] : '\0';
}

// ============================================================================
// Comparison
// ============================================================================

bool string_equals(string_t a, string_t b);
int string_compare(string_t a, string_t b);
bool string_starts_with(string_t str, string_t prefix);
bool string_ends_with(string_t str, string_t suffix);
bool string_contains(string_t str, string_t needle);

// ============================================================================
// Slicing (zero-copy operations)
// ============================================================================

string_t string_substr(string_t str, size_t start, size_t len);
string_t string_slice(string_t str, size_t start, size_t end);
string_t string_remove_prefix(string_t str, size_t n);
string_t string_remove_suffix(string_t str, size_t n);

// ============================================================================
// Searching
// ============================================================================

size_t string_find(string_t str, string_t needle);
int string_index_of(string_t str, char c);
int string_last_index_of(string_t str, char c);

// ============================================================================
// Splitting
// ============================================================================

string_t string_chop_left(string_t str, char delim);
string_t string_chop_right(string_t str, char delim);

// ============================================================================
// Allocating Operations (require allocator)
// ============================================================================

string_t string_copy(allocator_t* alloc, string_t str);
string_t string_concat(allocator_t* alloc, string_t a, string_t b);
const char* string_to_cstr(allocator_t* alloc, string_t str);
string_t* string_split(allocator_t* alloc, string_t str, char delim, bool keep_delim);

// ============================================================================
// Internal String Functions (not in public API)
// ============================================================================

// Copy buffer to new allocation (internal use)
string_t string_copy_buffer(allocator_t* allocator, const char* buffer, size_t length);

// Convert array_list of string_t to NULL-terminated array of C strings
const char** string_arr_to_cstr_arr(allocator_t* allocator, string_t* str_list);

// Chop on string delimiter
string_t string_chop_string(string_t str, string_t delim);

// ============================================================================
// Compat: Old function names (macros for code using old names)
// ============================================================================

#define string_from_buffer(buf, len) string_from((buf), (len))
#define string_isempty(str) string_is_empty(str)
#define string_front(str) string_first(str)
#define string_back(str) string_last(str)
#define string_indexof(str, c) string_index_of((str), (c))
#define string_lindexof(str, c) string_last_index_of((str), (c))
#define string_chop_delim_left(str, c) string_chop_left((str), (c))
#define string_chop_delim_right(str, c) string_chop_right((str), (c))
