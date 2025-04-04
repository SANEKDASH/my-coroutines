#include <stdio.h>
#include <ucontext.h>

void func1(void)
{
  printf("%s\n", __func__);
}

int main()
{
  ucontext_t main_ctx;
  ucontext_t func1_ctx;

  char main_ctx_stack[16384];
  char func1_ctx_stack[16384];
  
  getcontext(&func1_ctx);

  func1_ctx.uc_stack.ss_sp   =        main_ctx_stack;
  func1_ctx.uc_stack.ss_size = sizeof(main_ctx_stack);
  func1_ctx.uc_link          = &main_ctx;

  makecontext(&func1_ctx, func1, 0);

  swapcontext(&main_ctx, &func1_ctx);

  printf("Hello!\n");
  
  return 0;
}
