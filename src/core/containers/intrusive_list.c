#include <stddef.h>
#include "intrusive_list.h"

static void intrusive_list__insert(intrusive_list_node_t* node,
  intrusive_list_node_t* prev,
  intrusive_list_node_t* next)
{
  next->prev = node;
  node->next = next;
  node->prev = prev;
  prev->next = node;
}

void intrusive_list_init(intrusive_list_t* list)
{
  list->head.next = &list->head;
  list->head.prev = &list->head;
}

bool intrusive_list_empty(const intrusive_list_t* list)
{
  return list->head.next == &list->head;
}

bool intrusive_list_contains(const intrusive_list_t* list, const intrusive_list_node_t* node)
{
  for(intrusive_list_node_t* cur = list->head.next; cur != &list->head; cur = cur->next) {
    if(cur == node) {
      return true;
    }
  }
  return false;
}

void intrusive_list_push_front(intrusive_list_t* list, intrusive_list_node_t* node)
{
  intrusive_list__insert(node, &list->head, list->head.next);
}

void intrusive_list_push_back(intrusive_list_t* list, intrusive_list_node_t* node)
{
  intrusive_list__insert(node, list->head.prev, &list->head);
}

void intrusive_list_remove(intrusive_list_node_t* node)
{
  node->prev->next = node->next;
  node->next->prev = node->prev;
  node->next = NULL;
  node->prev = NULL;
}

intrusive_list_node_t* intrusive_list_front(intrusive_list_t* list)
{
  return intrusive_list_empty(list) ? NULL : list->head.next;
}

intrusive_list_node_t* intrusive_list_back(intrusive_list_t* list)
{
  return intrusive_list_empty(list) ? NULL : list->head.prev;
}

intrusive_list_node_t* intrusive_list_pop_front(intrusive_list_t* list)
{
  if(intrusive_list_empty(list)) {
    return NULL;
  }
  intrusive_list_node_t* node = list->head.next;
  intrusive_list_remove(node);
  return node;
}

intrusive_list_node_t* intrusive_list_pop_back(intrusive_list_t* list)
{
  if(intrusive_list_empty(list)) {
    return NULL;
  }
  intrusive_list_node_t* node = list->head.prev;
  intrusive_list_remove(node);
  return node;
}
