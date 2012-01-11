#ifndef SPI_h_
#define SPI_h_

#include <stdint.h>
#include <avr/io.h>

#define SPI_PORT PORTB
#define SPI_PIN  PINB

#define SS_PIN   _BV(PB2)
#define MOSI_PIN _BV(PB3)
#define MISO_PIN _BV(PB4)
#define SCK_PIN  _BV(PB5)

#define MISO_STATE (!!((SPI_PIN) & (MISO_PIN)))

enum {
	LOW = 0,
	HIGH = 1,
};

void spi_init(uint8_t mode);
void spi_set_mode(uint8_t config);
uint8_t spi_transfer(uint8_t value);
void spi_slave_select(uint8_t value);

#endif

