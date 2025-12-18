#pragma once

#include "i_allocator.h"
#include "string_helper.h"

typedef struct hash_table_entry_t {
  string_t key;
  void* value;
  struct hash_table_entry_t* next;
} hash_table_entry_t;

typedef struct
{
  hash_table_entry_t** entries;
  i_allocator_t* allocator;
  size_t capacity;
  size_t size;
} hash_table_t;

hash_table_t hash_table_init(i_allocator_t* allocator);
void hash_table_set(hash_table_t* table, string_t key, const void* value);
void* hash_table_get(hash_table_t* table, string_t key);
void hash_table_remove(hash_table_t* table, string_t key);

void hash_table_print(hash_table_t* table);

inline size_t hash_table_size(hash_table_t* table)
{
  return table->size;
}

inline size_t hash_table_capacity(hash_table_t* table)
{
  return table->capacity;
}
