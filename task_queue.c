#include <stdlib.h>

#include "task_queue.h"

static struct task_queue_node *task_queue_node_create();

static struct task_queue_node *task_queue_node_create(void (* new_task)(void *), void *new_arg)
{
  struct task_queue_node *new_node = (struct task_queue_node *) malloc(sizeof(struct task_queue_node));
  
  new_node->next = NULL;
  new_node->task = new_task;
  new_node->arg  = new_arg;
  
  return new_node;
}

int task_queue_node_destroy(struct task_queue_node *node)
{
  free(node);
  
  return 0;
}

int task_queue_create(task_queue_t *tl)
{
  tl->head = tl->tail = NULL;

  tl->task_count = 0;
  
  return 0;
}

int task_queue_add_task(task_queue_t *tl, void (* func)(void *), void *arg)
{
  struct task_queue_node *new_node = task_queue_node_create(func, arg);

  if (new_node == NULL) {
	perror("failed to allocate new node");
	return -1;
  }

  if (tl->tail == NULL) {
	tl->head = new_node;
  } else {
	tl->tail->next = new_node;
  }

  tl->tail = new_node;

  ++tl->task_count;
  
  return 0;
}

struct task_queue_node *task_queue_pop(task_queue_t *tl)
{
  struct task_queue_node *old_tail = tl->head; 

  if (old_tail == NULL) {
	return NULL;
  }

  if (tl->tail == tl->head) {	
	tl->head = tl->tail = NULL;
  } else {
	tl->head = tl->head->next;
  }

  --tl->task_count;
  
  return old_tail;
}

int task_queue_destroy(task_queue_t *tl)
{
  struct task_queue_node *cur_node = tl->head;

  while (cur_node != NULL) {
	struct task_queue_node *next_node = cur_node->next;

	task_queue_node_destroy(cur_node);

	cur_node = next_node;
  }
  
  return 0;
}



