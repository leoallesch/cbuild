#include "hash_table.h"
#include "i_allocator.h"
#include "string_helper.h"

#include <stdio.h>

#define INITIAL_CAPACITY 16

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3

static uint64_t hash(string_t key)
{
  uint64_t hash = FNV_OFFSET_BASIS;
  for(size_t i = 0; i < string_length(key) - 1; i++) {
    hash ^= string_at(key, i);
    hash *= FNV_PRIME;
  }
  return hash;
}

void hash_table_print(hash_table_t* table)
{
  for(size_t i = 0; i < table->capacity; i++) {
    hash_table_entry_t* entry = table->entries[i];
    if(entry) {
      printf("%ld ", i);
      while(entry != NULL) {
        printf(STR_FMT": %p -> ", STR_ARG(entry->key), entry->value);
        entry = entry->next;
      }
      printf("\n");
    }
  }
}

void hash_table_set(hash_table_t* ht, string_t key, const void* value)
{
  uint64_t index = hash(key) % ht->capacity;

  hash_table_entry_t* new_entry = allocator_alloc(ht->allocator, sizeof(hash_table_entry_t), 0);

  new_entry->key = key;
  new_entry->value = (void*)value;
  new_entry->next = ht->entries[index];

  ht->entries[index] = new_entry;

  ht->size++;
}

void* hash_table_get(hash_table_t* ht, string_t key)
{
  uint64_t index = hash(key) % ht->capacity;
  hash_table_entry_t* entry = ht->entries[index];

  while(entry != NULL) {
    if(string_equals(entry->key, key)) {
      return entry->value;
    }
    entry = entry->next;
  }
  return NULL;
}

hash_table_t hash_table_init(i_allocator_t* allocator)
{
  hash_table_entry_t** entries = allocator_alloc(allocator, sizeof(hash_table_entry_t*) * INITIAL_CAPACITY, 0);
  if(entries == NULL)
    return (hash_table_t){};

  return (hash_table_t){
    .allocator = allocator,
    .capacity = INITIAL_CAPACITY,
    .size = 0,
    .entries = entries
  };
}
