#include "arg.h"

#include <stdio.h>
#include <string.h>

void cli_print_usage(const char* program)
{
  printf("cbuild - C build system\n\n");
  printf("Usage: %s [command] [options]\n\n", program);
  printf("Commands:\n");
  printf("  build       Build the project (default)\n");
  printf("  clean       Remove build artifacts\n");
  printf("  rebuild     Clean and build\n");
  printf("  init        Create a template build.c\n");
  printf("  help        Show this message\n");
  printf("\nOptions:\n");
  printf("  -v, --verbose    Verbose output\n");
  printf("  -f, --file FILE  Use FILE instead of build.c\n");
  printf("  -h, --help       Show this message\n");
}

static cli_command_t parse_command(const char* arg)
{
  if(strcmp(arg, "build") == 0) return CMD_BUILD;
  if(strcmp(arg, "clean") == 0) return CMD_CLEAN;
  if(strcmp(arg, "rebuild") == 0) return CMD_REBUILD;
  if(strcmp(arg, "init") == 0) return CMD_INIT;
  if(strcmp(arg, "help") == 0) return CMD_HELP;
  return CMD_UNKNOWN;
}

cli_args_t cli_parse_args(int argc, char** argv)
{
  cli_args_t args = {
    .command = CMD_BUILD,
    .verbose = false,
    .build_file = "build.c",
  };

  for(int i = 1; i < argc; i++) {
    const char* arg = argv[i];

    // Flags
    if(strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
      args.verbose = true;
    }
    else if(strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      args.command = CMD_HELP;
    }
    else if(strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0) {
      if(i + 1 < argc) {
        args.build_file = argv[++i];
      }
    }
    // Commands (non-flag arguments)
    else if(arg[0] != '-') {
      cli_command_t cmd = parse_command(arg);
      if(cmd != CMD_UNKNOWN) {
        args.command = cmd;
      }
      else {
        // Unknown command - set to unknown so main can handle error
        args.command = CMD_UNKNOWN;
      }
    }
  }

  return args;
}
