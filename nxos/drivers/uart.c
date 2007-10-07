#include "at91sam7s256.h"

#include "nxt.h"
#include "aic.h"

#include "uart.h"

#define UART_CLOCK_DIVISOR(baud_rate) \
( ((NXT_CLOCK_FREQ / baud_rate)+8) / 16 )


static volatile struct {

  uart_read_callback_t callback;

} uart_state = {
  0
};





void uart_init(uart_read_callback_t callback)
{
  uart_state.callback = callback;


  /* configuration : USART registers */



  /* clock & power : PMC */
  /* must enable the USART clock */



  /* Interruptions : AIC */
  /* not in edge sensitive mode => level */



  /* Interruptions : PDC */



  /* PIO */
  /* TXBT = PA22 */
  /* TXBT = PA11 */


}

void uart_write(void *data, U16 lng)
{

}

bool uart_can_write()
{
  return 0;
}
