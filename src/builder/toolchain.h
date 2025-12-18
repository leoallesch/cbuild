#pragma once

#include "string_helper.h"

typedef struct
{
  string_t compiler_c;
  string_t compiler_cxx;
  string_t assembler;
  string_t archiver;
  string_t linker;
  string_t objcopy;
  string_t size;
} toolchain_t;
