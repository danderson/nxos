#include "base/mytypes.h"
#include "base/tlsf.h"
#include "base/memmap.h"
#include "base/interrupts.h"
#include "base/display.h"
#include "base/drivers/sound.h"
#include "base/drivers/systick.h"
#include "base/util.h"
#include "task.h"

/* Data structures to maintain task state. */
typedef struct task {
  U32 *stack_base; /* The allocated stack base. */
  U32 *stack_current; /* The current stack pointer of the running task. */
  struct task *next; /* Pointer to the next task in the list (circular list). */
} task_t;

volatile task_t *current_task = NULL;

/* The main scheduler code. */
void systick_cb() {
  static int mod = 0;

  mod = (mod + 1) % 1000;

  if (mod == 0) {
    current_task->stack_current = get_system_stack();
    current_task = current_task->next;
    set_system_stack(current_task->stack_current);
  }
}

/* Create a new task, ready to be fired up. */
static task_t *new_task(closure_t func) {
  struct task_state *state = NULL;
  task_t *task = rtl_malloc(sizeof(*task));
  task->stack_base = rtl_malloc(1024);
  task->stack_current = task->stack_base + 1024 - sizeof(*state);
  task->next = task;
  state = (struct task_state*)task->stack_current;
  state->pc = (U32)func;
  state->cpsr = 0x1F; // TODO: nice #define
  if (state->pc & 0x1) {
    state->pc &= 0xFFFFFFFE;
    state->cpsr |= 0x20;
  }
  return task;
}

/* Debug code. Dump the full state of a task. */
inline void dump_task(struct task_state *t) {
  display_clear();
  display_hex(t->cpsr); display_string(" "); display_hex(t->pc); display_string("\n");
  display_hex(t->r0); display_string(" "); display_hex(t->r1); display_string("\n");
  display_hex(t->r2); display_string(" "); display_hex(t->r3); display_string("\n");
  display_hex(t->r4); display_string(" "); display_hex(t->r5); display_string("\n");
  display_hex(t->r6); display_string(" "); display_hex(t->r7); display_string("\n");
  display_hex(t->r8); display_string(" "); display_hex(t->r9); display_string("\n");
  display_hex(t->r10); display_string(" "); display_hex(t->r11); display_string("\n");
  display_hex(t->r12); display_string(" "); display_hex(t->lr);
}

/* Test tasks. The idle task, a beeper, and a counter display task. */
void test_idle() {
  interrupts_enable();
  while(1);
}

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
    if (!counter)
      display_clear();
    display_cursor_set_pos(0,0);
    display_uint(counter);
  }
}

void main() {
  task_t *task1, *task2, *idle_task;
  init_memory_pool(USERSPACE_SIZE, USERSPACE_START);
  idle_task = new_task(NULL);
  task1 = new_task(test_beep);
  task2 = new_task(test_display);
  task1->next = task2;
  task2->next = idle_task;
  idle_task->next = task1;
  idle_task->stack_current += sizeof(struct task_state);
  current_task = idle_task;

  interrupts_disable();
  systick_install_scheduler(systick_cb);
  run_first_task(test_idle, idle_task->stack_current+1024);
}
