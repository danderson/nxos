/* Electric storm main code.
 *
 * This gets called as the payload of the crt0.
 */

#include "at91sam7s256.h"
#include "crt0.h"

#include "sys_timer.h"
#include "twi.h"
#include "system.h"

/*
 * This is the first function to get executed after the bootstrapper
 * does the bare metal board initialization. We arrive in this routine
 * with interrupts disabled, so we're free to do all the setup we
 * like.
 */
void kernel_main(void) {
  system_services_init();
  sys_timer_init();
  twi_init();
  interrupts_enable();
}
