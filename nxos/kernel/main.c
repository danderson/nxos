
#include "at91sam7s256.h"

#include "mytypes.h"
#include "interrupts.h"
#include "aic.h"
#include "systick.h"
#include "avr.h"
#include "twi.h"
#include "lcd.h"
#include "display.h"
#include "sound.h"
#include "usb.h"
#include "sensors.h"
#include "motors.h"
#include "tlsf.h"
#include "memmap.h"
#include "task.h"

#include "tests.h"

static void core_init() {
  aic_init();
  interrupts_enable();
  systick_init();
  sound_init();
  avr_init();
  motors_init();
  lcd_init();
  display_init();
  sensors_init();
  usb_init();

  /* Delay a little post-init, to let all the drivers settle down. */
  systick_wait_ms(100);
}

static void core_shutdown() {
  lcd_shutdown();
  usb_disable();
  avr_power_down();
}

void test_task() {
  tests_usb();
  core_shutdown();
}

void main() {
  U32 *system_stack = NULL;
  core_init();

  init_memory_pool(USERSPACE_SIZE, USERSPACE_START);
  system_stack = rtl_malloc(1024); // 1k stack
  run_first_task(test_task, system_stack + 1024);

  //tests_usb_hardcore();
  //tests_usb();
  //tests_all();

  //core_shutdown();
}
