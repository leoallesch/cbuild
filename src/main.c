#include <stdio.h>

#include "memory/memory.h"

int main()
{
  memory_context_t ctx = memory_context_create(MB(100), KB(10));

  int* arr = context_alloc_global_type(&ctx, int, 10);
  
  for (int i = 0; i < 10; i++) {
    arr[i] = i;
  }
  
  for (int i = 0; i < 10; i++) {
    printf("%d ", arr[i]);
  }

  printf("\nHello, cbuild!\n");
  
  memory_context_destroy(&ctx);
  
  return 0;
}
