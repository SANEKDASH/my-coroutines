#ifndef MY_CORO_HEADER
#define MY_CORO_HEADER

#include <ucontext.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static const size_t MY_CORO_STACK_SIZE = 1024 * 16; // bytes

struct my_coro {
  ucontext_t ctx;
  ucontext_t swap_ctx;

  pthread_t thr;
  pthread_cond_t wait_run;
  pthread_cond_t wait_complete;
  pthread_mutex_t mt;
  
  int run_count;
  int is_live;
};

int my_coro_init(struct my_coro *coro, void (*func)(void));

int my_coro_wait_for_complete(struct my_coro *my_coro);

int my_coro_yield(struct my_coro *coro);
int my_coro_start(struct my_coro *coro);

int my_coro_destroy(struct my_coro *coro);

#endif  /* MY_CORO_HEADER */
