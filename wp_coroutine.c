#include "wp_coroutine.h"

static void *wp_coroutine_idle(void *wp_coro);

int wp_coroutine_wait_for_tasks(wp_coroutine_t *wp_coro)
{
  pthread_mutex_lock(&wp_coro->mt);

  wp_coro->is_wait_for_tasks = 1;

  while (wp_coro->task_queue.task_count != 0) {
	pthread_cond_wait(&wp_coro->wait_for_tasks_completed,
					  &wp_coro->mt);
  }
  
  pthread_mutex_unlock(&wp_coro->mt);

  wp_coro->is_wait_for_tasks = 1;
  
  return 0;
}

int wp_coroutine_init(wp_coroutine_t *wp_coro, int worker_count)
{
  if (worker_count <= 0) {
	return -1;
  }
  
  wp_coro->is_live           = 1;
  wp_coro->is_wait_for_tasks = 0;
  
  wp_coro->threads_count = worker_count;
  wp_coro->waiters_count = 0;

  task_queue_create(&wp_coro->task_queue);


  /* Initializing conditional variable */

  pthread_cond_init (&wp_coro->wait_for_task,            NULL);
  pthread_cond_init (&wp_coro->wait_for_tasks_completed, NULL);

  pthread_mutex_init(&wp_coro->mt, NULL);


  /* Initializing thread attribute */
  
  pthread_attr_t thr_attr;
  pthread_attr_init(&thr_attr);
  
  pthread_attr_setschedpolicy(&thr_attr, SCHED_FIFO);
 
  
  /* Initializing threads */
    
  wp_coro->threads = (pthread_t *) malloc(sizeof(pthread_t) * wp_coro->threads_count);

  if (wp_coro->threads == NULL) {
	return -1;
  }
  
  for (int i = 0; i < wp_coro->threads_count; i++) {
	pthread_create(wp_coro->threads + i, &thr_attr, wp_coroutine_idle, (void *) wp_coro);
  }

  pthread_attr_destroy(&thr_attr);
  
  return 0;
}

int wp_coroutine_destroy(wp_coroutine_t *wp_coro)
{
  wp_coroutine_wait_for_tasks(wp_coro);
  
  wp_coro->is_live = 0;

  pthread_cond_broadcast(&wp_coro->wait_for_task);

  for (int i = 0; i < wp_coro->threads_count; i++) {
	pthread_join(wp_coro->threads[i], NULL);
  }

  pthread_mutex_destroy(&wp_coro->mt);
  pthread_cond_destroy (&wp_coro->wait_for_task);
  
  free(wp_coro->threads);

  task_queue_destroy(&wp_coro->task_queue);
  
  return 0;
}

int wp_coroutine_add_task(wp_coroutine_t *wp_coro, void (*new_task)(void *), void *arg)
{
  pthread_mutex_lock(&wp_coro->mt);

  task_queue_add_task(&wp_coro->task_queue, new_task, arg);

  if (wp_coro->task_queue.task_count > 0) {
	pthread_cond_signal(&wp_coro->wait_for_task);
  }
  
  pthread_mutex_unlock(&wp_coro->mt);

  return 0;
}

static void *wp_coroutine_idle(void *wp_coro_ptr)
{
  wp_coroutine_t *wp_coro = (wp_coroutine_t *) wp_coro_ptr;

  ucontext_t idle_ctx;
  
  while (1) {
	pthread_mutex_lock(&wp_coro->mt);

	if (wp_coro->is_wait_for_tasks) {
	  pthread_cond_signal(&wp_coro->wait_for_tasks_completed);
	}

	if (!wp_coro->is_live) {
	  pthread_mutex_unlock(&wp_coro->mt);
	  return NULL;
	}
	
	++wp_coro->waiters_count;

	while (wp_coro->task_queue.task_count == 0) {
	  if (!wp_coro->is_live) {
		pthread_mutex_unlock(&wp_coro->mt);
		return NULL;
	  }
	  
	  pthread_cond_wait(&wp_coro->wait_for_task,
						&wp_coro->mt);
	}

	--wp_coro->waiters_count;


	/*
	 * Getting new task.
	 *  
	 * If the queue is null then go next loop iteration.
	 * After that, if the amount of tasks is null again, go wait for them.
	 *
	 */
	
	if (wp_coro->task_queue.task_count == 0) {
	  if (wp_coro->is_wait_for_tasks) {
		pthread_cond_signal(&wp_coro->wait_for_task);
	  }

	  pthread_mutex_unlock(&wp_coro->mt);
	  continue;
	}
	
	struct task_queue_node *new_task = task_queue_pop(&wp_coro->task_queue);

   	pthread_mutex_unlock(&wp_coro->mt);

	ucontext_t   new_ctx;
	getcontext(&new_ctx);

	char *new_ctx_stack = (char *) malloc(16384 * sizeof(char)); // Need a constant.
	
	new_ctx.uc_stack.ss_sp   = new_ctx_stack;
	new_ctx.uc_stack.ss_size = 16384;
	new_ctx.uc_link          = &idle_ctx;
	
	makecontext(&new_ctx, (void (*)())new_task->task, 1, new_task->arg);
	
	/*
	 * Executing the task
	 *
	 */
	
	swapcontext(&idle_ctx, &new_ctx);

	free(new_ctx_stack);
	task_queue_node_destroy(new_task);
  }

  return NULL;
}
