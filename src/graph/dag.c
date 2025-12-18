#include <stddef.h>
#include "array_list.h"
#include "dag.h"
#include "i_allocator.h"
#include "memory_context.h"

void dag_init(dag_t* dag, i_allocator_t* allocator)
{
  dag->nodes = array_list(dag_node_t*, allocator);
  dag->allocator = allocator;
  dag->next_id = 0;
}

void dag_add_node(dag_t* dag, dag_node_t* node)
{
  node->id = dag->next_id++;
  node->dependencies = array_list(dag_node_t*, dag->allocator);
  node->dependents = array_list(dag_node_t*, dag->allocator);

  array_list_push(dag->nodes, node);
}

// Graph building
bool dag_add_edge(dag_node_t* from, dag_node_t* to)
{
  if(dag_has_edge(from, to)) {
    return true;
  }

  array_list_push(from->dependencies, to);
  array_list_push(to->dependents, from);

  return true;
}

// Queries
size_t dag_node_count(dag_t* dag)
{
  return array_list_length(dag->nodes);
}

size_t dag_dependency_count(dag_node_t* node)
{
  return array_list_length(node->dependencies);
}

size_t dag_dependent_count(dag_node_t* node)
{
  return array_list_length(node->dependents);
}

bool dag_has_edge(dag_node_t* from, dag_node_t* to)
{
  bool exists;
  array_list_contains(from->dependencies, to, exists);
  return exists;
}

static bool is_cyclic(dag_node_t* node, dag_state_t* states)
{
  if(states[node->id] == DAG_VISITED) {
    return false;
  }

  if(states[node->id] == DAG_VISITING) {
    return true;
  }

  states[node->id] = DAG_VISITING;

  for(size_t i = 0; i < array_list_length(node->dependencies); i++) {
    if(is_cyclic(node->dependencies[i], states)) {
      return true;
    }
  }

  states[node->id] = DAG_VISITED;
  return false;
}

// Traversal
bool dag_has_cycle(dag_t* dag)
{
  size_t count = array_list_length(dag->nodes);

  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  dag_state_t* states = allocator_alloc_arr(temp_alloc, dag_state_t, count);

  for(size_t i = 0; i < count; i++) {
    states[i] = DAG_UNVISITED;
  }

  bool has_cycle = false;

  for(size_t i = 0; i < count; i++) {
    if(states[dag->nodes[i]->id] == DAG_UNVISITED) {
      if(is_cyclic(dag->nodes[i], states)) {
        has_cycle = true;
        break;
      }
    }
  }

  scratch_end(scratch);
  return has_cycle;
}

dag_node_t** dag_topo_sort(dag_t* dag, i_allocator_t* alloc)
{
  size_t count = array_list_length(dag->nodes);

  scratch_t* scratch = scratch_begin();
  i_allocator_t* temp_alloc = scratch_allocator(scratch);

  size_t* incoming_degrees = allocator_alloc(temp_alloc, sizeof(size_t) * count, alignof(size_t));

  array_list_for_each(dag->nodes, dag_node_t*, node)
  {
    incoming_degrees[_i] = array_list_length(node->dependencies);
  }

  dag_node_t** queue = array_list(dag_node_t*, temp_alloc);

  array_list_for_each(dag->nodes, dag_node_t*, node)
  {
    if(incoming_degrees[node->id] == 0) {
      array_list_push(queue, node);
    }
  }

  dag_node_t** result = array_list(dag_node_t*, alloc);
  size_t queue_idx = 0;

  while(queue_idx < array_list_length(queue)) {
    dag_node_t* node = queue[queue_idx++];
    array_list_push(result, node);

    for(size_t i = 0; i < array_list_length(node->dependents); i++) {
      dag_node_t* dependent = node->dependents[i];
      incoming_degrees[dependent->id]--;

      if(incoming_degrees[dependent->id] == 0) {
        array_list_push(queue, dependent);
      }
    }
  }

  scratch_end(scratch);

  if(array_list_length(result) != count) {
    return NULL;
  }

  return result;
}

dag_node_t** dag_get_roots(dag_t* dag, i_allocator_t* alloc) // nodes with no dependents
{
  dag_node_t** result = array_list(dag_node_t*, alloc);

  array_list_for_each(dag->nodes, dag_node_t*, node)
  {
    if(dag_dependent_count(node) == 0) {
      array_list_push(result, node);
    }
  }

  return result;
}

dag_node_t** dag_get_leaves(dag_t* dag, i_allocator_t* alloc) // nodes with no dependencies
{
  dag_node_t** result = array_list(dag_node_t*, alloc);

  array_list_for_each(dag->nodes, dag_node_t*, node)
  {
    if(dag_dependency_count(node) == 0) {
      array_list_push(result, node);
    }
  }

  return result;
}
