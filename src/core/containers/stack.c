#include "core/container/stack.h"

void stack_init(cb_stack_t* stack)
{
  intrusive_list_init(&stack->list);
  stack->size = 0;
}

void stack_push(cb_stack_t* stack, stack_element_t* element)
{
  intrusive_list_push_back(&stack->list, element);
  stack->size++;
}

stack_element_t* _stack_pop(cb_stack_t* stack)
{
  stack->size--;
  return intrusive_list_pop_back(&stack->list);
}

stack_element_t* _stack_peek(cb_stack_t* stack)
{
  return stack->list.head.prev;
}

bool stack_isempty(cb_stack_t* stack)
{
  return stack->size == 0;
}

size_t stack_size(cb_stack_t* stack)
{
  return stack->size;
}
