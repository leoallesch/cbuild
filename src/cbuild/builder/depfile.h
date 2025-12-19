#pragma once

#include "core/alloc/allocator.h"
#include "core/string/string.h"

string_t* depfile_parse(string_t path, allocator_t* alloc);
