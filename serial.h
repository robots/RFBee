#ifndef SERIAL_h_
#define SERIAL_h_

#include <avr/io.h>


// 19200 @ 8mhz 
#define	BRREG_VALUE	25


void serial_init(uint32_t baud);
void serial_set_baudrate(uint32_t baud);
void serial_write_hex(uint8_t v);
void serial_write_string(uint8_t *str);
void serial_write(uint8_t c);
void serial_flush();
int serial_read(void);
uint8_t serial_available();

#endif

