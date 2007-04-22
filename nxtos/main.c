
#include "mytypes.h"
#include "interrupts.h"
#include "aic.h"
#include "at91sam7s256.h"
#include "systick.h"
#include "avr.h"
#include "twi.h"

#include "sound.h"

#include "inutil.h"

void
core_init() {
  aic_initialise();
  interrupts_enable();
  systick_init();
  sound_init();
  avr_init();
}

void
main()
{
  core_init();

  avr_set_motor(0, 80, 0);
  sound_freq_async(440, 1000);
  systick_wait_ms(1000);

  avr_set_motor(0, -80, 0);
  sound_freq_async(880, 1000);
  systick_wait_ms(1000);

  avr_set_motor(0, 80, 0);
  sound_freq_async(1320, 1000);
  systick_wait_ms(1000);

  avr_set_motor(0, 0, 1);

  sound_freq(2000, 100);
  systick_wait_ms(50);
  sound_freq(2000, 100);
  systick_wait_ms(50);

  avr_power_down();
}
