/* Driver for the NXT's LCD display.
 *
 * This driver contains a basic SPI driver to talk to the UltraChip
 * 1601 LCD controller, as well as a higher level API implementing the
 * UC1601's commandset.
 *
 * Note that the SPI driver is not suitable as a general-purpose SPI
 * driver: the MISO pin (Master-In Slave-Out) is instead wired to the
 * UC1601's CD input (used to select whether the transferred data is
 * control commands or display data). Thus, the SPI driver here takes
 * manual control of the MISO pin, and drives it depending on the type
 * of data being transferred.
 *
 * This also means that you can only write to the UC1601, not read
 * back from it. This is not too much of a problem, as we can just
 * introduce a little delay in the places where we really need it.
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "interrupts.h"
#include "systick.h"
#include "aic.h"

/*
 * SPI controller driver.
 */
static void spi_init() {
  interrupts_disable();

  /* Enable power to the SPI and PIO controllers. */
  *AT91C_PMC_PCER = (1 << AT91C_ID_SPI) | (1 << AT91C_ID_PIOA);

  /* Configure the PIO controller: Hand the MOSI (Master Out, Slave
   * In) and SPI clock pins over to the SPI controller, but keep MISO
   * (Master In, Slave Out) and PA10 (Chip Select in this case) and
   * configure them for output.
   */
  *AT91C_PIOA_PDR = AT91C_PA13_MOSI | AT91C_PA14_SPCK;
  *AT91C_PIOA_ASR = AT91C_PA13_MOSI | AT91C_PA14_SPCK;

  *AT91C_PIOA_PER = AT91C_PA12_MISO | AT91C_PA10_NPCS2;
  *AT91C_PIOA_OER = AT91C_PA12_MISO | AT91C_PA10_NPCS2;
  *AT91C_PIOA_SODR = AT91C_PA12_MISO | AT91C_PA10_NPCS2;

  /* Disable all SPI interrupts, then configure the SPI controller in
   * master mode, with the chip select locked to chip 0 (UC1601 LCD
   * controller), communication at 2MHz, 8 bits per transfer and an
   * inactive-high clock signal.
   */
  *AT91C_SPI_CR = AT91C_SPI_SPIEN;
  *AT91C_SPI_IDR = ~0;
  *AT91C_SPI_MR = (6 << 24) | AT91C_SPI_MSTR;
  AT91C_SPI_CSR[0] = ((0x18 << 24) | (0x18 << 16) | (0x18 << 8) |
                      AT91C_SPI_BITS_8 | AT91C_SPI_CPOL);

  interrupts_enable();
}

/*
 * Send a command byte to the LCD controller.
 */
static void spi_write_command(U8 command) {
  /* Flip the chip-select line... Because the uc1601 has a bug and
     needs to be pinged a little more directly? */
  *AT91C_PIOA_SODR = AT91C_PA10_NPCS2;
  *AT91C_PIOA_CODR = AT91C_PA10_NPCS2;

  /* Sending a command, clear the CD line. */
  *AT91C_PIOA_CODR = AT91C_PA12_MISO;

  /* Send the command byte and wait for a reply. */
  *AT91C_SPI_TDR = command;

  /* Wait for the transmission to complete. */
  while (!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY));

  /* Deselect the chip manually again. */
  *AT91C_PIOA_SODR = AT91C_PA10_NPCS2;
}

/*
 * Send data bytes to the LCD controller.
 */
void spi_write_data(U8 *data, U32 len) {
  /* Flip the chip-select line... Because the uc1601 has a bug and
     needs to be pinged a little more directly? */
  *AT91C_PIOA_SODR = AT91C_PA10_NPCS2;
  *AT91C_PIOA_CODR = AT91C_PA10_NPCS2;

  /* Sending data, set the CD line. */
  *AT91C_PIOA_SODR = AT91C_PA12_MISO;

  while (len) {
    /* Send the command byte and wait for a reply. */
    *AT91C_SPI_TDR = *data;
    data++;
    len--;

    /* Wait for the transmission to complete. */
    while (!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY)); /* TODO: Maybe just
                                                     wait for TDRE?
                                                     Would double data
                                                     rate... */
  }

  /* Deselect the chip manually again. */
  *AT91C_PIOA_SODR = AT91C_PA10_NPCS2;
}

/*
 * Internal functions implementing each basic command of the UC1601.
 */
void lcd_set_column_address(U32 addr) {
  spi_write_command(0x00 | (addr & 0xF));
  spi_write_command(0x10 | ((addr >> 4) & 0xF));
}

static void lcd_set_multiplex_rate(U32 rate) {
  spi_write_command(0x20 | (rate & 3));
}

void lcd_set_scroll_line(U32 sl) { /* TODO: See what it does. */
  spi_write_command(0x40 | (sl & 0x3f));
}

void lcd_set_page_address(U32 pa) {
  spi_write_command(0xB0 | (pa & 0xf));
}

static void lcd_set_bias_pot(U32 pot) {
  spi_write_command(0x81);
  spi_write_command(pot & 0xff);
}

static void lcd_set_ram_address_control(U32 ac) {
  spi_write_command(0x88 | (ac & 7));
}

void lcd_set_all_pixels_on(U32 on) {
  spi_write_command(0xA4 | ((on) ? 1 : 0));
}

static void lcd_inverse_display(U32 on) {
  spi_write_command(0xA6 | ((on) ? 1 : 0));
}

static void lcd_enable(U32 on) {
  spi_write_command(0xAE | ((on) ? 1 : 0));
}

static void lcd_set_map_control(U32 map_control) {
  spi_write_command(0xC0 | ((map_control & 3) << 1));
}

static void lcd_reset() {
  spi_write_command(0xE2);
}

static void lcd_set_bias_ratio(U32 ratio) {
  spi_write_command(0xE8 | (ratio & 3));
}


void lcd_init() {
  /* Initialize the SPI controller to enable communication. */
  spi_init();

  /* Wait for a little bit, so that the UC1601 can register our
   * presence. */
  systick_wait_ms(20);

  /* Issue a reset command, and wait. Normally here we'd check the
   * UC1601 status register, but as noted at the start of the file, we
   * can't read from the LCD controller due to the board setup.
   */
  lcd_reset();
  systick_wait_ms(20);

  /* Set the LCD power configuration.
   *
   * The LEGO Hardware Developer Kit documentation specifies that the
   * display should be configured with a multiplex rate (MR) of 1/65,
   * and a bias ratio (BR) of 1/9, and a display voltage V(LCD) of 9V.
   *
   * The specified MR and BR both map to simple command writes. V(LCD)
   * however is determined by an equation that takes into account both
   * the BR and the values of the PM (Potentiometer) and TC
   * (Temperature Compensation) configuration parameters.
   *
   * The equation and calculations required are a little too complex
   * to inline here, but the net result is that we should set a PM
   * value of 92. This will result in a smooth voltage gradient, from
   * 9.01V at -20 degrees Celsius to 8.66V at 60 degrees Celsius
   * (close to the maximum operational range of the LCD display).
   */
  lcd_set_multiplex_rate(3);
  lcd_set_bias_ratio(3);
  lcd_set_bias_pot(92);

  /* Set the RAM address control, which defines how the data we send
   * to the LCD controller are placed in its internal video RAM.
   *
   * We want the bytes we send to be written in row-major order (line
   * by line), with no automatic wrapping.
   */
  lcd_set_ram_address_control(0);

  /* Set the LCD mapping mode, which defines how the data in video RAM
   * is driven to the display. We don't want X or Y mirroring.
   */
  lcd_set_map_control(0);

  /* Turn the display on. */
  lcd_enable(1);
}

void lcd_shutdown() {
  lcd_reset();
  systick_wait_ms(20);
}

void lcd_test() {
  systick_wait_ms(2000);
  lcd_inverse_display(1);
  systick_wait_ms(2000);
  lcd_inverse_display(0);
  systick_wait_ms(2000);
}
