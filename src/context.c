#include <am.h>
#include <klib.h>
#include <rtthread.h>

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  assert(0);
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  assert(0);
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

typedef void(*entry_t)(void* );
typedef void(*exit_t)();
void helper(void* para){

  entry_t tentry =(entry_t) ((uintptr_t*)para)[0];
  void* parameter = (void*)((uintptr_t*)para)[1];
  exit_t texit = (exit_t) ((uintptr_t*)para)[2];

  tentry(parameter);
  texit();
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  stack_addr = (stack_addr - ((uintptr_t)stack_addr % sizeof(uintptr_t)));//align stack, the pointer type is uint8* add 1 wont cause double;
  uintptr_t* para = (uintptr_t*) stack_addr - 3;
  para[0] = (uintptr_t)tentry;
  para[1] = (uintptr_t)parameter;
  para[2] = (uintptr_t)texit;
  stack_addr = (rt_uint8_t*)para;
  
  return (rt_uint8_t*)kcontext((Area){0, stack_addr}, helper, para);
}

