#pragma once

#include <stdint.h>
#include "i_allocator.h"

typedef enum {
    DAG_UNVISITED = 0,
    DAG_VISITING,
    DAG_VISITED
} dag_state_t;

typedef struct dag_node_t {
  uint32_t id;
  struct dag_node_t** dependencies;
  struct dag_node_t** dependents;
} dag_node_t;

typedef struct
{
  dag_node_t** nodes;
  i_allocator_t* allocator;
  uint32_t next_id;
} dag_t;

void dag_init(dag_t* dag, i_allocator_t* allocator);

// Graph building
void dag_add_node(dag_t* dag, dag_node_t* node);
bool dag_add_edge(dag_node_t* from, dag_node_t* to); // from depends on to

// Queries
size_t dag_node_count(dag_t* dag);
size_t dag_dependency_count(dag_node_t* node);
size_t dag_dependent_count(dag_node_t* node);
bool dag_has_edge(dag_node_t* from, dag_node_t* to);

// Traversal
bool dag_has_cycle(dag_t* dag);
dag_node_t** dag_topo_sort(dag_t* dag, i_allocator_t* alloc); // returns array_list
dag_node_t** dag_get_roots(dag_t* dag, i_allocator_t* alloc); // nodes with no dependents
dag_node_t** dag_get_leaves(dag_t* dag, i_allocator_t* alloc); // nodes with no dependencies

// // Get nodes ready to execute (all dependencies satisfied)
// dag_node_t** dag_get_ready(dag_t* dag, i_allocator_t* alloc);

// // Mark node as completed (updates ready set)
// void dag_mark_complete(dag_t* dag, dag_node_t* node);

// // Reset traversal state (for re-running)
// void dag_reset(dag_t* dag);