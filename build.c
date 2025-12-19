#include "cbuild/builder.h"
#include "cbuild/cbuild.h"
#include "cbuild/target.h"

void build(builder_t* b)
{
  allocator_t* alloc = builder_allocator(b);
  target_t* core = target_static_lib(alloc, string("core"));

  target_add_source_dirs(core,
    string("src/core"),
    string("src/core/allocators"),
    string("src/core/containers"),
    string("src/core/logger"),
    string("src/core/memory"),
    string("src/core/os"),
    string("src/core/string"),
    string("src/core/platform/posix/core/memory"),
    string("src/core/platform/posix/core/os"));

  target_add_include(core, string("include"));

  builder_add_target(b, core);

  target_t* cbuild = target_static_lib(alloc, string("cbuild"));

  target_add_source_dirs(cbuild,
    string("src/cbuild"),
    string("src/cbuild/builder"),
    string("src/cbuild/builder/steps"),
    string("src/cbuild/cli"),
    string("src/cbuild/graph"));

  target_add_include(cbuild, string("include"));
  target_link_target(cbuild, core);

  builder_add_target(b, cbuild);

  target_t* exec = target_executable(alloc, string("cbuildexe"));
  target_add_source(exec, string("build.c"));
  target_link_target(exec, cbuild);
  target_add_include(exec, string("include"));

  builder_add_target(b, exec);
}
