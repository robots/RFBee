//  Config.pde Simple config library for rfBee
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: August 27, 2010
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


#include "globals.h"
#include "config.h"

#include <stdint.h>

#include <avr/eeprom.h>


static uint8_t ee_read(int address)
{
	return eeprom_read_byte((unsigned char *) address);
}

static void ee_write(int address, uint8_t value)
{
	eeprom_write_byte((unsigned char *) address, value);
}


void config_reset()
{
	ee_write(CONFIG_RFBEE_MARKER, CONFIG_RFBEE_MARKER_VALUE);
	ee_write(CONFIG_FW_VERSION, FIRMWAREVERSION);
	ee_write(CONFIG_DEST_ADDR, 0);
	ee_write(CONFIG_MY_ADDR, 0);
	ee_write(CONFIG_ADDR_CHECK, 0x00);
	ee_write(CONFIG_TX_THRESHOLD, 0x01);
	ee_write(CONFIG_BDINDEX, 0x00);
	ee_write(CONFIG_PAINDEX, 0x07);
	ee_write(CONFIG_CONFIG_ID, 0x00);

	if (ee_read(CONFIG_HW_VERSION) < 11) {
		ee_write(CONFIG_HW_VERSION, 11);  // dirty hack to ensure rfBee's without a hardware version get their hardware version set to 1.0
	}

	ee_write(CONFIG_OUTPUT_FORMAT, 0x00);   
	ee_write(CONFIG_RFBEE_MODE, 0x00); 
}

uint8_t config_get(uint8_t idx)
{
  return (ee_read(idx));
}

void config_set(uint8_t idx, uint8_t value)
{
  ee_write(idx,value);
}

int config_initialized()
{
	if ((ee_read(CONFIG_RFBEE_MARKER) == CONFIG_RFBEE_MARKER_VALUE) && (ee_read(CONFIG_FW_VERSION) == FIRMWAREVERSION)) {
		return OK;
	}

	return ERR; 
}
