#pragma once

#include <stddef.h>
#include "intrusive_list.h"
#include "utils.h"

typedef struct
{
  intrusive_list_t list;
  size_t size;
} cb_stack_t;

typedef intrusive_list_node_t stack_element_t;

void stack_init(cb_stack_t* stack);

void stack_push(cb_stack_t* stack, stack_element_t* element);

stack_element_t* _stack_pop(cb_stack_t* stack);
#define stack_pop(stack, type, member) CONTAINER_OF(_stack_pop(stack), type, member)

stack_element_t* _stack_peek(cb_stack_t* stack);
#define stack_peek(stack, type, member) CONTAINER_OF(_stack_peek(stack), type, member)

bool stack_isempty(cb_stack_t* stack);

size_t stack_size(cb_stack_t *stack);