//  Firmware for rfBee 
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: July 14, 2010
//
//  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with this program; 
//  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#include <stdio.h>
#include <avr/interrupt.h>

#include "globals.h"
#include "config.h"
#include "serial.h"
#include "ccx.h"
#include "rfBeeSerial.h"

#ifdef FACTORY_SELFTEST
#include "TestIO.h"  // factory selftest
#endif

#define GDO_PORT PORTD
#define GDO_PIN  PIND
#define GDO_DDR  DDRD

#define GDO0 _BV(PD2)
#define GDO2 _BV(PD3)


#undef DEBUG

static void rfBeeInit();

static void setup()
{
	sei();

	if (config_initialized() != OK) {
		serial_init(9600);
		printf("Initializing config\r\n");
#ifdef FACTORY_SELFTEST
		if ( TestIO() != OK )
			return;
#endif 
		config_reset();
	}

	setUartBaudRate();
	rfBeeInit();
	printf("ok\r\n");
}

static void loop()
{
	if (serial_available() > 0) {
#ifdef DEBUG
		printf("Sdat\r\n");
#endif
		sleepCounter = 1000; // reset the sleep counter
		if (serialMode == SERIALCMDMODE) {
			readSerialCmd();
    } else {
      readSerialData();
		}
  }

	if (GDO_PIN & GDO0) {
#ifdef DEBUG
		printf("Rdat\r\n");
#endif
		writeSerialData();
		sleepCounter++; // delay sleep
	}

	sleepCounter--;
  
	// check if we can go to sleep again, going into low power too early will result in lost data in the CCx fifo.
	if ((sleepCounter == 0) && (config_get(CONFIG_RFBEE_MODE) == LOWPOWER_MODE)) {
#ifdef DEBUG
		printf("low power on\r\n");
#endif
		lowPowerOn();
#ifdef DEBUG
		printf("woke up\r\n");
#endif
	}
}


static void rfBeeInit()
{
	ccx_power_on_startup();
	setCCxConfig();

	serialMode = SERIALDATAMODE;
	sleepCounter = 0;

	// used for polling the RF received data
	GDO_DDR &= ~GDO0;
	GDO_PORT &= ~GDO0;

	EICRA |= _BV(ISC01) | _BV(ISC00); // rising edge
	EIMSK |= _BV(INT0);
}

//GD00 is located on INT 0
ISR(INT0_vect)
{
	sleepCounter = 10;
}

int main(void)
{
	setup();
    
	while(1) {
		loop();
	}
	return 0;
}

