#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/alloc/allocator.h"
#include "core/string/string.h"

typedef enum {
  FS_OK = 0,
  FS_ERR_NOT_FOUND,
  FS_ERR_PERMISSION,
  FS_ERR_EXISTS,
  FS_ERR_NOT_EMPTY,
  FS_ERR_IO,
} fs_error_t;

typedef enum {
  FILE_MODE_R, // "r"  - read only, must exist
  FILE_MODE_W, // "w"  - write only, create/truncate
  FILE_MODE_A, // "a"  - append only, create if missing
  FILE_MODE_RP, // "r+" - read/write, must exist
  FILE_MODE_WP, // "w+" - read/write, create/truncate
  FILE_MODE_AP, // "a+" - read/append, create if missing
} file_mode_t;

typedef enum {
  FILE_TYPE_UNKNOWN,
  FILE_TYPE_FILE,
  FILE_TYPE_DIRECTORY,
  FILE_TYPE_SYMLINK,
  FILE_TYPE_BLOCK_DEVICE,
  FILE_TYPE_CHAR_DEVICE,
  FILE_TYPE_FIFO,
  FILE_TYPE_SOCKET
} file_type_t;

typedef union {
  int posix_fd;
  void* win_fd;
} fd_t;

typedef struct {
  uint64_t size;
  int64_t ctime;
  int64_t atime;
  int64_t mtime;
  file_type_t type;
  string_t path;
  string_t name;
} file_stat_t;

typedef struct {
  fs_error_t error;
  union {
    string_t string;
    size_t size;
  };
} fs_result_t;

// Low-level file descriptor operations
fd_t file_open(string_t path, file_mode_t mode);
fs_error_t file_close(fd_t fd);
fs_result_t file_read_fd(fd_t fd, allocator_t* allocator);
fs_result_t file_write_fd(fd_t fd, const void* data, size_t size);
fs_result_t file_append_fd(fd_t fd, const void* data, size_t size);

// High-level file operations
fs_result_t file_read(string_t path, allocator_t* allocator);
fs_result_t file_write(string_t path, const void* data, size_t size);
fs_result_t file_write_atomic(string_t path, const void* data, size_t size);
fs_result_t file_append(string_t path, const void* data, size_t size);

// File queries and manipulation
bool file_exists(string_t path);
bool file_delete(string_t path);
bool file_rename(string_t old_path, string_t new_path);
bool file_copy(string_t src_path, string_t dest_path);
bool file_is_newer(string_t path_a, string_t path_b);
file_stat_t file_stat(string_t path);
