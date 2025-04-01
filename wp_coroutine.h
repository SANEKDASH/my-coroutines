#ifndef WP_COROUTINE_HEADER
#define WP_COROUTINE_HEADER

#include <stdlib.h>
#include <ucontext.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


#include "task_queue.h"

typedef struct {
  int is_live;
  int is_wait_for_tasks;
  
  int waiters_count;

  pthread_cond_t wait_for_task;
  pthread_cond_t wait_for_tasks_completed;
  
  pthread_mutex_t mt;

  pthread_t *threads;
  int threads_count;  

  task_queue_t task_queue;
} wp_coroutine_t;

int wp_coroutine_init(wp_coroutine_t *wp_coro, int worker_count);
int wp_coroutine_destroy(wp_coroutine_t *wp_coro);
int wp_coroutine_add_task(wp_coroutine_t *wp_coro, void (*new_task)(void *), void *arg);
int wp_coroutine_wait_for_tasks(wp_coroutine_t *wp_coro);

#endif
