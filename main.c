#include "my_coro/my_coro.h"
#include <stdio.h>

struct my_coro coro;

char buf[1000000000];

void print_numbers()
{
  for (int i = 0; i < 10; i++) {  
	for (int i = 0; i < sizeof(buf); i++) {
	  buf[i] = 1;
	}

	printf("%d\n", i);
	my_coro_yield(&coro);
  } 
}

int main()
{
  if (my_coro_init(&coro, print_numbers)) {
	perror("failed to init coro");
  }
  
  my_coro_start(&coro);
  my_coro_start(&coro);
  my_coro_start(&coro);

  my_coro_wait_for_complete(&coro);
  
  my_coro_destroy(&coro);

  return 0;
}
