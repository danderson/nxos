/* Various test routines for components of the NXT. */

#include "base/types.h"
#include "base/interrupts.h"
#include "base/display.h"
#include "base/memmap.h"
#include "base/drivers/aic.h"
#include "base/drivers/systick.h"
/* TODO: evil, decide if necessary. */
#include "base/drivers/_avr.h"
//#include "base/drivers/twi.h"
//#include "base/drivers/lcd.h"
#include "base/drivers/sound.h"
#include "base/drivers/sensors.h"
#include "base/drivers/motors.h"
#include "base/drivers/usb.h"
#include "base/drivers/radar.h"
#include "base/drivers/bt.h"
#include "base/drivers/_uart.h"

#include "tests.h"

static bool test_silent = FALSE;

static void hello() {
  if (test_silent)
    return;
  nx_sound_freq(1000, 100);
  nx_systick_wait_ms(50);
  nx_sound_freq(2000, 100);
  nx_systick_wait_ms(900);
}

static void goodbye() {
  if (test_silent)
    return;
  nx_sound_freq(2000, 100);
  nx_systick_wait_ms(50);
  nx_sound_freq(1000, 100);
  nx_systick_wait_ms(900);
}

void beep_word(U32 value) {
  U32 i=32;

  hello();

  while (i > 0 && !(value & 0x80000000)) {
    value <<= 1;
    i--;
  }
  while (i > 0) {
    if (value & 0x80000000)
      nx_sound_freq(2000, 300);
    else
      nx_sound_freq(1000, 300);
    nx_systick_wait_ms(700);
    value <<= 1;
    i--;
  }

  goodbye();
}


void tests_display() {
  char buf[2] = { 0, 0 };
  int i;

  hello();

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);

  nx_display_string("- Display test -\n"
		    "----------------\n");
  for (i=32; i<128; i++) {
    buf[0] = i;
    nx_display_string(buf);
    if ((i % 16) == 15)
      nx_display_string("\n");
  }

  nx_systick_wait_ms(5000);
  goodbye();
}


void tests_sound() {
  enum {
    end = 0, sleep500 = 1, si = 990, dod = 1122,
    re = 1188, mi = 1320, fad = 1496, sol = 1584,
  } pain[] = {
    si, sleep500,
    fad, si, sol, sleep500,
    fad, mi, fad, sleep500,
    mi, fad, sol, sol, fad, mi, si, sleep500,
    fad, si, sol, sleep500,
    fad, mi, re,  sleep500,
    mi, re,  dod, dod, re,  dod, si, end
  };
  int i = 0;

  hello();

  nx_display_clear();
  nx_display_cursor_set_pos(0,0);
  nx_display_string("-- Sound test --\n"
		    "----------------\n");

  while (pain[i] != end) {
    if (pain[i] == sleep500)
      nx_systick_wait_ms(150);
    else
      nx_sound_freq(pain[i], 150);
    nx_systick_wait_ms(150);
    i++;
  }

  nx_systick_wait_ms(1000);
  goodbye();
}


void
tests_motor() {
  hello();

  nx_display_clear();
  nx_display_cursor_set_pos(0,0);
  nx_display_string("--- AVR test ---\n"
		    "----------------\n");

  nx__avr_set_motor(0, 80, 0);
  nx_systick_wait_ms(1000);

  nx__avr_set_motor(0, -80, 0);
  nx_systick_wait_ms(1000);

  nx__avr_set_motor(0, 80, 0);
  nx_systick_wait_ms(1000);

  nx__avr_set_motor(0, 0, 1);
  nx_systick_wait_ms(200);

  goodbye();
}


void tests_tachy() {
  int i;
  hello();

  nx_motors_rotate_angle(0, 80, 1024, TRUE);
  nx_motors_rotate_time(1, -80, 3000, FALSE);
  nx_motors_rotate(2, 80);

  for (i=0; i<30; i++) {
    nx_display_clear();
    nx_display_cursor_set_pos(0,0);

    nx_display_clear();
    nx_display_cursor_set_pos(0,0);
    nx_display_string("Tachymeter  test\n"
		      "----------------\n");

    nx_display_string("Tach A: ");
    nx_display_hex(nx_motors_get_tach_count(0));
    nx_display_end_line();

    nx_display_string("Tach B: ");
    nx_display_hex(nx_motors_get_tach_count(1));
    nx_display_end_line();

    nx_display_string("Tach C: ");
    nx_display_hex(nx_motors_get_tach_count(2));
    nx_display_end_line();

    nx_display_string("Refresh: ");
    nx_display_uint(i);
    nx_display_end_line();

    nx_systick_wait_ms(250);
  }

  nx_motors_stop(2, TRUE);

  goodbye();
}


void tests_sensors() {
  U32 i;
  const U32 display_seconds = 15;
  hello();

  nx_sensors_analog_enable(0);

  for (i=0; i<(display_seconds*4); i++) {
    nx_display_clear();
    nx_display_cursor_set_pos(0,0);
    nx_display_string("- Sensor  info -\n"
		      "----------------\n");

    nx_display_string("Port 1: ");
    nx_display_uint(nx_sensors_analog_get(0));
    nx_display_end_line();

    nx_display_string("Port 2: ");
    nx_display_uint(nx_sensors_analog_get(1));
    nx_display_end_line();

    nx_display_string("Port 3: ");
    nx_display_uint(nx_sensors_analog_get(2));
    nx_display_end_line();

    nx_display_string("Port 4: ");
    nx_display_uint(nx_sensors_analog_get(3));
    nx_display_end_line();

    nx_systick_wait_ms(250);
  }

  goodbye();
}


void tests_sysinfo() {
  U32 i;
  U32 t = 0;
  const U32 display_seconds = 15;
  U8 avr_major, avr_minor;
  hello();

  nx_avr_get_version(&avr_major, &avr_minor);

  for (i=0; i<(display_seconds*4); i++) {
    if (i % 4 == 0)
      t = nx_systick_get_ms();

    nx_display_clear();
    nx_display_cursor_set_pos(0,0);
    nx_display_string("- System  info -\n"
		      "----------------\n");

    nx_display_string("Time  : ");
    nx_display_uint(t);
    nx_display_end_line();

    nx_display_string("Boot from ");
    if (NX_BOOT_FROM_SAMBA)
      nx_display_string("SAM-BA");
    else
      nx_display_string("ROM");
    nx_display_end_line();

    nx_display_string("Free RAM: ");
    nx_display_uint(NX_USERSPACE_SIZE);
    nx_display_end_line();

    nx_display_string("Buttons: ");
    nx_display_uint(nx_avr_get_button());
    nx_display_end_line();

    nx_display_string("Battery: ");
    nx_display_uint(nx_avr_get_battery_voltage());
    nx_display_string(" mV");
    nx_display_end_line();

    nx_systick_wait_ms(250);
  }

  goodbye();
}


void tests_bt()
{
  int i;
  bt_device_t *dev;

  nx_bt_init();

  nx_display_clear();
  nx_display_string("Setting friendly name ...");
  nx_display_end_line();

  nx_bt_set_friendly_name("tulipe");

  nx_display_string("Setting as discoverable ...");
  nx_display_end_line();

  nx_bt_set_discoverable(TRUE);

  nx_display_clear();
  nx_display_string("Scanning ...");
  nx_display_end_line();


  nx_bt_begin_inquiry(/* max dev : */ 255,
		      /* timeout : */ 0x20,
		      /* class :   */ (U8[]){ 0, 0, 0, 0 });

  while(nx_bt_get_state() == BT_STATE_INQUIRING) {
    if (nx_bt_has_found_device()) {
      dev = nx_bt_get_discovered_device();
      nx_display_string("# ");
      nx_display_string(dev->name);
      nx_display_end_line();
    }
  }

  for (i = 0 ; i < 10 ; i++)
    {
      nx_display_clear();
      nx_display_uint(nx_uart_nmb_interrupt());
      nx_display_end_line();
      nx_display_hex(nx_uart_get_last_csr());
      nx_display_end_line();
      nx_display_hex(nx_uart_get_csr());
      nx_display_end_line();
      nx_display_hex(nx_uart_get_state());
      nx_display_end_line();
      nx_bt_debug();

      nx_systick_wait_ms(1000);
    }
}



/* returns 1 if they are identic
 * 0 else
 */
static U8 compare_str(char *str_a, char *str_b, U32 max)
{

  while (*str_a != '\0'
	 && *str_b != '\0'
	 && max > 0)
    {
      if (*str_a != *str_b) {
	return 0;
      }
      str_a++;
      str_b++;
      max--;
    }

  if (*str_a != *str_b && max > 0) {
    return 0;
  }

  return 1;
}


#define USB_UNKNOWN    "Unknown"
#define USB_OK         "Ok"
#define USB_OVERLOADED "Ok but overloaded"

void tests_all();

#define MOVE_TIME 1000

void tests_usb() {
  U16 i;
  S32 lng, t;
  char *buffer;

  hello();

  while(1) {
    nx_display_cursor_set_pos(0, 0);
    nx_display_string("Waiting command ...");

    for (i = 0 ; i < 500 && !nx_usb_has_data(); i++)
    {
      nx_systick_wait_ms(200);
    }

    if (i >= 500)
      break;

    nx_display_clear();

    lng = nx_usb_has_data();

    buffer = (char *)nx_usb_get_buffer();
    if ((lng+1) < NX_USB_BUFFER_SIZE)
      buffer[lng+1] = '\0';
    else
      buffer[NX_USB_BUFFER_SIZE-1] = '\0';

    nx_display_cursor_set_pos(0, 0);
    nx_display_string("==");
    nx_display_uint(lng);

    nx_display_cursor_set_pos(0, 1);
    nx_display_string(buffer);

    nx_display_cursor_set_pos(0, 2);
    nx_display_hex((U32)(USB_UNKNOWN));

    /* Start interpreting */

    i = 0;
    if (compare_str(buffer, "motor", lng))
      tests_motor();
    else if (compare_str(buffer, "sound", lng))
      tests_sound();
    else if (compare_str(buffer, "display", lng))
      tests_display();
    else if (compare_str(buffer, "sysinfo", lng))
      tests_sysinfo();
    else if (compare_str(buffer, "sensors", lng))
      tests_sensors();
    else if (compare_str(buffer, "tachy", lng))
      tests_tachy();
    else if (compare_str(buffer, "radar", lng))
      tests_radar();
    else if (compare_str(buffer, "bt", lng))
      tests_bt();
    else if (compare_str(buffer, "all", lng))
      tests_all();
    else if (compare_str(buffer, "halt", lng))
      break;
    else if (compare_str(buffer, "Al", lng))
      nx_motors_rotate_angle(0, 70, 100, 1);
    else if (compare_str(buffer, "Ar", lng))
      nx_motors_rotate_angle(0, -70, 100, 1);
    else if (compare_str(buffer, "Ac", lng)) {
      nx_motors_rotate(0, 75);
      while((t = nx_motors_get_tach_count(0)) != 0) {
	if (t < 0) {
	  nx_motors_rotate(0, 75);
	} else {
	  nx_motors_rotate(0, -75);
	}
	nx_display_cursor_set_pos(1, 1);
	nx_display_hex(t);
	nx_display_string("          ");
      }
      nx_motors_stop(0, 1);
    } else if (compare_str(buffer, "BCf", lng)) {
      nx_motors_rotate(1, -100);
      nx_motors_rotate(2, -100);
      nx_systick_wait_ms(MOVE_TIME);
      nx_motors_stop(1, 0);
      nx_motors_stop(2, 0);
    } else if (compare_str(buffer, "BCr", lng)) {
      nx_motors_rotate(1, 80);
      nx_motors_rotate(2, 80);
      nx_systick_wait_ms(MOVE_TIME);
      nx_motors_stop(1, 0);
      nx_motors_stop(2, 0);
    }
    else {
      i = 1;
      nx_usb_send((U8 *)USB_UNKNOWN, sizeof(USB_UNKNOWN)-1);
    }

    if (i == 0) {
      if (!nx_usb_overloaded())
	nx_usb_send((U8 *)USB_OK, sizeof(USB_OK)-1);
      else
	nx_usb_send((U8 *)USB_OVERLOADED, sizeof(USB_OVERLOADED)-1);
    }

    /* Stop interpreting */

    nx_systick_wait_ms(500);

    nx_display_clear();

    nx_usb_flush_buffer();

  }

  goodbye();
}

void tests_usb_hardcore() {
  int i, lng;

  char *buffer;

  hello();

  nx_systick_wait_ms(6000);

  for (i = 0 ; i < 1800 ; i++) {

    if ( (lng = nx_usb_has_data()) > 0) {
      buffer = (char *)nx_usb_get_buffer();
      if (compare_str(buffer, "halt", lng)) {
	break;
      }
      nx_usb_flush_buffer();
    }


    nx_usb_send((U8 *)"TEST", 5);
  }

  goodbye();
}

void tests_radar() {
  U8 interval, reading;
  S8 object;

  hello();

  nx_radar_init(0);

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);
  nx_display_string("Discovering...\n");

  while (!nx_radar_detect(0)) {
    nx_display_string("Error! Retrying\n");
    nx_systick_wait_ms(500);

    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);
    nx_display_string("Discovering...\n");
  }

  nx_display_string("Found.\n\n");
  nx_radar_info(0);
  interval = nx_radar_get_interval(0);

  while (nx_avr_get_button() != BUTTON_OK);

  while (nx_avr_get_button() != BUTTON_RIGHT) {
    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);

    for (object=0 ; object<8 ; object++) {
      nx_display_uint(object);
      nx_display_string("> ");

      reading = nx_radar_read_distance(0, object);

      if (reading > 0x00 && reading < 0xFF) {
        nx_display_uint(reading);
        nx_display_string(" cm\n");
      } else {
        nx_display_string("n/a\n");
      }
    }

    nx_systick_wait_ms(interval*500);
  }

  goodbye();
}

void tests_all() {
  test_silent = TRUE;

  tests_display();
  tests_sound();
  tests_motor();
  tests_tachy();
  tests_sensors();
  tests_sysinfo();
  tests_radar();

  test_silent = FALSE;
  goodbye();
}
