#ifndef TASK_QUEUE_HEADER
#define TASK_QUEUE_HEADER

#include <stdio.h>
#include <ucontext.h>

/////////
//TODO:// I need to implement tasks with use of ucontext_t.
///////// Because coroutines functionality assumes exiting function
///////// in many points.

struct task_queue_node{
  struct task_queue_node *next;
  
  void (* task)(void *);

  void *arg;
};

typedef struct {
  struct task_queue_node *head;
  struct task_queue_node *tail;

  int task_count;
  
} task_queue_t;


int task_queue_destroy(task_queue_t *tl);
int task_queue_create(task_queue_t *tl);

int task_queue_add_task(task_queue_t *tl, void (* func)(void *), void *arg);

struct task_queue_node *task_queue_pop(task_queue_t *tl);

int task_queue_node_destroy(struct task_queue_node *node);

#endif
