#include <am.h>
#include <klib.h>
#include <rtthread.h>
//#define RISCV_ONLY

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD:
      #ifdef CONFIG_ISA_riscv
      if(c->gpr[12] == 0){
        c = *(Context**) c->gpr[10];
      }else if(c->gpr[12] == 1){
        *(Context**)c->gpr[10] = c;
        c = *(Context**) c->gpr[11];
      }
      break;
      #endif
      rt_thread_t current = rt_thread_self();
      rt_ubase_t* data = (rt_ubase_t*)(current->user_data);
      if(data[0] == 0){
        c = *(Context**) data[1];
      }else if(data[0] == 1){
        *(Context**)data[1] = c;
        c = *(Context**) data[2];
      }else{
        printf("flag: %08x", data[0]);
        assert(0);
      }
      break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  #ifdef RISCV_ONLY
  asm volatile("li a2, 0");
  yield();
  #endif

  rt_ubase_t data[2] = {0, to};

  rt_thread_t current = rt_thread_self();
  rt_ubase_t tmp_data = current->user_data;
  current->user_data = (rt_ubase_t)data;
  yield();
  current->user_data = tmp_data;
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  #ifdef RISCV_ONLY
  asm volatile("li a2, 1");
  yield();
  #endif

  rt_ubase_t data[3] = {1, from, to};

  rt_thread_t current = rt_thread_self();
  rt_ubase_t tmp_data = current->user_data;
  current->user_data = (rt_ubase_t)data;
  yield();
  current->user_data = tmp_data;

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

