#include "my_coro.h"

static void *my_coro_idle(void *coro);

int my_coro_init(struct my_coro *my_coro, void (*func)(void))
{ 
  int err = getcontext(&my_coro->ctx);
  if (err) {
	perror("my_coro_init(): failed fetcontext()");
	return err;
  }
  
  my_coro->ctx.uc_stack.ss_sp = (char *) malloc(sizeof(char) * MY_CORO_STACK_SIZE);
  if (my_coro->ctx.uc_stack.ss_sp == NULL) {
	perror("my_coro_init(): failed to alloc memory");	
	return -1;
  }
  
  my_coro->ctx.uc_stack.ss_size = MY_CORO_STACK_SIZE;
  my_coro->ctx.uc_link          = &my_coro->swap_ctx;

  makecontext(&my_coro->ctx, func, 0);

  my_coro->is_live   = 1;
  my_coro->run_count = 0;
  
  err = pthread_cond_init(&my_coro->wait_run, NULL);
  if (err) {
	perror("failed pthread_cond_init");
	return err;
  }

  err = pthread_cond_init(&my_coro->wait_complete, NULL);
  if (err) {
	perror("failed pthread_cond_init");
	return err;
  }
  
  // mutex attributes?
  err = pthread_mutex_init(&my_coro->mt, NULL);
  if (err) {
	perror("failed pthread_mutex_init");
	return err;
  }
  
  err = pthread_create(&my_coro->thr, NULL, my_coro_idle, (void *) my_coro);
  if (err) {
	perror("failed pthread_create");
	return err;
  }
  
  return 0;
}

void *my_coro_idle(void *my_coro_p)
{
  struct my_coro *my_coro = (struct my_coro *) my_coro_p;
  
  while (my_coro->is_live) {
	pthread_mutex_lock(&my_coro->mt);

	while(my_coro->run_count <= 0  && my_coro->is_live) {
	  pthread_cond_wait(&my_coro->wait_run, &my_coro->mt);
	}

	--my_coro->run_count;

	pthread_mutex_unlock(&my_coro->mt);
	
	if (my_coro->is_live) {
	  swapcontext(&my_coro->swap_ctx, &my_coro->ctx);  
	}

	if (my_coro->run_count == 0) {
	  pthread_cond_signal(&my_coro->wait_complete);
	}
  }
  
  return NULL;
}

int my_coro_yield(struct my_coro *my_coro)
{ 
  swapcontext(&my_coro->ctx, &my_coro->swap_ctx);
  return 0;
}

int my_coro_start(struct my_coro *my_coro)
{
  pthread_mutex_lock(&my_coro->mt);
  
  ++my_coro->run_count;

  pthread_mutex_unlock(&my_coro->mt);
  
  pthread_cond_signal(&my_coro->wait_run);

  return 0;
}

int my_coro_wait_for_complete(struct my_coro *my_coro)
{
  pthread_mutex_lock(&my_coro->mt);
  
  while (my_coro->run_count) {
	pthread_cond_wait(&my_coro->wait_complete, &my_coro->mt);
  }

  pthread_mutex_unlock(&my_coro->mt);
  
  return 0;
}

int my_coro_destroy(struct my_coro *my_coro)
{
  my_coro->is_live = 0;

  pthread_cond_signal(&my_coro->wait_run);
  pthread_join(my_coro->thr, NULL);
  
  pthread_mutex_destroy(&my_coro->mt);
  pthread_cond_destroy(&my_coro->wait_run);
  free(my_coro->ctx.uc_stack.ss_sp);  

  return 0;
}
