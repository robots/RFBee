//  rfBeeSerial.h serial interface to rfBee
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: July 16, 2010
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


#ifndef RFBEESERIAL_H
#define RFBEESERIAL_H 1

#include "globals.h"
#include "config.h"
#include "ccx.h"
#include "rfBeeCore.h"
#include <avr/pgmspace.h>

#define BUFFLEN CCx_PACKT_LEN
#define SERIALCMDMODE 1
#define SERIALDATAMODE 0
#define SERIALCMDTERMINATOR 13  // use <CR> to terminate commands

// Supported commands, Commands and parameters in ASCII
// Example: ATDA14 means: change the RF module Destination Address to 14

typedef int (*AT_Command_Function_t)(); 

typedef struct
{
  const char *name;
  const uint8_t configItem;   // the ID used in the EEPROM
  const uint8_t paramDigits;  // how many digits for the parameter
  const uint8_t maxValue;     // maximum value of the parameter
  AT_Command_Function_t function; // the function which does the real work on change
}  AT_Command_t;

extern uint8_t errNo;
extern uint8_t serialData[BUFFLEN+1]; // 1 extra so we can easily add a /0 when doing a debug print ;-)
extern uint8_t serialMode;
extern volatile int sleepCounter;

// operating modes, used by ATMD

#define TRANSCEIVE_MODE 0
#define TRANSMIT_MODE 1     
#define RECEIVE_MODE 2 
#define LOWPOWER_MODE 3

#ifdef INTERRUPT_RECEIVE
volatile enum state
#endif


uint8_t getNumParamData(int *result, int size);
int modifyConfig(uint8_t configLabel, uint8_t paramSize, uint8_t paramMaxValue);
int processSerialCmd(uint8_t size);
void readSerialCmd();
void readSerialData();
void writeSerialError();
void writeSerialData();
int setMyAddress();
int setAddressCheck();
int setPowerAmplifier();
int changeUartBaudRate();
int setUartBaudRate();
int showFirmwareVersion();
int showHardwareVersion();
int resetConfig();
int setCCxConfig();
int setSerialDataMode();
int setRFBeeMode();
int setSleepMode();

#endif
