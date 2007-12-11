/** Exploring bot.
 *
 * The NXT brick/rover will go forward until it detects an
 * object in front of it and make a right turn before continuing.
 * The bot will keep this behavior for RUNTIME seconds.
 */

#include "base/types.h"
#include "base/core.h"
#include "base/display.h"
#include "base/dump.h"
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/drivers/sound.h"
#include "base/drivers/sensors.h"
#include "base/drivers/motors.h"
#include "base/drivers/radar.h"

/** Demo runtime in seconds. */
#define RUNTIME 60
#define SIMU_TICK 250

/** Sensors/motors ports definitions. */
#define RADAR 0
#define M_A 0
#define M_B 2

#define DETECT_DISTANCE 40
#define SPEED -100
#define BRAKE_WAIT 500
#define TURN_WAIT 750

#define BEEP(x) nx_sound_freq_async(x, 200)

typedef enum {
  ST_STOP = 0,
  ST_DETECT,
  ST_FORWARD,
  ST_TURN,
  ST_TURN1,
} simu_state;

static simu_state state;
static U32 start_tick;

/** Security hook. A press on concel will inconditionally halt the brick.
 */
static void watchdog(void) {
 if (nx_avr_get_button() == BUTTON_CANCEL)
    nx_core_halt();
}

static void init(void) {
  nx_systick_install_scheduler(watchdog);
  nx_radar_init(RADAR);

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);
  nx_display_string("Discovering...\n");

  while (!nx_radar_detect(RADAR)) {
    nx_display_string("Error! Retrying\n");
    nx_systick_wait_ms(500);

    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);
    nx_display_string("Discovering...\n");
  }

  nx_display_string("Found.\n\n");
  nx_radar_info(RADAR);

  nx_display_string("Press OK to start\n");

  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);
}

static void start(void) {
  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);

  start_tick = nx_systick_get_ms();
  state = ST_FORWARD;
}

static void live(void) {
	static U8 turn = 0;
  U32 obj;
  bool detect = FALSE;

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);

  switch (state) {
    case ST_STOP:
      nx_motors_stop(M_A, TRUE);
      nx_motors_stop(M_B, TRUE);
      break;

    case ST_FORWARD:
      nx_display_string("Forward...\n");
      nx_motors_rotate(M_A, SPEED);
      nx_motors_rotate(M_B, SPEED);

      state = ST_DETECT;
      break;

    case ST_DETECT:      
      for (obj = 0 ; obj < 8 ; obj++) {
        U8 reading = nx_radar_read_distance(RADAR, obj);
        nx_display_uint(obj);
        nx_display_string("> ");
        nx_display_uint(reading);
        nx_display_string(" cm\n");

        if (reading > 0 && reading <= DETECT_DISTANCE)
          detect = TRUE;
      }

      if (detect) {
        BEEP(2000);
        state = ST_TURN;
      }
      break;

    case ST_TURN:
      nx_display_string("Braking...\n");

      nx_motors_rotate(M_A, -SPEED);
      nx_motors_rotate(M_B, -SPEED);
      nx_systick_wait_ms(BRAKE_WAIT);

      state = ST_TURN1;
      break;

    case ST_TURN1:
      nx_display_string("Turning...\n");

			if (turn == 0) {
      	nx_motors_rotate(M_A, SPEED);
      	nx_motors_rotate(M_B,-SPEED);
				turn = 1;
			} else {
				nx_motors_rotate(M_A,-SPEED);
      	nx_motors_rotate(M_B, SPEED);
				turn = 0;
			}
			
      nx_systick_wait_ms(TURN_WAIT);

      state = ST_FORWARD;
      break;

    default:
      state = ST_STOP;
  }
}

static void die(void) {
  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);
  nx_display_string("Going down...\n");

  nx_motors_stop(M_A, TRUE);
  nx_motors_stop(M_B, TRUE);

  nx_systick_wait_ms(1000);
  nx_display_string("Motors stopped.\n");
  
  nx_display_string("Bye!\n");
  nx_systick_wait_ms(1000);
}

void main(void) {
  init();
  start();
  
  while (nx_systick_get_ms() < start_tick + RUNTIME * 1000) {
    live();
    nx_systick_wait_ms(SIMU_TICK);
  }
  
  die();  
}

