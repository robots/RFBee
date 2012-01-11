//  rfBeeCore.h core routines for the rfBee
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: June 22, 2010
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


#ifndef RFBEECORE_H
#define RFBEECORE_H 1

#include "globals.h"
#include "ccx.h"
#include <avr/power.h>
#include <avr/sleep.h>

void transmitData(uint8_t *txData,uint8_t len, uint8_t srcAddress, uint8_t destAddress);
uint8_t txFifoFree();
int receiveData(uint8_t *rxData, uint8_t *len, uint8_t *srcAddress, uint8_t *destAddress, uint8_t *rssi , uint8_t *lqi);
void sleepNow(uint8_t mode);
void lowPowerOn();

#endif
