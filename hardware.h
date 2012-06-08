#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>
#include <avr/io.h>

#define SPI_PORT          PORTB
#define SPI_PIN           PINB

#define SS_PIN            _BV(PB2)
#define MOSI_PIN          _BV(PB3)
#define MISO_PIN          _BV(PB4)
#define SCK_PIN           _BV(PB5)

#define MISO_STATE        (!!((SPI_PIN) & (MISO_PIN)))

#define GDO_PORT          PORTD
#define GDO_PIN           PIND
#define GDO_DDR           DDRD

#define GDO0              _BV(PD2)
#define GDO2              _BV(PD3)

#define wait_GDO2_high()  while(!(GDO_PIN & GDO2));
#define wait_GDO2_low()   while(GDO_PIN & GDO2);

#endif
