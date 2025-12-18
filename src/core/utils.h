#pragma once

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define OFFSET_OF(type, member) ((size_t)(&((type*)0)->member))
#define CONTAINER_OF(ptr, type, member) ((type*)((char*)(ptr) - OFFSET_OF(type, member)))

#define NUM_ELEMENTS(arr) (sizeof(arr) / sizeof(arr[0]))

#define defer(end)            \
  for(int _done = 0; !_done;) \
    for(; !(_done++); end)

#define _defer(...)                              \
  auto void CONCAT(_defer_fn_, __LINE__)(void) \
  {                                              \
    __VA_ARGS__                                  \
  }                                              \
  for(int _done = 0; !_done;)                    \
    for(; !_done; _done = 1, CONCAT(_defer_fn_, __LINE__)())
