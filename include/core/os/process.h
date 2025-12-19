#pragma once

#include <stdint.h>

#include "core/alloc/allocator.h"
#include "core/os/file.h"
#include "core/string/string.h"

// Use this to indicate "no redirection" (let process_run create pipes)
#define PROCESS_FD_INVALID ((fd_t){ .posix_fd = -1 })

typedef struct
{
  string_t program;
  string_t* argv;
  const char* cwd;

  fd_t stdin;
  fd_t stdout;
  fd_t stderror;
} process_command_t;

#define CMD_ARGV(prog, ...) \
  (char*[])                 \
  {                         \
    prog, __VA_ARGS__, NULL \
  }

typedef enum {
  PROCESS_OK = 0,
  PROCESS_ERR_NOT_FOUND, // Command/executable not found
  PROCESS_ERR_PERMISSION, // Permission denied
  PROCESS_ERR_FORK_FAILED, // Failed to create child process
  PROCESS_ERR_EXEC_FAILED, // Failed to execute command
  PROCESS_ERR_PIPE_FAILED, // Failed to create pipes
  PROCESS_ERR_TIMEOUT, // Process timed out
  PROCESS_ERR_SIGNAL, // Process killed by signal
  PROCESS_ERR_IO, // I/O error reading output
} process_error_t;

typedef struct
{
  int exit_code;
  int signal;

  string_t stdout;
  string_t stderr;
} process_output_t;

typedef struct
{
  process_error_t error;
  process_output_t output;
} process_result_t;

string_t process_error_string(process_error_t error);
process_result_t process_run(process_command_t command, allocator_t* allocator);

// Replace current process with command (does not return on success)
process_error_t process_exec(process_command_t command, allocator_t* allocator);
