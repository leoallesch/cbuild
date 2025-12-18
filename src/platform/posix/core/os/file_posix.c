#include "file.h"
#include "i_allocator.h"
#include "memory_context.h"
#include "path.h"
#include "string_helper.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int file_mode_to_posix(file_mode_t mode)
{
  switch(mode) {
    case FILE_MODE_R:
      return O_RDONLY;
    case FILE_MODE_W:
      return O_WRONLY | O_CREAT | O_TRUNC;
    case FILE_MODE_A:
      return O_WRONLY | O_CREAT | O_APPEND;
    case FILE_MODE_RP:
      return O_RDWR;
    case FILE_MODE_WP:
      return O_RDWR | O_CREAT | O_TRUNC;
    case FILE_MODE_AP:
      return O_RDWR | O_CREAT | O_APPEND;
  }
  return O_RDONLY;
}

static size_t file_size(fd_t fd)
{
  off_t size = lseek(fd.posix_fd, 0, SEEK_END);
  lseek(fd.posix_fd, 0, SEEK_SET);
  if(size < 0) {
    size = 0;
  }
  return size;
}

file_type_t file_type_from_mode(__mode_t mode)
{
  switch(mode & S_IFMT) {
    case S_IFBLK:
      return FILE_TYPE_BLOCK_DEVICE;
    case S_IFCHR:
      return FILE_TYPE_CHAR_DEVICE;
    case S_IFDIR:
      return FILE_TYPE_DIRECTORY;
    case S_IFIFO:
      return FILE_TYPE_FIFO;
    case S_IFLNK:
      return FILE_TYPE_SYMLINK;
    case S_IFREG:
      return FILE_TYPE_FILE;
    case S_IFSOCK:
      return FILE_TYPE_SOCKET;
    default:
      return FILE_TYPE_UNKNOWN;
  }
}

fd_t file_open(string_t path, file_mode_t mode)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp = scratch_allocator(scratch);
  string_t normalized = path_normalize(path, temp);
  const char* cstr = string_to_cstr(temp, normalized);

  int flags = file_mode_to_posix(mode);
  fd_t fd;
  if(flags & O_CREAT) {
    fd.posix_fd = open(cstr, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  }
  else {
    fd.posix_fd = open(cstr, flags);
  }

  scratch_end(scratch);
  return fd;
}

fs_error_t file_close(fd_t fd)
{
  fs_error_t err = FS_OK;
  int res = close(fd.posix_fd);
  if(res) {
    err = FS_ERR_IO;
  }
  return err;
}

fs_result_t file_read_fd(fd_t fd, i_allocator_t* allocator)
{
  fs_result_t res = { .error = FS_OK };

  size_t size = file_size(fd);
  char* buffer = allocator_alloc(allocator, size, alignof(char));
  ssize_t br = read(fd.posix_fd, buffer, size);
  if(br < 0) {
    res.error = FS_ERR_IO;
    return res;
  }

  res.string = string_from_buffer(buffer, br);
  return res;
}

fs_result_t file_write_fd(fd_t fd, const void* data, size_t size)
{
  fs_result_t res = { .error = FS_OK };

  ssize_t written = write(fd.posix_fd, data, size);
  if(written < 0) {
    res.error = FS_ERR_IO;
    return res;
  }

  res.size = (size_t)written;
  return res;
}

fs_result_t file_read(string_t path, i_allocator_t* allocator)
{
  fs_result_t res = { .error = FS_OK };
  fd_t fd = file_open(path, FILE_MODE_R);
  if(fd.posix_fd < 0) {
    res.error = FS_ERR_NOT_FOUND;
    return res;
  }
  res = file_read_fd(fd, allocator);
  file_close(fd);
  return res;
}

fs_result_t file_write(string_t path, const void* data, size_t size)
{
  fs_result_t res = { .error = FS_OK, .size = 0 };
  fd_t fd = file_open(path, FILE_MODE_W);
  if(fd.posix_fd < 0) {
    res.error = FS_ERR_IO;
    return res;
  }
  res = file_write_fd(fd, data, size);
  file_close(fd);
  return res;
}

fs_result_t file_write_atomic(string_t path, const void* data, size_t size)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  string_t tmp_path = string_concat(temp_alloc, path, string(".tmp"));
  fs_result_t res = file_write(tmp_path, data, size);
  if(res.error == FS_OK) {
    if(!file_rename(tmp_path, path)) {
      res.error = FS_ERR_IO;
    }
  }

  scratch_end(scratch);

  return res;
}

fs_result_t file_append(string_t path, const void* data, size_t size)
{
  fs_result_t res = { .error = FS_OK, .size = 0 };
  fd_t fd = file_open(path, FILE_MODE_A);
  if(fd.posix_fd < 0) {
    res.error = FS_ERR_IO;
    return res;
  }
  res = file_write_fd(fd, data, size);
  file_close(fd);
  return res;
}

bool file_delete(string_t path)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp = scratch_allocator(scratch);
  const char* cstr = string_to_cstr(temp, path);

  bool res = remove(cstr) == 0;
  scratch_end(scratch);

  return res;
}

bool file_exists(string_t path)
{
  fd_t fd = file_open(path, FILE_MODE_R);

  if(fd.posix_fd >= 0) {
    file_close(fd);
    return true;
  }
  return false;
}

file_stat_t file_stat(string_t path)
{
  file_stat_t fstat_result = { 0 };

  fd_t fd = file_open(path, FILE_MODE_R);
  if(fd.posix_fd < 0) {
    return fstat_result;
  }

  struct stat fs;
  int err = fstat(fd.posix_fd, &fs);

  if(err) {
    file_close(fd);
    return fstat_result;
  }

  fstat_result.size = fs.st_size;
  fstat_result.ctime = fs.st_ctime;
  fstat_result.atime = fs.st_atime;
  fstat_result.mtime = fs.st_mtime;
  fstat_result.type = file_type_from_mode(fs.st_mode);
  fstat_result.path = path;
  fstat_result.name = path_basename(path);

  file_close(fd);

  return fstat_result;
}

bool file_rename(string_t old_path, string_t new_path)
{
  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp = scratch_allocator(scratch);
  const char* old_cstr = string_to_cstr(temp, old_path);
  const char* new_cstr = string_to_cstr(temp, new_path);

  int res = rename(old_cstr, new_cstr) == 0;

  scratch_end(scratch);

  return res;
}

bool file_copy(string_t src_path, string_t dest_path)
{
  fd_t src_fd = file_open(src_path, FILE_MODE_R);
  if(src_fd.posix_fd < 0) {
    return false;
  }

  fd_t dest_fd = file_open(dest_path, FILE_MODE_W);
  if(dest_fd.posix_fd < 0) {
    file_close(src_fd);
    return false;
  }

  char buffer[4096];
  ssize_t bytes;
  while((bytes = read(src_fd.posix_fd, buffer, sizeof(buffer))) > 0) {
    write(dest_fd.posix_fd, buffer, bytes);
  }

  file_close(src_fd);
  file_close(dest_fd);

  return true;
}

bool file_is_newer(string_t path_a, string_t path_b)
{
  file_stat_t stat_a = file_stat(path_a);
  file_stat_t stat_b = file_stat(path_b);
  return stat_a.mtime > stat_b.mtime;
}
