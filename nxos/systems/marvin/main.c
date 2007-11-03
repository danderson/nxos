#include "base/types.h"
#include "base/tlsf.h"
#include "base/memmap.h"
#include "base/interrupts.h"
#include "base/display.h"
#include "base/drivers/sound.h"
#include "base/drivers/systick.h"
#include "base/drivers/avr.h"
#include "base/drivers/lcd.h"
#include "base/drivers/usb.h"
#include "base/util.h"
#include "task.h"

/* Number of milliseconds to let tasks run between context switches. */
#define TASK_SWITCH_RESOLUTION 10

/* Data structures to maintain task state. */
typedef struct task {
  U32 *stack_base; /* The allocated stack base. */
  U32 *stack_current; /* The current stack pointer of the running task. */
  struct task *next; /* Pointer to the next task in the list (circular list). */
} task_t;

task_t *available_tasks = NULL;
task_t *current_task = NULL;
task_t *idle_task = NULL;

static void shutdown() {
  nx_lcd_shutdown();
  nx_usb_disable();
  nx_avr_power_down();
}

/* The main scheduler code. */
void systick_cb() {
  static int mod = 0;

  if (nx_avr_get_button() == BUTTON_CANCEL)
    shutdown();

  mod = (mod + 1) % TASK_SWITCH_RESOLUTION;

  if (mod == 0) {
    current_task->stack_current = get_system_stack();
    if (current_task == idle_task && available_tasks != NULL) {
      current_task = available_tasks;
    } else if (current_task->next != current_task) {
      current_task = current_task->next;
    }
    set_system_stack(current_task->stack_current);
  }
}

/* Create a new task, ready to be fired up. */
static task_t *new_task(nx_closure_t func) {
  nx_task_stack_t *state = NULL;
  task_t *task = malloc(sizeof(*task));
  task->stack_base = malloc(1024);
  task->stack_current = task->stack_base + 1024 - sizeof(*state);
  task->next = task;
  state = (nx_task_stack_t*)task->stack_current;
  state->pc = (U32)func;
  state->cpsr = 0x1F; // TODO: nice #define
  if (state->pc & 0x1) {
    state->pc &= 0xFFFFFFFE;
    state->cpsr |= 0x20;
  }
  return task;
}

/* Debug code. Dump the full state of a task. */
inline void dump_task(nx_task_stack_t *t) {
  nx_display_clear();
  nx_display_hex(t->cpsr); nx_display_string(" "); nx_display_hex(t->pc); nx_display_string("\n");
  nx_display_hex(t->r0); nx_display_string(" "); nx_display_hex(t->r1); nx_display_string("\n");
  nx_display_hex(t->r2); nx_display_string(" "); nx_display_hex(t->r3); nx_display_string("\n");
  nx_display_hex(t->r4); nx_display_string(" "); nx_display_hex(t->r5); nx_display_string("\n");
  nx_display_hex(t->r6); nx_display_string(" "); nx_display_hex(t->r7); nx_display_string("\n");
  nx_display_hex(t->r8); nx_display_string(" "); nx_display_hex(t->r9); nx_display_string("\n");
  nx_display_hex(t->r10); nx_display_string(" "); nx_display_hex(t->r11); nx_display_string("\n");
  nx_display_hex(t->r12); nx_display_string(" "); nx_display_hex(t->lr);
}

/* Test tasks. The idle task, a beeper, and a counter display task. */
void test_idle() {
  nx_interrupts_enable();
  while(1);
}

void test_beep() {
  while(1) {
    nx_sound_freq(440, 500);
    nx_systick_wait_ms(1500);
  }
}

void test_display() {
  U32 counter = 0;
  while(1) {
    counter++;
    if (!counter)
      nx_display_clear();
    nx_display_cursor_set_pos(0,0);
    nx_display_uint(counter);
  }
}

void main() {
  task_t *task1, *task2, *idle;
  nx_mem_init(NX_USERSPACE_SIZE, NX_USERSPACE_START);
  idle = new_task(NULL);
  task1 = new_task(test_beep);
  task2 = new_task(test_display);
  task1->next = task2;
  task2->next = task1;
  idle->next = idle;
  idle->stack_current += sizeof(nx_task_stack_t);
  idle_task = current_task = idle;
  available_tasks = task1;

  nx_interrupts_disable();
  nx_systick_install_scheduler(systick_cb);
  run_first_task(test_idle, idle->stack_current+1024);
}
