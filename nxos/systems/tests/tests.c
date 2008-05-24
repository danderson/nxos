/* Various test routines for components of the NXT. */

#include "base/types.h"
#include "base/interrupts.h"
#include "base/display.h"
#include "base/memmap.h"
#include "base/util.h"
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

#include "tests/tests.h"

static bool test_silent = FALSE;

static void hello(void) {
  if (test_silent)
    return;
  nx_sound_freq(1000, 100);
  nx_systick_wait_ms(50);
  nx_sound_freq(2000, 100);
  nx_systick_wait_ms(900);
}

static void goodbye(void) {
  if (test_silent)
    return;
  nx_sound_freq(2000, 100);
  nx_systick_wait_ms(50);
  nx_sound_freq(1000, 100);
  nx_systick_wait_ms(900);
}

void tests_display(void) {
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


void tests_sound(void) {
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


void tests_motor(void) {
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


void tests_tachy(void) {
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


void tests_sensors(void) {
  U32 i, sensor;
  const U32 display_seconds = 15;
  hello();

  for (sensor=0; sensor<NXT_N_SENSORS; sensor++) {
    nx_sensors_analog_enable(sensor);
  }

  for (i=0; i<(display_seconds*4); i++) {
    nx_display_clear();
    nx_display_cursor_set_pos(0,0);
    nx_display_string("- Sensor  info -\n"
		      "----------------\n");

    for (sensor=0; sensor<NXT_N_SENSORS; sensor++){
      nx_display_string("Port ");
      nx_display_uint(sensor);
      nx_display_string(": ");


      nx_display_uint(nx_sensors_analog_get(sensor));
      nx_display_end_line();
    }

    nx_systick_wait_ms(250);
  }

  for (sensor=0; sensor<NXT_N_SENSORS; sensor++) {
    nx_sensors_analog_disable(sensor);
  }

  goodbye();
}


void tests_sysinfo(void) {
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



static void tests_bt_list_known_devices(void) {
  bt_device_t dev;

  /* Listing known devices */

  nx_display_clear();
  nx_display_string("Known devices: ");
  nx_display_end_line();

  nx_bt_begin_known_devices_dumping();
  while(nx_bt_get_state() == BT_STATE_KNOWN_DEVICES_DUMPING) {
    if (nx_bt_has_known_device()) {
      nx_bt_get_known_device(&dev);
      nx_display_string("# ");
      nx_display_string(dev.name);
      nx_display_end_line();
    }
  }

  nx_systick_wait_ms(2000);
}


static void tests_bt_scan_and_add(void) {
  bt_device_t dev;

  nx_display_clear();
  nx_display_string("Scanning and adding ...");
  nx_display_end_line();


  nx_bt_begin_inquiry(/* max dev : */ 255,
		      /* timeout : */ 0x20,
		      /* class :   */ (U8[]){ 0, 0, 0, 0 });

  while(nx_bt_get_state() == BT_STATE_INQUIRING) {
    if (nx_bt_has_found_device()) {
      nx_bt_get_discovered_device(&dev);
      nx_display_string("# ");
      nx_display_string(dev.name);
      nx_display_end_line();

      nx_bt_add_known_device(&dev);
    }
  }

}


static void tests_bt_scan_and_remove(void) {
  bt_device_t dev;

  nx_display_clear();
  nx_display_string("Scanning and removing ...");
  nx_display_end_line();


  nx_bt_begin_inquiry(/* max dev : */ 255,
		      /* timeout : */ 0x20,
		      /* class :   */ (U8[]){ 0, 0, 0, 0 });

  while(nx_bt_get_state() == BT_STATE_INQUIRING) {
    if (nx_bt_has_found_device()) {
      nx_bt_get_discovered_device(&dev);
      nx_display_string("# ");
      nx_display_string(dev.name);
      nx_display_end_line();

      nx_bt_remove_known_device(dev.addr);
    }
  }

}

void tests_bt2(void)
{
  /*int i;
   */

  hello();

  /* Configuring the BT */

  nx_bt_init();

  nx_display_clear();
  nx_display_string("Setting friendly name ...");
  nx_display_end_line();

  nx_bt_set_friendly_name("tulipe");

  nx_display_string("Setting as discoverable ...");
  nx_display_end_line();

  nx_bt_set_discoverable(TRUE);

  /* Listing known devices */

  tests_bt_list_known_devices();

  /* Scanning & adding */

  tests_bt_scan_and_add();

  /* Listing known devices */

  tests_bt_list_known_devices();


  /* Scanning & removing */

  tests_bt_scan_and_remove();


  /* Listing known devices */

  tests_bt_list_known_devices();


  /*
  for (i = 0 ; i < 10 ; i++)
    {
      nx_display_clear();
      nx_bt_debug();

      nx_systick_wait_ms(1000);
    }
  */

  goodbye();
}



/* returns 1 if they are identic
 * 0 else
 *
 * TODO: use base/util.h:strncmp instead.
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


#define CMD_UNKNOWN    "Unknown"
#define CMD_OK         "Ok"

void tests_all();

#define MOVE_TIME_AV 1000
#define MOVE_TIME_AR 3000

/**
 * @return 0 if success ; 1 if unknown command ; 2 if halt
 */
static int tests_command(char *buffer, int lng)
{
  int i;
  S32 t;

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
  else if (compare_str(buffer, "bt2", lng))
    tests_bt2();
  else if (compare_str(buffer, "all", lng))
    tests_all();
  else if (compare_str(buffer, "halt", lng))
    return 2;
  else if (compare_str(buffer, "Al", lng))
    nx_motors_rotate_angle(0, 90, 100, 1);
  else if (compare_str(buffer, "Ar", lng))
    nx_motors_rotate_angle(0, -90, 100, 1);
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
    nx_systick_wait_ms(MOVE_TIME_AV);
    nx_motors_stop(1, 0);
    nx_motors_stop(2, 0);
  } else if (compare_str(buffer, "BCr", lng)) {
    nx_motors_rotate(1, 80);
    nx_motors_rotate(2, 80);
    nx_systick_wait_ms(MOVE_TIME_AR);
    nx_motors_stop(1, 0);
    nx_motors_stop(2, 0);
  }
  else {
    i = 1;
  }

  return i;
}

#define BT_PACKET_SIZE 128

void tests_bt(void) {
  U16 i;
  U16 lng = 0;
  int port_handle = -1;
  int connection_handle = -1;
  int bleh = -1;

  char buffer[BT_PACKET_SIZE];

  for (i = 0 ; i < BT_PACKET_SIZE ; i++)
    buffer[i] = 0;
  lng = 0;

  nx_bt_init();

  hello();

  nx_display_cursor_set_pos(0, 0);
  nx_display_string("Setting discoverable ...");
  nx_bt_set_friendly_name("Tulipe");
  nx_bt_set_discoverable(TRUE);

  port_handle = nx_bt_open_port();

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);
  nx_display_string("Waiting connection ...");

  while(TRUE) {

    for (i = 0 ;
         i < 800 && (!nx_bt_stream_opened() || nx_bt_stream_data_read() < 2);
         i++)
      {
        if (connection_handle >= 0) {

          nx_display_cursor_set_pos(0, 2);
          nx_display_string("Waiting command ...");

        }

        if (nx_bt_has_dev_waiting_for_pin()) {

          nx_bt_send_pin("1234");

        } else if (nx_bt_connection_pending()) {

          nx_bt_accept_connection(TRUE);

          while ( (bleh = nx_bt_connection_established()) < 0)
            nx_systick_wait_ms(100);

          connection_handle = bleh;

          nx_bt_debug();

          nx_bt_stream_open(connection_handle);

          nx_bt_debug();

          nx_bt_stream_read((U8 *)&buffer, 2); /* we read the packet size first */

          nx_display_cursor_set_pos(0, 4);

        }

        nx_bt_debug();
        nx_systick_wait_ms(100);
      }

    if (!nx_bt_stream_opened() || i >= 800)
      break;

    lng = buffer[0] + (buffer[1] << 8);

    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);
    nx_display_string("Reading ...");
    nx_display_cursor_set_pos(0, 1);
    nx_display_uint(lng);

    nx_bt_stream_read((U8 *)&buffer, lng);

    for(i = 0;
        nx_bt_stream_opened() && nx_bt_stream_data_read() < lng && i < 100;
        i++)
      nx_systick_wait_ms(100);

    if (!nx_bt_stream_opened() || i >= 100)
      break;

    /* Start interpreting */

    i = tests_command(buffer, lng);

    nx_bt_stream_read((U8 *)&buffer, 2);

    if (i == 2) {
      break;
    }

    /*
    if (i == 1) {
      nx_bt_stream_write((U8 *)CMD_UNKNOWN, sizeof(CMD_UNKNOWN)-1);
    }

    if (i == 0) {
      nx_bt_stream_write((U8 *)CMD_OK, sizeof(CMD_OK)-1);
    }
    */

    nx_systick_wait_ms(100);
    nx_display_clear();

  }

  if (nx_bt_stream_opened())
    nx_bt_stream_close();

  nx_systick_wait_ms(1000);

  nx_bt_debug();

  goodbye();
}

void tests_usb(void) {
  U16 i;
  U32 lng = 0;

  char buffer[NX_USB_PACKET_SIZE];

  hello();

  while(1) {
    nx_display_cursor_set_pos(0, 0);
    nx_display_string("Waiting command ...");

    nx_usb_read((U8 *)&buffer, NX_USB_PACKET_SIZE * sizeof(char));

    for (i = 0 ; i < 500 && !nx_usb_data_read(); i++)
    {
      nx_systick_wait_ms(200);
    }

    if (i >= 500)
      break;

    nx_display_clear();

    lng = nx_usb_data_read();

    if ((lng+1) < NX_USB_PACKET_SIZE)
      buffer[lng+1] = '\0';
    else
      buffer[NX_USB_PACKET_SIZE-1] = '\0';

    nx_display_cursor_set_pos(0, 0);
    nx_display_string("==");
    nx_display_uint(lng);

    nx_display_cursor_set_pos(0, 1);
    nx_display_string(buffer);

    nx_display_cursor_set_pos(0, 2);
    nx_display_hex((U32)(CMD_UNKNOWN));

    /* Start interpreting */

    i = tests_command(buffer, lng);

    if (i == 2) {
      break;
    }

    if (i == 1) {
      nx_usb_write((U8 *)CMD_UNKNOWN, sizeof(CMD_UNKNOWN)-1);
    }

    if (i == 0) {
      nx_usb_write((U8 *)CMD_OK, sizeof(CMD_OK)-1);
    }

    nx_systick_wait_ms(500);

    nx_display_clear();

  }

  goodbye();
}

void tests_usb_hardcore(void) {
  int i, lng;

  char buffer[NX_USB_PACKET_SIZE];

  hello();

  nx_systick_wait_ms(6000);

  nx_usb_read((U8 *)(&buffer), NX_USB_PACKET_SIZE);

  for (i = 0 ; i < 1800 ; i++) {

    if ( (lng = nx_usb_data_read()) > 0) {
      if (compare_str(buffer, "halt", lng)) {
	break;
      }
      nx_usb_read((U8 *)(&buffer), NX_USB_PACKET_SIZE);
    }

    nx_usb_write((U8 *)"TEST", 5);
  }

  goodbye();
}

void tests_radar(void) {
  U32 sensor = 0;
  U8 interval, reading;
  S8 object;

  hello();

  nx_display_clear();
  nx_display_cursor_set_pos(0, 0);
  nx_radar_init(sensor);

  nx_display_string("Discovering...\n");

  while (!nx_radar_detect(sensor)) {
    nx_display_string("Error! Retrying\n");
    nx_systick_wait_ms(500);

    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);
    nx_display_string("Discovering...\n");
  }

  nx_display_string("Found.\n\n");
  nx_radar_info(sensor);
  interval = nx_radar_read_value(sensor, RADAR_INTERVAL);

  while (nx_avr_get_button() != BUTTON_OK);

  while (nx_avr_get_button() != BUTTON_RIGHT) {
    nx_display_clear();
    nx_display_cursor_set_pos(0, 0);

    for (object=0 ; object<8 ; object++) {
      nx_display_uint(object);
      nx_display_string("> ");

      reading = nx_radar_read_distance(sensor, object);

      if (reading > 0x00 && reading < 0xFF) {
        nx_display_uint(reading);
        nx_display_string(" cm\n");
      } else {
        nx_display_string("n/a\n");
      }
    }

    nx_systick_wait_ms((interval > 0) ? interval * 500 : 1000);
  }

  nx_radar_close(sensor);
  goodbye();
}

void tests_all(void) {
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
