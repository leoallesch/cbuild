#include "target.h"

export_target(lib_cbuild)
{
  target_t* target = target_static_lib(allocator, string("lib_cbuild"));

  return target;
}
