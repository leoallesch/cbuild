#pragma once

#include "i_allocator.h"
#include "string_helper.h"

string_t path_join(string_t base, string_t relative, i_allocator_t* alloc);
string_t path_dirname(string_t path); // /foo/bar.c -> /foo
string_t path_basename(string_t path); // /foo/bar.c -> bar.c
string_t path_extension(string_t path); // /foo/bar.c -> .c
string_t path_stem(string_t path); // /foo/bar.c -> bar
string_t path_absolute(string_t path, i_allocator_t* alloc); // resolve to absolute
string_t path_relative(string_t path, i_allocator_t* alloc);
string_t path_normalize(string_t path, i_allocator_t* alloc); // remove /./ and /../

string_t *path_glob(string_t path, i_allocator_t* alloc);

bool path_is_absolute(string_t path);
