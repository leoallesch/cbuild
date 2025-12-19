
#include "arg.h"
#include "builder.h"
#include "core/alloc/arena_allocator.h"
#include "core/logger/logger.h"
#include "core/mem/mem.h"
#include "core/mem/mem_utils.h"
#include "core/mem/memory_context.h"
#include "core/os/directory.h"

#include "core/mem/virtual_mem.h"

extern void build(builder_t* builder);

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
  mem_t* vmem = virtual_mem_init();
  arena_t arena = arena_init(vmem, MB(10));
  memory_context_t* ctx = memory_context_create(vmem, &arena.interface, KB(10));

  logger_init();
  logger_set_level(LOG_DEBUG);

  cli_args_t args = cli_parse_args(argc, argv);

  if(args.verbose) {
    logger_set_level(LOG_DEBUG);
  }

  int result = 0;

  builder_t* builder = builder_create(ctx->allocator);
  builder->argc = argc;
  builder->argv = argv;

  switch(args.command) {
    case CMD_HELP:
      cli_print_usage("./cbuild");
      goto cleanup;
    case CMD_CLEAN:
      // Add clean hook and run it (don't call build())
      builder_add_pre_hook_internal(builder, hook_clean(builder));
      break;

    case CMD_REBUILD:
      // Add clean hook, then call build() and run
      builder_add_pre_hook_internal(builder, hook_clean(builder));
      build(builder);
      break;

    case CMD_BUILD:
    default:
      // Normal build
      build(builder);
      break;
  }

  builder_run(builder);

cleanup:
  logger_shutdown();
  memory_context_destroy(ctx);

  return result;
}
