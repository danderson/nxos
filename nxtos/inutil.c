#include "sound.h"
#include "systick.h"

enum notes {
  end = 0,
  sleep500 = 2,
  si = 495,
  dod = 561,
  re = 594,
  mi = 660,
  fad = 748,
  sol = 792,
};

static enum notes pain[] = {
  si, sleep500, fad, si, sol, sleep500, fad, mi, fad, sleep500, mi, fad, sol, sol, fad, mi,
  si, sleep500, fad, si, sol, sleep500, fad, mi, re,  sleep500, mi, re,  dod, dod, re,  dod, si,
  end };

void play_pain() {
  int i = 0;

  while (pain[i] != end) {
    if (pain[i] != sleep500)
      //      systick_wait_ms(200);
      //    else
      sound_freq(2*pain[i], 200);
    systick_wait_ms(300);
    i++;
  }
}

void
beep_word(U32 value)
{
  U32 i=32;

  sound_freq(1000, 300);
  systick_wait_ms(500);
  sound_freq(2000, 300);
  systick_wait_ms(2500);

  while (i > 0 && !(value & 0x80000000)) {
    value <<= 1;
    i--;
  }
  while (i > 0) {
    if (value & 0x80000000)
      sound_freq(2000, 300);
    else
      sound_freq(1000, 300);
    systick_wait_ms(1000);
    value <<= 1;
    i--;
  }

  systick_wait_ms(2000);
}
