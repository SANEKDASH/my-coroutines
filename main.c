#include <stdio.h>

#include "wp_coroutine.h"

void func1()
{
  printf("Hello1!\n");
}


void func2()
{
  printf("Hello2!\n");
}


int main()
{
  wp_coroutine_t coro;

  wp_coroutine_init(&coro, 10);

  wp_coroutine_add_task(&coro, func1, NULL); 
  wp_coroutine_add_task(&coro, func2, NULL); 
  
  wp_coroutine_wait_for_tasks(&coro);
  
  wp_coroutine_destroy(&coro);
    
  return 0;
}
