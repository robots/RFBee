/*
 *
 * 2011 Michal Demin
 *
 */


#include <stdint.h>

#include "serial_fifo.h"

void serial_fifo_init(struct serial_fifo_t *b)
{
	b->write = 0;
	b->read = 0;
}

uint8_t serial_fifo_empty(struct serial_fifo_t *b)
{
	if (SERIAL_FIFO_EMPTY(b)) {
		return 1;
	}

	return 0;
}

/* FIXME:
uint16_t serial_fifo_GetAvailable(struct serial_fifo_t *b)
{
	uint16_t out;

	if (b->read > b->write) {
		out = b->read - b->write;
		out = SERIAL_FIFO_SIZE - out;
	} else {
		out = b->write - b->read;
	}

	return out;
}
*/

uint8_t serial_fifo_read(struct serial_fifo_t *b)
{
	uint8_t out;
	
	out = b->buf[b->read];

	if (!SERIAL_FIFO_EMPTY(b)) {
		b->read++;
		b->read %= SERIAL_FIFO_SIZE;
	}

	return out;
}

void serial_fifo_write(struct serial_fifo_t *b, uint8_t a)
{
	b->buf[b->write] = a;

	if (SERIAL_FIFO_FULL(b)) {
		return;
	}

	b->write ++;
	b->write %= SERIAL_FIFO_SIZE;
}

