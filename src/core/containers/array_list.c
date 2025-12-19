#include <string.h>
#include "core/container/array_list.h"

void* array_list_init(allocator_t* allocator, size_t initial_capacity, size_t element_size)
{
  void* ptr = NULL;
  size_t size = initial_capacity * element_size + sizeof(array_list_header_t);

  array_list_header_t* header = allocator_alloc(allocator, size, 0);

  if(!header) {
    return NULL;
  }

  header->length = 0;
  header->capacity = initial_capacity;
  header->allocator = allocator;
  ptr = header + 1;

  return ptr;
}

void* array_list_ensure_capacity(void* list, size_t element_count, size_t element_size)
{
  array_list_header_t* header = array_list_header(list);

  size_t needed_capacity = element_count + header->length;

  if(needed_capacity <= header->capacity) {
    return list;
  }

  size_t new_capacity = header->capacity * 2;
  while(needed_capacity > new_capacity) {
    new_capacity *= 2;
  }

  size_t new_size = new_capacity * element_size + sizeof(array_list_header_t);
  array_list_header_t* new_header = allocator_alloc(header->allocator, new_size, 0);

  if(!new_header) {
    return NULL;
  }

  new_header->length = header->length;
  new_header->capacity = new_capacity;
  new_header->allocator = header->allocator;

  void* new_list = new_header + 1;
  memcpy(new_list, list, header->length * element_size);

  return new_list;
}
