#pragma once

#include "i_allocator.h"

#define ARRAY_LIST_INIT_CAPACITY 16

typedef struct
{
  size_t length;
  size_t capacity;
  size_t _padding;
  i_allocator_t* allocator;
} array_list_header_t;

void* array_list_init(i_allocator_t* allocator, size_t initial_capacity, size_t element_size);

void* array_list_ensure_capacity(void* list, size_t element_count, size_t element_size);

#define array_list(type, alloc) (type*)array_list_init(alloc, ARRAY_LIST_INIT_CAPACITY, sizeof(type))
#define array_list_header(list) (((array_list_header_t*)(list)) - 1)
#define array_list_length(list) (array_list_header(list)->length)
#define array_list_empty(list) (array_list_length(list) == 0)
#define array_list_capacity(list) (array_list_header(list)->capacity)

#define array_list_push(list, element)                         \
  do {                                                         \
    list = array_list_ensure_capacity(list, 1, sizeof(*list)); \
    array_list_header(list)->length++;                         \
    list[array_list_length(list) - 1] = element;               \
  } while(0)

#define array_list_pop(list)             \
  do {                                   \
    if(array_list_length(list) > 0) {    \
      array_list_header(list)->length--; \
    }                                    \
  } while(0)

#define array_list_remove(list, i)                         \
  do {                                                     \
    array_list_header_t* header = array_list_header(list); \
    if(i < header->length) {                               \
      for(size_t _j = i; _j < header->length - 1; _j++) {  \
        list[_j] = list[_j + 1];                           \
      }                                                    \
      header->length--;                                    \
    }                                                      \
  } while(0)

#define array_list_clear(list) array_list_header(list)->length = 0;

#define array_list_for_each(list, type, item)                         \
  for(size_t _i = 0, _len = array_list_length(list); _i < _len; _i++) \
    for(type item = (list)[_i], *_once = (void*)1; _once; _once = NULL)

#define array_list_contains(list, element, result)           \
  do {                                                       \
    result = false;                                          \
    for(size_t _i = 0; _i < array_list_length(list); _i++) { \
      if((list)[_i] == (element)) {                          \
        result = true;                                       \
        break;                                               \
      }                                                      \
    }                                                        \
  } while(0)
