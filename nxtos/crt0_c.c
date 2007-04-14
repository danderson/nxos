/* NXT bootup code */

#include "at91sam7s256.h"

extern void nxt_default_irq_handler(void);

/* Boot the board's main oscillator and step the CPU up to 48MHz. */
static inline void init_clocks()
{
  /* Enable the internal main oscillator.
   *
   * The oscillator must be given a quartz-dependent time to start
   * up. For the NXT, this must be set to 6 cycles of the slow clock,
   * which at this stage controls the board.
   */
  *AT91C_CKGR_MOR = AT91C_CKGR_MOSCEN | (6 << 8);

  /* Wait for the oscillator to stabilize. */
  while ((*AT91C_PMC_SR & AT91C_PMC_MOSCS) == 0);

  /* Initialize the PLL clock.
   *
   * This clock will later provide the basis for the main board
   * clock. The quartz on the board runs at 18.432MHz, and we want a
   * main clock running at 48MHz. The best we can do is to set the PLL
   * to run at 96MHz, and then clock back down when configuring the
   * main clock.
   *
   * To do so, we divide the input signal by 14, then multiply the
   * output by 73 (72+1, as the datasheet says that the board adds 1
   * to the value written in the register), which clocks us to
   * 96.11MHz. Note that the USB clock requires an input signal of
   * 96MHz +/- 0.25%. A frequency of 96.11MHz is a deviation of .11%,
   * therefore acceptable.
   *
   * The PLL clock is estimated to lock within 0.844ms (estimate
   * documented in the LEGO source code), which maps to ~28 slow clock
   * cycles.
   */
  *AT91C_CKGR_PLLR = (14 | (28 << 8) | (72 << 16));

  /* Wait for the PLL to lock. */
  while ((*AT91C_PMC_SR & AT91C_PMC_LOCK) == 0);

  /* Wait for master clock ready before fiddling with it, just as a
     precaution. It should be running fine at this stage. */
  while ((*AT91C_PMC_SR & AT91C_PMC_MCKRDY) == 0);

  /* Set the master clock prescaler to divide by two, which sets the
   * master clock to 16KHz (half the slow clock frequency).
   *
   * Note that we stay on the slow clock, because of the procedure
   * explained in the specification: you must first set the prescaler,
   * wait for the newly scaled clock to stabilize, then switch to a
   * new clock source in a separate operation.
   */
  *AT91C_PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK | AT91C_PMC_PRES_CLK_2;

  /* Wait for the master clock to stabilize. */
  while ((*AT91C_PMC_SR & AT91C_PMC_MCKRDY) == 0);

  /* Switch the master clock source over to the PLL clock. */
  *AT91C_PMC_MCKR = AT91C_PMC_CSS_PLL_CLK | AT91C_PMC_PRES_CLK_2;

  /* Wait for the master clock to stabilize at 48MHz. */
  while ((*AT91C_PMC_SR & AT91C_PMC_MCKRDY) == 0);

  /* TODO enable USB clocks */
}


/*
 * Initialize the Advanced Interrupt Controller with stub handlers and
 * all interrupts disabled.
 */
static inline void init_aic() {
  int i;

  /* Do some cleanup on the AIC. All these are to protect against the
   * case where we are coming from a warm boot. These values define
   * the modes in which the AIC should be booting up.
   *
   *  - Disable all peripheral interrupt lines.
   *  - Turn off Fast Forcing for all peripheral interrupt lines.
   *  - Clear all pending interrupts.
   *  - Tell the AIC that it is no longer handling any interrupt.
   */
  *AT91C_AIC_IDCR  = 0xFFFFFFFF;
  *AT91C_AIC_FFDR  = 0xFFFFFFFF;
  *AT91C_AIC_ICCR  = 0xFFFFFFFF;
  *AT91C_AIC_EOICR = 0xFFFFFFFF;

  /* Since it can be useful for JTAG debugging, enable debug mode in
   * the AIC. In this mode, reading the interrupt vector register will
   * not unlock the AIC and let it trigger any higher priority
   * IRQs. We will have to write back to the vector register to unlock
   * the AIC (the low-level IRQ dispatcher routine does this already).
   */
  *AT91C_AIC_DCR = 0x1;

  /* Set up the AIC with the default handlers. The handlers at startup
   * are undefined, which will cause undefined behavior if the kernel
   * activates an interrupt line before configuring the handler.
   */
  for (i=0; i<31; i++) {
    AT91C_AIC_SVR[i] = (long int) nxt_default_irq_handler;
  }
  *AT91C_AIC_SPU = (long int) nxt_default_irq_handler;
}


/* Routine called by the low level bootstrapper assembly code to
 * perform basic board initialization.
 */
void nxt_low_level_init() {
  /* Configure the Flash controller with write speed settings. These
   * settings are valid for writing everywhere but the non-volatile bits
   * (lock, security, general-purpose NVM).
   *
   * These values are valid only after the master clock is configured
   * to run at 48MHz, ie. after the call to init_clocks. Do NOT write
   * to flash before then!
   */
  *AT91C_MC_FMR = AT91C_MC_FWS_1FWS | (0x48 << 16);

  /* Disable the watchdog timer for now. The kernel can reenable it
   * later if it feels like it.
   */
  *AT91C_WDTC_WDMR = AT91C_WDTC_WDDIS;

  /* Start the board clocks and step up to 48MHz. */
  init_clocks();

  /* Initialize the Advanced Interrupt Controller. */
  init_aic();
}
