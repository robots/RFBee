/*
 *
 * 2011 Michal Demin
 *
 * parts taken from adruino code
 */
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "serial_fifo.h"

struct serial_fifo_t rx_fifo;
struct serial_fifo_t tx_fifo;
volatile uint8_t tx_sending;

#ifndef SERIAL_NO_PRINTF
static int serial_putchar(char c, FILE *stream);
FILE mystdout = FDEV_SETUP_STREAM(serial_putchar, NULL, _FDEV_SETUP_WRITE);
#endif

void serial_init(uint32_t baud)
{
	serial_set_baudrate(baud);

	serial_fifo_init(&rx_fifo);
	serial_fifo_init(&tx_fifo);
	tx_sending = 0;

#ifndef SERIAL_NO_PRINTF
	stdout = &mystdout;
#endif
}

void serial_set_baudrate(uint32_t baud)
{
  uint16_t baud_setting;
  uint8_t use_u2x = 1;

try_again:
  
  if (use_u2x) {
    UCSR0A = _BV(U2X0);
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    UCSR0A = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }
  
  if ((baud_setting > 4095) && use_u2x) {
    use_u2x = 0;
    goto try_again;
  }

	UBRR0H = baud_setting >> 8;
	UBRR0L = baud_setting & 0xFF;

	UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);
#ifdef DEBUG
	UDR0 = 'a';
#endif
}


void serial_write_hex(uint8_t v)
{
	unsigned char temp;

	temp = v >> 4;
	if(temp > 9)
		temp += 55;
	else
		temp += '0';
	serial_write(temp);

	temp = v & 0x0F;
	if(temp > 9)
		temp += 55;
	else
		temp += '0';

	serial_write(temp);
}

#ifndef SERIAL_NO_PRINTF
static int serial_putchar(char c, FILE *stream)
{
	serial_write(c);
	return 0;
}
#endif

void serial_write(uint8_t c)
{
	if (SERIAL_FIFO_FULL(&tx_fifo)) {
		return;
	}

	cli();

	if (tx_sending == 1) {
		serial_fifo_write(&tx_fifo, c);	
	} else {
		tx_sending = 1;
		UCSR0B |= _BV(UDRIE0);
		UDR0 = c;
	}

	sei();
}

void serial_write_string(uint8_t *str)
{
	while (str) {
		serial_write(*str);
		str++;
	}
}

void serial_flush()
{
	while (tx_sending);
}

int serial_read(void)
{
	uint8_t c;

	if (SERIAL_FIFO_EMPTY(&rx_fifo)) {
		return EOF;
	}

	c = serial_fifo_read(&rx_fifo);

	return c;
}

uint8_t serial_available()
{
	return !SERIAL_FIFO_EMPTY(&rx_fifo);
}

ISR(USART_UDRE_vect)
{
	if (SERIAL_FIFO_EMPTY(&tx_fifo)) {
		tx_sending = 0;
		UCSR0B &= ~_BV(UDRIE0);
		return;
	}

	UDR0 = serial_fifo_read(&tx_fifo);
}

ISR(USART_RX_vect)
{
	uint8_t c;

	c = UDR0;
	serial_fifo_write(&rx_fifo, c);
}
