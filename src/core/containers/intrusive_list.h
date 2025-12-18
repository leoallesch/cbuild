#pragma once

#include <stdbool.h>

// Intrusive linked list node - embed this in your structs
typedef struct intrusive_list_node_t {
  struct intrusive_list_node_t* next;
  struct intrusive_list_node_t* prev;
} intrusive_list_node_t;

// List head/anchor
typedef struct intrusive_list_t {
  intrusive_list_node_t head; // Sentinel node (circular)
} intrusive_list_t;

// Iterate over list entries
#define intrusive_list_foreach(pos, list, type, member)            \
  for(pos = intrusive_list_entry((list)->head.next, type, member); \
    &pos->member != &(list)->head;                                 \
    pos = intrusive_list_entry(pos->member.next, type, member))

// Iterate over list entries (safe for removal)
#define intrusive_list_foreach_safe(pos, tmp, list, type, member)  \
  for(pos = intrusive_list_entry((list)->head.next, type, member), \
  tmp = intrusive_list_entry(pos->member.next, type, member);      \
    &pos->member != &(list)->head;                                 \
    pos = tmp, tmp = intrusive_list_entry(tmp->member.next, type, member))

void intrusive_list_init(intrusive_list_t* list);
bool intrusive_list_empty(const intrusive_list_t* list);
bool intrusive_list_contains(const intrusive_list_t* list, const intrusive_list_node_t* node);
void intrusive_list_push_front(intrusive_list_t* list, intrusive_list_node_t* node);
void intrusive_list_push_back(intrusive_list_t* list, intrusive_list_node_t* node);
void intrusive_list_remove(intrusive_list_node_t* node);
intrusive_list_node_t* intrusive_list_front(intrusive_list_t* list);
intrusive_list_node_t* intrusive_list_back(intrusive_list_t* list);
intrusive_list_node_t* intrusive_list_pop_front(intrusive_list_t* list);
intrusive_list_node_t* intrusive_list_pop_back(intrusive_list_t* list);
