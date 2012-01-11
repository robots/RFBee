/*
  Spi.cpp - SPI library
  Copyright (c) 2008 Cam Thompson.
  Author: Cam Thompson, Micromega Corporation, <www.micromegacorp.com>
  Version: December 15, 2008

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <avr/io.h>

#include "spi.h"


void spi_init(uint8_t mode)
{
	DDRB |= SCK_PIN | MOSI_PIN | SS_PIN;
	DDRB &= ~MISO_PIN;

  // enable SPI Master, MSB, SPI mode 0, FOSC/4
  spi_set_mode(mode);
  
	SPI_PORT |= SS_PIN | SCK_PIN;
	SPI_PORT &= ~MOSI_PIN;
}

void spi_set_mode(uint8_t config)
{
  volatile uint8_t tmp;

  // enable SPI master with configuration byte specified
  SPCR = 0;
  SPCR = (config & 0x7F) | _BV(SPE) | _BV(MSTR);

  tmp = SPSR;
  tmp = SPDR;
	(void)tmp;
}

uint8_t spi_transfer(uint8_t value)
{
  uint8_t x;

  SPDR = value;
  while (!(SPSR & _BV(SPIF)));
  x = SPDR;

  return x;
}

void spi_slave_select(uint8_t value)
{
	if (value) {
		SPI_PORT |= SS_PIN;
	} else {
		SPI_PORT &= ~SS_PIN;
	}
}

