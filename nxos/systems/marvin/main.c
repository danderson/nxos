#include "base/tests.h"
#include "base/tlsf.h"
#include "base/memmap.h"
#include "task.h"

void test_task() {
  tests_usb();
}

void main() {
  U32 *system_stack = NULL;
  tests_usb();
  return;
  init_memory_pool(USERSPACE_SIZE, USERSPACE_START);
  system_stack = rtl_malloc(1024); // 1k stack
  run_first_task(test_task, system_stack + 1024);
}
