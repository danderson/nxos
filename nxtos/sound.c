#include "at91sam7s256.h"
#include "mytypes.h"
#include "aic.h"
#include "sound.h"

const U32 tone_pattern[16] =
  {
    0xF0F0F0F0,0xF0F0F0F0,
    0xFCFCFCFC,0xFCFCFDFD,
    0xFFFFFFFF,0xFFFFFFFF,
    0xFDFDFCFC,0xFCFCFCFC,
    0xF0F0F0F0,0xF0F0F0F0,
    0xC0C0C0C0,0xC0C08080,
    0x00000000,0x00000000,
    0x8080C0C0,0xC0C0C0C0
  };

U32 tone_cycles;

static void
sound_interrupt_enable()
{
  *AT91C_SSC_IER = AT91C_SSC_ENDTX;
}

static void
sound_interrupt_disable()
{
  *AT91C_SSC_IDR = AT91C_SSC_ENDTX;
}

static void
sound_enable()
{
  *AT91C_PIOA_PDR = AT91C_PA17_TD;
}


static void
sound_disable()
{
  *AT91C_PIOA_PER = AT91C_PA17_TD;
}


static void
sound_isr()
{
  if (tone_cycles--)
  {
    *AT91C_SSC_TNPR = (unsigned int) tone_pattern;
    *AT91C_SSC_TNCR = 16;
    sound_enable();
  }
  else
  {
  	sound_disable();
  	sound_interrupt_disable();
  }
}


void
sound_init()
{
  sound_interrupt_disable();
  sound_disable();

  *AT91C_PMC_PCER = (1 << AT91C_ID_SSC);

  *AT91C_SSC_CR = AT91C_SSC_SWRST;
  *AT91C_SSC_TCMR = AT91C_SSC_CKS_DIV + AT91C_SSC_CKO_CONTINOUS + AT91C_SSC_START_CONTINOUS;
  *AT91C_SSC_TFMR = 31 + (7 << 8) + AT91C_SSC_MSBF; // 8 32-bit words
  *AT91C_SSC_CR = AT91C_SSC_TXEN;

  aic_install_isr(AT91C_ID_SSC, AT91C_AIC_PRIOR_LOWEST, sound_isr);
}


void sound_freq(U32 freq, U32 ms)
{
  *AT91C_SSC_CMR = ((96109714 / 1024) / freq) + 1;
  *AT91C_SSC_PTCR = AT91C_PDC_TXTEN;
  tone_cycles = (freq * ms) / 2000 - 1;
  sound_interrupt_enable();
}
