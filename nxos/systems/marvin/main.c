#include "base/mytypes.h"
#include "base/tlsf.h"
#include "base/memmap.h"
#include "base/interrupts.h"
#include "base/display.h"
#include "base/drivers/sound.h"
#include "base/drivers/systick.h"
#include "base/util.h"
#include "task.h"

typedef struct task {
  U32 *stack_base; /* The allocated stack base. */
  U32 *stack_current; /* The current stack pointer of the running task. */
  struct task *next; /* Pointer to the next task in the list (circular list). */
} task_t;

volatile task_t *current_task = NULL;

void test_beep() {
  while(1) {
    sound_freq(440, 500);
    systick_wait_ms(1500);
  }
}

void test_display() {
  U32 counter = 0;
  while(1) {
    counter++;
    display_clear();
    display_uint(counter);
    systick_wait_ms(250);
  }
}

volatile struct task_state test;
U32 pc, cpsr, test2, test3;
extern U32 __irq_stack__;
U32 *irq = ((U32*)(&__irq_stack__));
volatile U32 counter = 0, counter2 = 0;


inline void dump_task(struct task_state *t) {
    display_cursor_set_pos(0,0);
    display_hex(t->cpsr); display_string(" "); display_hex(t->pc); display_string("\n");
    display_hex(t->r0); display_string(" "); display_hex(t->r1); display_string("\n");
    display_hex(t->r2); display_string(" "); display_hex(t->r3); display_string("\n");
    display_hex(t->r4); display_string(" "); display_hex(t->r5); display_string("\n");
    display_uint(counter); display_string(" "); display_uint(counter2); display_end_line();
    display_hex(t->r8); display_string(" "); display_hex(t->r9); display_string("\n");
    display_hex(t->r10); display_string(" "); display_hex(t->r11); display_string("\n");
    display_hex(t->r12); display_string(" "); display_hex(t->lr);
}

void systick_cb() {
  static int mod = 0;
  task_t *next = current_task->next;
/*   memcpy(&test, current_task->stack_base+1024-sizeof(test), sizeof(test)); */
  counter++;
  dump_task((struct task_state*)get_system_stack());
  display_cursor_set_pos(0,0);
/*   display_hex(get_system_stack()[2]); */
  pc = irq[-1];
  cpsr = irq[-2];
  test2 = irq[0];
  test3 = irq[-3];
  return;

  mod = (mod + 1) % 100;

  if (next != current_task && mod == 0)
    current_task->stack_current = swap_task_stacks(next->stack_current);
}

static task_t *new_task(closure_t func) {
  struct task_state *state = NULL;
  task_t *task = rtl_malloc(sizeof(*task));
  task->stack_base = rtl_malloc(1024);
  task->stack_current = task->stack_base + 1024 - sizeof(*task);
  task->next = task;
  state = (struct task_state*) (task->stack_base + 1024 - sizeof(*state));
  state->pc = (U32)func;
  state->cpsr = 0x1F; // TODO: nice #define
  return task;
}

void test_idle() {
  //interrupts_enable();
  while(1) {
/*     display_clear(); */
/*     display_hex(test.cpsr); display_string(" "); display_hex(test.pc); display_string(" cpsr/pc\n"); */
/*     display_hex(cpsr); display_string(" "); display_hex(pc); display_string(" cpsr/pc\n"); */
/*     display_hex(test2); display_string(" "); display_hex(test3); display_string(" cpsr/pc\n"); */
/*     dump_task(&test); */
    counter2++;
    systick_wait_ms(250);
  }
}

void main() {
  task_t *task1, *task2, *idle_task;
  init_memory_pool(USERSPACE_SIZE, USERSPACE_START);
  task1 = new_task(test_idle);
  task2 = new_task(test_display);
  idle_task = new_task(NULL);
  task1->next = task2;
  task2->next = idle_task;
  idle_task->next = task1;
  idle_task->stack_current += sizeof(struct task_state);
  current_task = idle_task;

  //interrupts_disable();
  systick_install_scheduler(systick_cb);
  run_first_task(test_idle, idle_task->stack_current+1024);
}
