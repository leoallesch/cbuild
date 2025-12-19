#include "builder.h"
#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/logger/logger.h"
#include "core/os/process.h"
#include "core/string/string.h"

static string_t libcbuild_path = string("build/bin/libcbuild.a");
static string_t libcore_path = string("build/bin/libcore.a");

static bool run(builder_t* b, hook_step_t* h)
{
  (void)h;
  process_command_t cmd = { 0 };
  string_t* args = array_list(string_t, b->allocator);

  // [compiler] -Iinclude [build.c] [libcbuild.a] [libcore.a] -o cbuild
  array_list_push(args, b->toolchain.c_compiler);
  array_list_push(args, string("-Iinclude"));
  array_list_push(args, b->config_file_path);
  array_list_push(args, libcbuild_path);
  array_list_push(args, libcore_path);
  array_list_push(args, string("-o"));
  array_list_push(args, string("cbuild"));

  cmd.program = b->toolchain.c_compiler;
  cmd.argv = args;
  cmd.cwd = NULL;
  cmd.stdin = PROCESS_FD_INVALID;
  cmd.stdout = PROCESS_FD_INVALID;
  cmd.stderror = PROCESS_FD_INVALID;

  log_info_tag("SELF_REBUILD", "Rebuilding cbuild...");

  process_result_t res = process_run(cmd, b->allocator);
  if(res.error != PROCESS_OK || res.output.exit_code != 0) {
    log_error_tag("SELF_REBUILD", "Failed: " STR_FMT, STR_ARG(res.output.stderr));
    return false;
  }

  return true;
}

static void on_complete(builder_t* b, hook_step_t* h)
{
  (void)h;
  process_command_t cmd = { 0 };
  string_t* args = array_list(string_t, b->allocator);

  for(int i = 0; i < b->argc; i++) {
    array_list_push(args, string(b->argv[i]));
  }

  cmd.program = string(b->argv[0]);
  cmd.argv = args;
  cmd.cwd = NULL;
  cmd.stdin = PROCESS_FD_INVALID;
  cmd.stdout = PROCESS_FD_INVALID;
  cmd.stderror = PROCESS_FD_INVALID;

  // Replace current process with new binary
  log_info_tag("SELF_REBUILD", "Restarting with new binary...");

  process_error_t err = process_exec(cmd, b->allocator);

  // Only reaches here if exec failed
  log_error_tag("SELF_REBUILD", "exec failed: " STR_FMT, STR_ARG(process_error_string(err)));
}

hook_step_t* hook_self_rebuild(builder_t* b)
{
  hook_step_t* hook = allocator_alloc(b->allocator, sizeof(hook_step_t), alignof(hook_step_t));

  hook->name = string("self-rebuild");
  hook->inputs = array_list(string_t, b->allocator);
  array_list_push(hook->inputs, b->config_file_path);
  array_list_push(hook->inputs, libcbuild_path);
  array_list_push(hook->inputs, libcore_path);
  hook->output = string("cbuild");
  hook->run = run;
  hook->on_complete = on_complete;

  return hook;
}
