//  CCx.cpp  Class to control the chipcon CCxxxx series transceivers
//  see http://focus.ti.com/lit/ds/symlink/cc1101.pdf for details on the CC1101

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: May 22, 2010
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ccx.h"
#include "ccx_cfg.h"

#include "spi.h"


// Power On Reset as described in  19.1.2 of cc1100 datasheet, tried APOR as described in 19.1.1 but that did not work :-(
void ccx_power_on_startup()
{
	//SPICLK=CPU/64
	spi_init(_BV(SPR1) | _BV(SPR0));

  // start manual Power On Reset 
  spi_slave_select(HIGH);
  _delay_us(1);
  
  spi_slave_select(LOW);
  _delay_us(10);
  
  spi_slave_select(HIGH);
  _delay_us(41);
  
  spi_slave_select(LOW);
  
  // wait for MISO to go low
  while(MISO_STATE);
  
  spi_transfer(CCx_SRES);
 
//  printf("Waiting for CCx to complete POR\n");

  // wait for MISO to go low
  while(MISO_STATE);
  
  spi_slave_select(HIGH);
  
//  printf("CCx POR complete\n");
}

uint8_t ccx_read(uint8_t addr, uint8_t *data)
{
  uint8_t result;
  
  spi_slave_select(LOW);
  // wait for MISO to go low
  while(MISO_STATE);
  
  result = spi_transfer(addr | 0x80);
  *data = spi_transfer(0);

  spi_slave_select(HIGH);
//printf("R: %02x %02x %02x\n", addr, result, *data);

  return result;
}

uint8_t ccx_read_burst(uint8_t addr, uint8_t *dataPtr, uint8_t size)
{
  uint8_t result;
  
  spi_slave_select(LOW);
  // wait for MISO to go low
  while(MISO_STATE);
  
  result = spi_transfer(addr | 0xc0);

  while(size) {
    *dataPtr++ = spi_transfer(0);
    size--;
  }
  
  spi_slave_select(HIGH);
  
  return result;
}

uint8_t ccx_write(uint8_t addr, uint8_t dat)
{
  uint8_t result;
  
  spi_slave_select(LOW);
//printf("W: %02x %02x\n\r", addr, dat);
  // wait for MISO to go low
  while(MISO_STATE);
  
  result = spi_transfer(addr);
  result = spi_transfer(dat);

  spi_slave_select(HIGH);
  
  return result;
}

uint8_t ccx_write_burst(uint8_t addr, const uint8_t* dataPtr, uint8_t size)
{
  uint8_t result;
  
  spi_slave_select(LOW);
  // wait for MISO to go low
  while(MISO_STATE);
  
  result = spi_transfer(addr | 0x40);

  while(size)
  {
    result = spi_transfer(*dataPtr++);
    size--;
  }
  
  spi_slave_select(HIGH);
  
  return result;
}

uint8_t ccx_strobe(uint8_t addr)
{
  uint8_t result;

  spi_slave_select(LOW);
  // wait for MISO to go low
  while(MISO_STATE);
  
  result = spi_transfer(addr);
//printf("S %02x\n", addr);
  
  spi_slave_select(HIGH);
  
  return result;
}

//configure registers of cc1100 making it work in specific mode
void ccx_setup(uint8_t configId)
{
	uint8_t reg;
	uint8_t val;
	uint8_t temp;

	(void)temp;

	if (configId < CCX_NR_OF_CONFIGS) {
		for(uint8_t i = 0; i< CCX_NR_OF_REGISTERS; i++) {
			reg = pgm_read_byte(&CCx_registers[i]);
			val = pgm_read_byte(&CCx_registerSettings[configId][i]);//read flash data no problem
			temp = ccx_write(reg, val);
		}
	}
}

// to aid debugging
//#ifdef DEBUG
void ccx_read_setup()
{
	uint8_t reg;
	uint8_t value;
	for(uint8_t i = 0; i< CCX_NR_OF_REGISTERS; i++) {
		reg = pgm_read_byte(&CCx_registers[i]);
		ccx_read(reg, &value);
		printf("%02x: %02x\r\n", reg, value);
	}
}
//#endif

void ccx_set_pa(uint8_t configId,uint8_t paIndex)
{
	uint8_t PAval; 

	PAval = pgm_read_byte(&CCx_paTable[configId][paIndex]);
	ccx_write(CCx_PATABLE, PAval);
}

void ccx_idle()
{
  ccx_strobe(CCx_SIDLE);
  while (ccx_strobe(CCx_SNOP) & 0xF0);
}

void ccx_mode(uint8_t md)
{

}

uint8_t ccx_get_config_num()
{
  return CCX_NR_OF_CONFIGS;
}

uint8_t ccx_decode_rssi(uint8_t rssiEnc)
{
  uint8_t rssi;
  uint8_t rssiOffset = 74;  // is actually dataRate dependant, but for simplicity assumed to be fixed.

  // RSSI is coded as 2's complement see section 17.3 RSSI of the cc1100 datasheet
  if (rssiEnc >= 128)
    rssi = ((rssiEnc - 256) >> 1) - rssiOffset;
  else
    rssi = (rssiEnc >> 1) - rssiOffset;
  return rssi;
}

