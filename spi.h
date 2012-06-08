#ifndef SPI_h_
#define SPI_h_

#include <stdint.h>
#include <avr/io.h>

#include "hardware.h"

enum {
	LOW = 0,
	HIGH = 1,
};

void spi_init(uint8_t mode);
void spi_set_mode(uint8_t config);
uint8_t spi_transfer(uint8_t value);
void spi_slave_select(uint8_t value);

#endif

