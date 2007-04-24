#include "at91sam7s256.h"
#include "interrupts.h"
#include "aic.h"
#include "avr.h"
#include "lcd.h"
#include "systick.h"


#define CLOCK_FREQUENCY 48000000
#define PIT_FREQ 1000		/* Hz */

#define LOW_PRIORITY_IRQ 10

static volatile U32 systick_sec;
static volatile U32 systick_sub_sec;
static volatile U32 systick_ms;

// Systick low priority
static void
systick_low_priority()
{
  aic_clear(LOW_PRIORITY_IRQ);
  avr_1kHz_update();
  lcd_1kHz_update();
}

// Called at 1000Hz
static void
systick_isr()
{
  U32 status;

  /* Read status to confirm interrupt */
  status = *AT91C_PITC_PIVR;

//  systick_low_priority_C();

  systick_ms++;

  systick_sub_sec++;

  if (systick_sub_sec >= PIT_FREQ) {
    systick_sub_sec = 0;
    systick_sec++;
  }
  // Trigger low priority task
  aic_set(LOW_PRIORITY_IRQ);
}



U32
systick_get_ms()
{
  // We're using a 32-bitter and can assume that we
  // don't need to do any locking here.
  return systick_ms;
}


void
systick_wait_ms(U32 ms)
{
  volatile U32 final = ms + systick_ms;

  while (systick_ms < final) {
  }
}


void
systick_wait_ns(U32 ns)
{
  volatile int x = (ns >> 7) + 1;

  while (x) {
    x--;
  }
}

void
systick_init()
{
  interrupts_disable();

  aic_install_isr(LOW_PRIORITY_IRQ, AIC_INT_LEVEL_LOW, systick_low_priority);
  aic_install_isr(AT91C_ID_SYS, AIC_INT_LEVEL_NORMAL, systick_isr);

  *AT91C_PITC_PIMR = ((CLOCK_FREQUENCY / 16 / PIT_FREQ) - 1) | 0x03000000;

  interrupts_enable();
}

void
systick_get_time(U32 *sec, U32 *usec)
{
  interrupts_disable();

  if (sec)
    *sec = systick_sec;
  if (usec)
    *usec = systick_sub_sec * (1000000 / PIT_FREQ);

  interrupts_enable();
}
