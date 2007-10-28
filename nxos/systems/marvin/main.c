#include "base/tlsf.h"
#include "base/memmap.h"
#include "base/drivers/sound.h"
#include "task.h"

void test_task() {
  sound_freq(440, 1000);
}

void main() {
  U32 *system_stack = NULL;
  init_memory_pool(USERSPACE_SIZE, USERSPACE_START);
  system_stack = rtl_malloc(1024); // 1k stack
  run_first_task(test_task, system_stack + 1024);
}
