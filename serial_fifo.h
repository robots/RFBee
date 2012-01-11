/*
 * 2010-2011 Michal Demin
 *
 */

#ifndef SERIAL_FIFO_H_
#define SERIAL_FIFO_H_

#include <stdint.h>

#define SERIAL_FIFO_SIZE 64

#define SERIAL_FIFO_EMPTY(x) (((x)->write == (x)->read))
#define SERIAL_FIFO_FULL(x)  ((((x)->write + 1) % SERIAL_FIFO_SIZE) == (x)->read)


struct serial_fifo_t {
	uint8_t read;
	uint8_t write;
	uint8_t buf[SERIAL_FIFO_SIZE];
} __attribute__ ((packed));


void serial_fifo_init(struct serial_fifo_t *b);
uint8_t serial_fifo_empty(struct serial_fifo_t *b);
uint8_t serial_fifo_read(struct serial_fifo_t *b);
void serial_fifo_write(struct serial_fifo_t *b, uint8_t a);

#endif
