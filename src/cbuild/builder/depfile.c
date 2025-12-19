#include "depfile.h"

#include "core/alloc/allocator.h"
#include "core/container/array_list.h"
#include "core/os/file.h"

string_t* depfile_parse(string_t path, allocator_t* alloc)
{
  string_t* deps = array_list(string_t, alloc);

  fs_result_t res = file_read(path, alloc);
  if(res.error != FS_OK) {
    return deps;
  }

  string_t content = res.string;
  content = string_chop_delim_right(content, ':');
  string_t* tokens = string_split(alloc, content, ' ', false);

  array_list_for_each(tokens, string_t, token)
  {
    if(string_equals(token, path) ||
      string_isempty(token) ||
      string_equals(token, string("\\\n")) ||
      string_equals(token, string("\\"))) {
      continue;
    }

    array_list_push(deps, token);
  }

  return deps;
}
