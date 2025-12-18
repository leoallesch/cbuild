#include "array_list.h"
#include "i_allocator.h"
#include "logger.h"
#include "string_builder.h"
#include "string_helper.h"
#include "target.h"

static string_t target_kind_to_string(target_kind_t kind)
{
  switch(kind) {
    case TARGET_EXECUTABLE:
      return string("executable");
    case TARGET_STATIC_LIB:
      return string("static lib");
    case TARGET_SHARED_LIB:
      return string("shared lib");
    case TARGET_OBJECT:
      return string("object");
    default:
      return string("unknown");
  }
}

static string_t source_lang_to_string(source_lang_t lang)
{
  switch(lang) {
    case LANG_C:
      return string("C");
    case LANG_CXX:
      return string("C++");
    case LANG_ASM:
      return string("Assembly");
    case LANG_AUTO:
      return string("Auto");
    default:
      return string("unknown");
  }
}

// Add these static helper functions near the top, after the existing to_string functions

static string_t optimize_mode_to_string(optimize_mode_t mode)
{
  switch(mode) {
    case OPT_NONE:
      return string("none (-O0)");
    case OPT_DEBUG:
      return string("debug (-Og)");
    case OPT_RELEASE:
      return string("release (-O2)");
    case OPT_FAST:
      return string("fast (-O3)");
    case OPT_SIZE:
      return string("size (-Os)");
    case OPT_SIZE_MIN:
      return string("minimal size (-Oz)");
    default:
      return string("unknown");
  }
}

static string_t include_kind_to_string(include_kind_t kind)
{
  switch(kind) {
    case INCLUDE_PATH:
      return string("-I (local)");
    case INCLUDE_SYSTEM:
      return string("-isystem (system)");
    case INCLUDE_AFTER:
      return string("-idirafter");
    case INCLUDE_FRAMEWORK:
      return string("-F (framework)");
    default:
      return string("unknown");
  }
}

static string_t link_kind_to_string(link_kind_t kind)
{
  switch(kind) {
    case LINK_TARGET:
      return string("target dependency");
    case LINK_SYSTEM_LIB:
      return string("system library (-l)");
    case LINK_STATIC_PATH:
      return string("static library path");
    case LINK_SHARED_PATH:
      return string("shared library path");
    case LINK_FRAMEWORK:
      return string("framework (-framework)");
    case LINK_OBJECT_FILE:
      return string("object file");
    default:
      return string("unknown");
  }
}

static void log_string_list(const char* tag, const char* label, string_t* list)
{
  if(array_list_length(list) == 0) {
    log_info_tag(tag, "  %s: (none)", label);
    return;
  }

  log_info_tag(tag, "  %s (%zu):", label, array_list_length(list));
  array_list_for_each(list, string_t, item)
  {
    log_info_tag(tag, "    - " STR_FMT, STR_ARG(item));
  }
}

static void log_include_dirs(const char* tag, include_dir_t* list)
{
  if(array_list_length(list) == 0) {
    log_info_tag(tag, "  Include directories: (none)");
    return;
  }

  log_info_tag(tag, "  Include directories (%zu):", array_list_length(list));
  array_list_for_each(list, include_dir_t, inc)
  {
    log_info_tag(tag, "    " STR_FMT "  [" STR_FMT "]", STR_ARG(inc.path), STR_ARG(include_kind_to_string(inc.kind)));
  }
}

static void log_sources(const char* tag, source_file_t* list)
{
  if(array_list_length(list) == 0) {
    log_info_tag(tag, "  Sources: (none)");
    return;
  }

  log_info_tag(tag, "  Sources (%zu):", array_list_length(list));
  array_list_for_each(list, source_file_t, src)
  {
    log_info_tag(tag, "    " STR_FMT "  [lang: " STR_FMT "]", STR_ARG(src.path), STR_ARG(source_lang_to_string(src.language)));

    if(src.flags && array_list_length(src.flags) > 0) {
      log_info_tag(tag, "      per-file flags (%zu):", array_list_length(src.flags));
      array_list_for_each(src.flags, string_t, flag)
      {
        log_info_tag(tag, "        - " STR_FMT, STR_ARG(flag));
      }
    }
  }
}

static void log_link_objects(const char* tag, link_object_t* list)
{
  if(array_list_length(list) == 0) {
    log_info_tag(tag, "  Link objects: (none)");
    return;
  }

  log_info_tag(tag, "  Link objects (%zu):", array_list_length(list));
  array_list_for_each(list, link_object_t, obj)
  {
    string_t kind_str = link_kind_to_string(obj.kind);

    switch(obj.kind) {
      case LINK_TARGET:
        if(obj.target) {
          log_info_tag(tag, "    -> target '" STR_FMT "'  [" STR_FMT "]", STR_ARG(obj.target->name), STR_ARG(kind_str));
        }
        else {
          log_info_tag(tag, "    -> (null target)  [" STR_FMT "]", STR_ARG(kind_str));
        }
        break;
      case LINK_SYSTEM_LIB:
      case LINK_FRAMEWORK:
        log_info_tag(tag, "    " STR_FMT "  [" STR_FMT "]", STR_ARG(obj.name), STR_ARG(kind_str));
        break;
      case LINK_STATIC_PATH:
      case LINK_SHARED_PATH:
      case LINK_OBJECT_FILE:
        log_info_tag(tag, "    " STR_FMT "  [" STR_FMT "]", STR_ARG(obj.path), STR_ARG(kind_str));
        break;
      default:
        log_info_tag(tag, "    (unknown link kind)");
    }
  }
}

void target_log_config(target_t* t)
{
  if(!t) {
    log_error("Cannot log config: target is NULL");
    return;
  }

  const char* tag = "BUILD";

  log_info_tag(tag, "Target: " STR_FMT, STR_ARG(t->name));
  log_info_tag(tag, "  Kind: " STR_FMT, STR_ARG(target_kind_to_string(t->kind)));
  log_info_tag(tag, "  Default language: " STR_FMT, STR_ARG(source_lang_to_string(t->default_lang)));
  log_info_tag(tag, "  Optimization: " STR_FMT, STR_ARG(optimize_mode_to_string(t->optimize)));

  log_sources(tag, t->sources);
  log_string_list(tag, "Source directories", t->source_dirs);
  log_include_dirs(tag, t->include_dirs);
  log_string_list(tag, "Defines (-D)", t->defines);
  log_string_list(tag, "C flags", t->c_flags);
  log_string_list(tag, "CPP flags", t->cpp_flags);

  log_link_objects(tag, t->link_objects);
  log_string_list(tag, "Link flags", t->link_flags);
  log_string_list(tag, "Library paths (-L)", t->lib_paths);

  log_info_tag(tag, "  Output directory: " STR_FMT, STR_ARG(t->output_dir));
  log_info_tag(tag, "  Output name: " STR_FMT, STR_ARG(t->output_name));

  log_info_tag(tag, "  PIE: %s", t->pie ? "yes" : "no");
  log_info_tag(tag, "  LTO: %s", t->lto ? "yes" : "no");
  log_info_tag(tag, "  Strip symbols: %s", t->strip ? "yes" : "no");
  log_info_tag(tag, "  Emit dependencies: %s", t->emit_deps ? "yes" : "no");

  log_info_tag(tag, "--------------------------------------------------");
}

target_t* target_create(i_allocator_t* alloc, string_t name, target_kind_t kind)
{
  if(!alloc) {
    return NULL;
  }

  target_t* target = allocator_alloc(alloc, sizeof(target_t), alignof(target_t));

  target->name = name;
  target->kind = kind;

  target->sources = array_list(source_file_t, alloc);
  target->source_dirs = array_list(string_t, alloc);
  target->include_dirs = array_list(include_dir_t, alloc);

  target->default_lang = LANG_AUTO;
  target->optimize = OPT_DEBUG;
  target->c_flags = array_list(string_t, alloc);
  target->cxx_flags = array_list(string_t, alloc);
  target->cpp_flags = array_list(string_t, alloc);
  target->defines = array_list(string_t, alloc);

  target->link_objects = array_list(link_object_t, alloc);
  target->link_flags = array_list(string_t, alloc);
  target->lib_paths = array_list(string_t, alloc);

  target->output_dir = string("bin");
  target->output_name = name;

  target->pie = false;
  target->lto = false;
  target->strip = false;
  target->emit_deps = true;

  target->allocator = alloc;

  return target;
}

// Convenience constructors
target_t* target_executable(i_allocator_t* alloc, string_t name)
{
  target_t* target = target_create(alloc, name, TARGET_EXECUTABLE);
  target->pie = true;
  return target;
}

target_t* target_static_lib(i_allocator_t* alloc, string_t name)
{
  target_t* target = target_create(alloc, name, TARGET_STATIC_LIB);
  return target;
}

target_t* target_shared_lib(i_allocator_t* alloc, string_t name)
{
  target_t* target = target_create(alloc, name, TARGET_SHARED_LIB);
  return target;
}

// Sources
void target_add_source(target_t* t, string_t path)
{
  source_file_t src = {
    .path = path,
    .language = LANG_AUTO,
    .flags = NULL
  };

  array_list_push(t->sources, src);
}

void target_add_source_with_flags(target_t* t, string_t path, source_lang_t lang, string_t* flags)
{
  source_file_t src = {
    .path = path,
    .language = lang,
    .flags = flags
  };

  array_list_push(t->sources, src);
}

void _target_add_sources(target_t* t, string_t* paths, size_t count)
{
  for(size_t i = 0; i < count; i++) {
    target_add_source(t, paths[i]);
  }
}

void target_add_source_dir(target_t* t, string_t path)
{
  array_list_push(t->source_dirs, path);
}

void _target_add_source_dirs(target_t* t, string_t* paths, size_t count)
{
  for(size_t i = 0; i < count; i++) {
    array_list_push(t->source_dirs, paths[i]);
  }
}

// Include paths
void target_add_include(target_t* t, string_t path)
{
  target_add_include_dir(t, path, INCLUDE_PATH);
}

void target_add_include_system(target_t* t, string_t path)
{
  target_add_include_dir(t, path, INCLUDE_SYSTEM);
}

void target_add_include_dir(target_t* t, string_t path, include_kind_t kind)
{
  include_dir_t inc = {
    .kind = kind,
    .path = path
  };

  array_list_push(t->include_dirs, inc);
}

// Defines
void target_add_define(target_t* t, string_t define)
{
  array_list_push(t->defines, define);
}

void target_add_define_value(target_t* t, string_t name, string_t value)
{
  string_builder_t sb = string_builder_from_string(t->allocator, name);
  string_builder_append(&sb, string("="));
  string_builder_append(&sb, value);
  array_list_push(t->defines, string_builder_to_string(&sb));
}

// Flags
void target_add_c_flag(target_t* t, string_t flag)
{
  array_list_push(t->c_flags, flag);
}

void target_add_cxx_flag(target_t* t, string_t flag)
{
  array_list_push(t->cxx_flags, flag);
}

void target_add_cpp_flag(target_t* t, string_t flag)
{
  array_list_push(t->cpp_flags, flag);
}

void target_add_link_flag(target_t* t, string_t flag)
{
  array_list_push(t->link_flags, flag);
}

// Linking
void target_link_target(target_t* t, target_t* dep)
{
  link_object_t obj = {
    .kind = LINK_TARGET,
    .target = dep,
  };
  array_list_push(t->link_objects, obj);
}

void target_link_system_lib(target_t* t, string_t name)
{
  link_object_t obj = {
    .kind = LINK_SYSTEM_LIB,
    .name = name
  };
  array_list_push(t->link_objects, obj);
}

void target_link_static(target_t* t, string_t path)
{
  link_object_t obj = {
    .kind = LINK_STATIC_PATH,
    .path = path
  };
  array_list_push(t->link_objects, obj);
}

void target_link_shared(target_t* t, string_t path)
{
  link_object_t obj = {
    .kind = LINK_SHARED_PATH,
    .path = path
  };
  array_list_push(t->link_objects, obj);
}

void target_add_lib_path(target_t* t, string_t path)
{
  array_list_push(t->lib_paths, path);
}

// Configuration
void target_set_optimize(target_t* t, optimize_mode_t mode)
{
  t->optimize = mode;
}

void target_set_output_dir(target_t* t, string_t dir)
{
  t->output_dir = dir;
}

void target_set_output_name(target_t* t, string_t name)
{
  t->output_name = name;
}
