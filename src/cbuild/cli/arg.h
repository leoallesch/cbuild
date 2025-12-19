#pragma once

#include <stdbool.h>

typedef enum {
  CMD_BUILD,
  CMD_CLEAN,
  CMD_REBUILD,
  CMD_INIT,
  CMD_HELP,
  CMD_UNKNOWN,
} cli_command_t;

typedef struct {
  cli_command_t command;
  bool verbose;
  const char* build_file;  // custom build file path (default: "build.c")
} cli_args_t;

cli_args_t cli_parse_args(int argc, char** argv);
void cli_print_usage(const char* program);
