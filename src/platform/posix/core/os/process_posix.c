#include "array_list.h"
#include "process.h"
#include "string_helper.h"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

#define FD_INVALID (-1)

static void close_pipe(int pipe[2])
{
  if(pipe[PIPE_READ] != FD_INVALID) {
    close(pipe[PIPE_READ]);
  }
  if(pipe[PIPE_WRITE] != FD_INVALID) {
    close(pipe[PIPE_WRITE]);
  }
}

string_t process_error_string(process_error_t error)
{
  switch(error) {
    case PROCESS_OK:
      return string("No error");
    case PROCESS_ERR_NOT_FOUND:
      return string("Command or executable not found");
    case PROCESS_ERR_PERMISSION:
      return string("Permission denied");
    case PROCESS_ERR_FORK_FAILED:
      return string("Failed to create child process");
    case PROCESS_ERR_EXEC_FAILED:
      return string("Failed to execute command");
    case PROCESS_ERR_PIPE_FAILED:
      return string("Failed to create pipes");
    case PROCESS_ERR_TIMEOUT:
      return string("Process timed out");
    case PROCESS_ERR_SIGNAL:
      return string("Process killed by signal");
    case PROCESS_ERR_IO:
      return string("I/O error reading/writing output");
  }
  return string("Unknown error");
}

static string_t read_pipe(int fd, i_allocator_t* allocator)
{
  char buf[4096];
  ssize_t len = read(fd, buf, sizeof(buf) - 1);
  if(len < 0) {
    return string_empty();
  }
  return string_copy_buffer(allocator, buf, len);
}

process_result_t process_run(process_command_t command, i_allocator_t* allocator)
{
  process_result_t result = { 0 };

  const char* program = string_to_cstr(allocator, command.program);
  const char** argv = string_arr_to_cstr_arr(allocator, command.argv);
  array_list_push(argv, NULL);

  int stdout_pipe[2];
  int stderr_pipe[2];

  if(pipe(stdout_pipe) == -1) {
    result.error = PROCESS_ERR_PIPE_FAILED;
    return result;
  }

  if(pipe(stderr_pipe) == -1) {
    result.error = PROCESS_ERR_PIPE_FAILED;
    close_pipe(stdout_pipe);
    return result;
  }

  pid_t pid = fork();

  if(pid < 0) {
    result.error = PROCESS_ERR_FORK_FAILED;
    close_pipe(stdout_pipe);
    close_pipe(stderr_pipe);
    return result;
  }

  if(pid == 0) {
    // Child process
    if(command.cwd != NULL) {
      chdir(command.cwd);
    }

    if(command.stdin.posix_fd != FD_INVALID) {
      dup2(command.stdin.posix_fd, STDIN_FILENO);
    }

    dup2(stdout_pipe[PIPE_WRITE], STDOUT_FILENO);
    dup2(stderr_pipe[PIPE_WRITE], STDERR_FILENO);

    // Also redirect to user FDs if provided
    if(command.stdout.posix_fd != FD_INVALID) {
      dup2(stdout_pipe[PIPE_WRITE], command.stdout.posix_fd);
    }
    if(command.stderror.posix_fd != FD_INVALID) {
      dup2(stderr_pipe[PIPE_WRITE], command.stderror.posix_fd);
    }

    close_pipe(stdout_pipe);
    close_pipe(stderr_pipe);

    execvp(program, (char* const*)argv);
    _exit(127);
  }

  // Parent process
  close(stdout_pipe[PIPE_WRITE]);
  close(stderr_pipe[PIPE_WRITE]);

  result.output.stdout = read_pipe(stdout_pipe[PIPE_READ], allocator);
  result.output.stderr = read_pipe(stderr_pipe[PIPE_READ], allocator);

  // Also write to user FDs if provided
  if(command.stdout.posix_fd != FD_INVALID) {
    fs_result_t res = file_write_fd(command.stdout, result.output.stdout.string, result.output.stdout.length);
    if(res.error != FS_OK) {
      result.error = PROCESS_ERR_IO;
    }
  }
  if(command.stderror.posix_fd != FD_INVALID) {
    fs_result_t res = file_write_fd(command.stderror, result.output.stderr.string, result.output.stderr.length);
    if(res.error != FS_OK) {
      result.error = PROCESS_ERR_IO;
    }
  }

  close(stdout_pipe[PIPE_READ]);
  close(stderr_pipe[PIPE_READ]);

  int status;
  if(waitpid(pid, &status, 0) == -1) {
    result.error = PROCESS_ERR_IO;
    return result;
  }

  if(WIFEXITED(status)) {
    result.output.exit_code = WEXITSTATUS(status);
    if(result.output.exit_code == 127) {
      result.error = PROCESS_ERR_NOT_FOUND;
    }
    else if(result.output.exit_code == 126) {
      result.error = PROCESS_ERR_PERMISSION;
    }
  }
  else if(WIFSIGNALED(status)) {
    result.output.signal = WTERMSIG(status);
    result.error = PROCESS_ERR_SIGNAL;
  }

  return result;
}
