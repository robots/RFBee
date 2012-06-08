//  rfBeeSerial.pde serial interface to rfBee
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: August 18, 2010
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
#include <string.h>

#include <util/delay.h>

#include "serial.h"
#include "ccx.h"

#include "rfBeeSerial.h"

#define PAYLOAD_OFFSET 2

#define BUFFLEN    (CCx_PACKT_LEN-PAYLOAD_OFFSET)

struct ccxPacket_t tx_packet;
uint8_t *serialData = &tx_packet.frame[PAYLOAD_OFFSET];

uint8_t serialMode;
volatile int sleepCounter;


// RFbee AT commands

// Need to define the labels outside the struct :-(
static const char DA_label[] PROGMEM = "DA";
static const char MA_label[] PROGMEM = "MA";
static const char AC_label[] PROGMEM = "AC";
static const char PA_label[] PROGMEM = "PA";
static const char TH_label[] PROGMEM = "TH";
static const char BD_label[] PROGMEM = "BD";
static const char MD_label[] PROGMEM = "MD";
static const char FV_label[] PROGMEM = "FV";
static const char HV_label[] PROGMEM = "HV";
static const char RS_label[] PROGMEM = "RS";
static const char CF_label[] PROGMEM = "CF";
static const char OF_label[] PROGMEM = "OF";
static const char O0_label[] PROGMEM = "O0";
static const char SL_label[] PROGMEM = "SL";

static const AT_Command_t atCommands[] PROGMEM =
{
// Addressing:
	{ DA_label, CONFIG_DEST_ADDR,     3, 255, setDstAddress },                   // Destination address   (0~255)
	{ MA_label, CONFIG_MY_ADDR,       3, 255, setMyAddress },        // My address            (0~255)
	{ AC_label, CONFIG_ADDR_CHECK,    1, 2,   setAddressCheck },     // address check option  (0: no, 1: address check , 2: address check and 0 broadcast )
// RF
	{ PA_label, CONFIG_PAINDEX,       1, 7,   setPowerAmplifier },   // Power amplifier           (0: -30 , 1: -20 , 2: -15 , 3: -10 , 4: 0 , 5: 5 , 6: 7 , 7: 10 )
	{ CF_label, CONFIG_CONFIG_ID,     1, 5,   setCCxConfig },        // select CCx configuration  (0: 915 Mhz - 76.8k, 1: 915 Mhz - 4.8k sensitivity, 2: 915 Mhz - 4.8k low current, 3: 868 Mhz - 76.8k, 4: 868 Mhz - 4.8k sensitivity, 5: 868 Mhz - 4.8k low current )
// Serial
	{ BD_label, CONFIG_BDINDEX,       1, 3,   changeUartBaudRate },  // Uart baudrate                    (0: 9600 , 1:19200, 2:38400 ,3:115200)
	{ TH_label, CONFIG_TX_THRESHOLD,  2, 32,  0 },                   // TH- threshold of transmitting    (0~32) 
	{ OF_label, CONFIG_OUTPUT_FORMAT, 1, 4,   0 },                   // Output Format                    (0: payload only, 1: source, dest, payload ,  2: payload len, source, dest, payload, rssi, lqi, 3: same as 2, but all except for payload as decimal and separated by comma's )
// Mode 
	{ MD_label, CONFIG_RFBEE_MODE,    1, 3,   setRFBeeMode },        // CCx Working mode                 (0:transceive , 1:transmit , 2:receive, 3:lowpower)
	{ O0_label, 0,                    0, 0,   setSerialDataMode },   // thats o+ zero, go back to online mode
	{ SL_label, 0,                    0, 0,   setSleepMode },        // put the rfBee to sleep
// Diagnostics
	{ FV_label, 0,                    0, 0,   showFirmwareVersion }, // firmware version
	{ HV_label, 0,                    0, 0,   showHardwareVersion }, // hardware version
// Miscelaneous
	{ RS_label, 0,                    0, 0,   resetConfig }          // restore default settings
};

// error codes and labels
uint8_t errNo;

static const char error_0[] PROGMEM = "no error";
static const char error_1[] PROGMEM = "Packet too large";
static const char error_2[] PROGMEM = "received invalid RF data";
static const char error_3[] PROGMEM = "RX buffer overflow";
static const char error_4[] PROGMEM = "CRC check failed";

static const char * const error_codes[] PROGMEM =
{
	error_0,
	error_1,
	error_2,
	error_3,
	error_4,
};

const long baudRateTable[] PROGMEM = { 9600, 19200, 38400, 115200 };

uint8_t getNumParamData(int *result, int size)
{
	// try to read a number
	uint8_t c;
	int value = 0;
	uint8_t valid = 0;
	int pos = 4; // we start to read at pos 5 as 0-1 = AT and 2-3 = CMD

	if (serialData[pos] == SERIALCMDTERMINATOR )  // no data was available
		return NOTHING;

	while (size-- > 0) {
		c = serialData[pos++];
		if (c == SERIALCMDTERMINATOR)  // no more data available 
			break;
		if ((c < '0') || (c > '9'))     // illegal char
			return ERR;

		// got a digit
		valid = 1;
		value = (value * 10) + (c - '0');
	}

	if (valid) {
		*result = value;
		return OK;
	}

	return ERR;
}

// simple standardized setup for commands that only modify config
int modifyConfig(uint8_t configLabel, uint8_t paramSize, uint8_t paramMaxValue)
{
	int param;

	uint8_t result = getNumParamData(&param, paramSize);
	if (result == OK) {
		if (param <= paramMaxValue) {
			config_set(configLabel, param);
			return MODIFIED;
		}
	}

	if (result == NOTHING) {
		// return current setting
		printf("%d\r\n", config_get(configLabel));
		return OK;
	}

	return ERR;
}

int processSerialCmd(uint8_t size)
{
	int result = MODIFIED;

	uint8_t configItem;   // the ID used in the EEPROM
	uint8_t paramDigits;  // how many digits for the parameter
	uint8_t maxValue;     // maximum value of the parameter
	AT_Command_Function_t function; // the function which does the real work on change

	// read the AT
	if (strncasecmp("AT", (char *)serialData, 2) != 0) {
		return ERR;
	}

	// read the command
	for(int i = 0; i <= sizeof(atCommands)/sizeof(AT_Command_t); i++) {
		// do we have a known command
		if (strncasecmp_P((char *) serialData+2 , (PGM_P) pgm_read_word(&(atCommands[i].name)), 2) == 0) {
			// get the data from PROGMEM
			configItem = pgm_read_byte(&(atCommands[i].configItem));
			paramDigits = pgm_read_byte(&(atCommands[i].paramDigits));
			maxValue = pgm_read_byte(&(atCommands[i].maxValue));
			function = (AT_Command_Function_t) pgm_read_word(&(atCommands[i].function));

			if (paramDigits > 0)
				result = modifyConfig(configItem, paramDigits, maxValue);

			if (result == MODIFIED) {
				result = OK;  // config only commands always return OK
				if (function)
					result = function();  // call the command function
			}

			return(result);  // return the result of the execution of the function linked to the command
		}
	}

	return ERR;
}

void readSerialCmd()
{
	int result;
	char data;
	static uint8_t pos = 0;

	while (serial_available() && (serialMode == SERIALCMDMODE)) {      //ATSL changes commandmode while there is a char waiting in the serial buffer.
		result = NOTHING;
		data = serial_read();
		serialData[pos++] = data; //serialData is our global serial buffer
		if (data == SERIALCMDTERMINATOR) {
			if (pos > 3) { // we need 4 bytes
				result = processSerialCmd(pos);
			} else {
				result = ERR;
			}
			pos = 0;
		}
		// check if we don't overrun the buffer, if so empty it
		if (pos > BUFFLEN){
			result = ERR;
			pos = 0;
		}
		if (result == OK) {
			printf("ok\r\n");
		} else if (result == ERR) {
			printf("error\r\n");
		}
	}
}

void readSerialData()
{
	uint8_t len;
	uint8_t data;
	uint8_t fifoSize=0;
	static uint8_t plus=0;
	static uint8_t pos=0;
	uint8_t rfBeeMode;
	int i;

	// insert any plusses from last round
	for(i=pos; i < plus; i++) //be careful, i should start from pos, -changed by Icing
		serialData[i]='+';

	len = serial_available() + plus + pos;
	if (len > BUFFLEN)
		len = BUFFLEN; //only process at most BUFFLEN chars

	// check how much space we have in the TX fifo
	fifoSize = txFifoFree();// the fifoSize should be the number of bytes in TX FIFO

	if (fifoSize <= 0) {
		serial_flush();
		//ccx_strobe(CCx_SFTX);
		plus = 0;
		pos = 0;
		return;
	}

	if (len > fifoSize)
		len = fifoSize;  // don't overflow the TX fifo

	for (i = plus + pos; i < len; i++) {
		data = serial_read();
		serialData[i] = data;  //serialData is our global serial buffer
		if (data == '+') {
			plus++;
		} else {
			plus=0;
		}

		if (plus == 3) {
			len = i - 2; // do not send the last 2 plusses
			plus = 0;
			serialMode = SERIALCMDMODE;
			ccx_strobe(CCx_SIDLE);
			printf("ok, starting cmd mode\r\n");
			break;  // jump out of the loop, but still send the remaining chars in the buffer 
		}
	}

	if (plus > 0) {
		// save any trailing plusses for the next round
		len -= plus;
	}

	// check if we have more input than the transmitThreshold, if we have just switched to commandmode send  the current buffer anyway.
	if ((serialMode != SERIALCMDMODE)  && (len < config_get(CONFIG_TX_THRESHOLD))){
		pos = len;  // keep the current bytes in the buffer and wait till next round.
		return;
	}

	if (len > 0) {
		rfBeeMode = config_get(CONFIG_RFBEE_MODE);
		//only when TRANSMIT_MODE or TRANSCEIVE,transmit the buffer data,otherwise ignore
		if ((rfBeeMode == TRANSMIT_MODE) || (rfBeeMode == TRANSCEIVE_MODE)) {
			tx_packet.len = len + PAYLOAD_OFFSET;
			transmitData(&tx_packet);
			//transmitData(serialData, len, config_get(CONFIG_MY_ADDR), config_get(CONFIG_DEST_ADDR));
		}
		pos = 0; // serial databuffer is free again.
	}
}

// write a text label from progmem, uses less bytes than individual Serial.println()
//void writeSerialLabel(uint8_t i){
//  char buffer[64];
//  strcpy_P(buffer, (char * )pgm_read_word(&(labels[i])));
//  Serial.println(buffer);
//}

void writeSerialError()
{
	char buffer[64];

	strcpy_P(buffer, (char * )pgm_read_word(&(error_codes[errNo])));
	printf("error: %s\r\n", buffer);
}

// read data from CCx and write it to Serial based on the selected output format
void writeSerialData()
{
	struct ccxPacket_t packet;

	int result;
	uint8_t of;
	uint8_t i;

	result = receiveData(&packet);
	if (result == ERR) {
		writeSerialError();
		return;
	}

	if (result == NOTHING) {
		return;
	}

// write to serial based on output format:
//  0: payload only
//  1: source, dest, payload
//  2: payload len, source, dest, payload, rssi, lqi
//  3: payload len, source, dest, payload,",", rssi (DEC),",",lqi (DEC)
	of = config_get(CONFIG_OUTPUT_FORMAT);
	packet.frame[packet.len] = '\0';

	if ((of == 4) && (packet.len <= 8)) {
		of = 3;
	}

	if (of == 0) {
		printf("%s", packet.frame);
	} else if (of == 1) {
		printf("%02x,%02x,%s\r\n", packet.frame[0], packet.frame[1], &packet.frame[2]);
	} else if (of == 2) {
		printf("%d,%02x,%02x,%s,%02x,%02x\n\r", packet.len, packet.frame[0], packet.frame[1], &packet.frame[2], packet.rssi, packet.lqi);
	} else if (of == 3) {
		printf("%02x,%02x,%02x,", packet.len, packet.frame[0], packet.frame[1]);
		for (i = 0; i < packet.len-2; i++)
			printf("%02x", packet.frame[2+i]);
		printf(",%d,%d\r\n", packet.rssi, packet.lqi);
	} else if (of == 4) {
		printf("%02x,", packet.len);

		for (i = 0; i < 4; i++)
			printf("%02x", packet.frame[i]);
		printf(",");

		for (i = 4; i < 8; i++)
			printf("%02x", packet.frame[i]);
		printf(",");

		for (i = 8; i < packet.len; i++)
			printf("%02x", packet.frame[i]);

		printf(",%d,%d\r\n", packet.rssi, packet.lqi);
	}	else {
		printf("error: unknown format\r\n");
	}
}

int setMyAddress()
{
	uint8_t addr;

	addr = config_get(CONFIG_MY_ADDR);
	tx_packet.frame[1] = addr;
	ccx_write(CCx_ADDR, addr);
	return OK;
}

int setDstAddress()
{
	uint8_t addr;

	addr = config_get(CONFIG_DEST_ADDR);
	tx_packet.frame[0] = addr;

	return OK;
}

int setAddressCheck()
{
	ccx_write(CCx_PKTCTRL1, config_get(CONFIG_ADDR_CHECK) | 0x04);
	return OK;
}

int setPowerAmplifier()
{
	ccx_set_pa(config_get(CONFIG_CONFIG_ID), config_get(CONFIG_PAINDEX));
	return OK;
}

int changeUartBaudRate()
{
	printf("ok\r\n");
	serial_flush();
	_delay_ms(1);
	setUartBaudRate();
	return OK;
}

int setUartBaudRate()
{
	serial_init(pgm_read_dword(&baudRateTable[config_get(CONFIG_BDINDEX)]));
	return NOTHING;  // we already sent an ok.
}

int showFirmwareVersion()
{
	printf("%d.%d\r\n", FIRMWAREVERSION / 10, FIRMWAREVERSION % 10);
	return OK;
}

int showHardwareVersion()
{
	uint8_t hw = config_get(CONFIG_HW_VERSION);
	printf("%d.%d\r\n", hw / 10, hw % 10);
	return OK;
}

int resetConfig()
{
	config_reset();
	return OK;
}

int setCCxConfig()
{
	// load the appropriate config table
	uint8_t cfg = config_get(CONFIG_CONFIG_ID);

	ccx_setup(cfg);
	//ccx_read_setup();
	// and restore the config settings
	setMyAddress();
	setDstAddress();
	setAddressCheck();
	setPowerAmplifier();
	setRFBeeMode();

	return OK;
}

int setSerialDataMode()
{
	serialMode = SERIALDATAMODE;
	return OK;
}

int setRFBeeMode()
{
	// CCx_MCSM1 is configured to have TX and RX return to proper state on completion or timeout
	switch (config_get(CONFIG_RFBEE_MODE)) {
	case TRANSMIT_MODE:
		ccx_strobe(CCx_SIDLE);
		_delay_ms(1);
		ccx_write(CCx_MCSM1 ,   0x00 );//TXOFF_MODE->stay in IDLE
		ccx_strobe(CCx_SFTX);
		break;
	case RECEIVE_MODE:
		ccx_strobe(CCx_SIDLE);
		_delay_ms(1);
		ccx_write(CCx_MCSM1 ,   0x0C );//RXOFF_MODE->stay in RX
		ccx_strobe(CCx_SFRX);
		ccx_strobe(CCx_SRX);
		break;
	case TRANSCEIVE_MODE:
		ccx_strobe(CCx_SIDLE);
		_delay_ms(1);
		ccx_write(CCx_MCSM1 ,   0x0F );//RXOFF_MODE and TXOFF_MODE stay in RX
		ccx_strobe(CCx_SFTX);
		ccx_strobe(CCx_SFRX);
		ccx_strobe(CCx_SRX);
		break;
	case LOWPOWER_MODE:
		ccx_strobe(CCx_SIDLE);
		break;
	default:
		break;
	}
	return OK;
}

// put the rfbee into sleep
int setSleepMode()
{
#ifdef DEBUG
	printf("going to sleep\r\n");
#endif
	ccx_strobe(CCx_SIDLE);
	ccx_strobe(CCx_SPWD);
	sleepNow(SLEEP_MODE_IDLE);
	//sleepNow(SLEEP_MODE_PWR_DOWN);
#ifdef DEBUG
	printf("just woke up\r\n");
#endif
	setRFBeeMode();
	setSerialDataMode();
	return NOTHING;
}
