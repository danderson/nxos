/** Exploring bot.
 *
 * The NXT brick/rover will go forward until it detects an
 * object in front of it and make a right turn before continuing.
 * The bot will keep this behavior for RUNTIME seconds.
 */

#include "base/types.h"
#include "base/display.h"
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/drivers/sound.h"
#include "base/drivers/sensors.h"
#include "base/drivers/motors.h"
#include "base/drivers/radar.h"

/** Demo runtime in seconds. */
#define RUNTIME 60

/** Sensors/motors ports definitions. */
#define RADAR 0
#define M_A 0
#define M_B 2

#define DETECT_DISTANCE 60
#define SPEED 100

#define BEEP(x) nx_sound_freq_async(x, 100)

void main() {
  U32 start;

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);

  nx_radar_init(RADAR);

  nx_display_string("++ Exploration ++\n");
  nx_display_string("Starts in 3 secs.\n");
  nx_systick_wait_ms(3000);
  nx_display_string("Go!\n");

  start = nx_systick_get_ms();
  
  while (nx_systick_get_ms() < start + RUNTIME * 1000
    && nx_avr_get_button() != BUTTON_OK) {
    U8 reading = nx_radar_read_distance(RADAR, 0);
    
    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);
    
    nx_display_string("radar> ");
    nx_display_uint(reading);
    nx_display_string(" cm\n");
    
    /* For now, stop when detecting an object close to us */
    if (reading < DETECT_DISTANCE) {
      nx_motors_stop(M_A, FALSE);
      nx_motors_stop(M_B, FALSE);
    } else {
      nx_motors_rotate(M_A, SPEED);
      nx_motors_rotate(M_B, SPEED);
    }

    nx_systick_wait_ms(500);
  }
  
  nx_display_string("Done.\n");
}

