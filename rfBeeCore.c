//  rfBeeCore.cpp core routines for the rfBee
//  see www.seeedstudio.com for details and ordering rfBee hardware.

//  Copyright (c) 2010 Hans Klunder <hans.klunder (at) bigfoot.com>
//  Author: Hans Klunder, based on the original Rfbee v1.0 firmware by Seeedstudio
//  Version: July 14, 2010
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

#include <avr/interrupt.h>
#include <util/delay.h>

#include "globals.h"
#include "rfBeeCore.h"
#include "rfBeeSerial.h"

// send data via RF
//void transmitData(uint8_t *txData, uint8_t len, uint8_t srcAddress, uint8_t destAddress)
void transmitData(struct ccxPacket_t *packet)
{
	uint8_t stat;
	uint8_t size;

	(void)stat;

	// disable RX mode
	ccx_idle();

	// flush the RX buffer
	ccx_strobe(CCx_SFRX);

	// flush the TX buffer
	ccx_strobe(CCx_SFTX);

	// prepare TX mode
	ccx_strobe(CCx_SFSTXON);

#ifdef DEBUG
	printf("len= %d\n\r", packet->len + 1);
#endif

	// write packet (len + data)
	ccx_write_burst(CCx_TXFIFO, (uint8_t *)packet, packet->len + 1);

	// send packet
	ccx_strobe(CCx_STX);

	// wait for SYNC transmission
	wait_GDO2_high();
#ifdef DEBUG
	printf("sync\n\r");
#endif

	// wait for packet completion
	wait_GDO2_low();
#ifdef DEBUG
	printf("sent\n\r");
#endif

	// return to RX mode
	ccx_strobe(CCx_SFTX); // flush the TX buffer
	ccx_strobe(CCx_SRX);

#ifdef DEBUG
	packet->frame[packet->len] = '\0';
	for (int i = 0; i < packet->len; i++)
		printf("%02x", packet->frame[i]);
#endif
}

// read available txFifo size and handle underflow (which should not have occured anyway)
uint8_t txFifoFree()
{
	uint8_t stat;
	uint8_t size;

	(void)stat;

	stat = ccx_read(CCx_TXBYTES, &size);

	// handle a potential TX underflow by flushing the TX FIFO as described in section 10.1 of the CC 1100 datasheet
	if (size & 0x80) {//state got here seems not right, so using size to make sure it no more than 64
		ccx_strobe(CCx_SFTX);
		stat = ccx_read(CCx_TXBYTES, &size);
#ifdef DEBUG
		printf("stat %02x\r\n", stat);
#endif
	}

#ifdef DEBUG
	printf("fifo left %02x size %d\r\n", CCx_FIFO_SIZE - size, size);
#endif
	return (CCx_FIFO_SIZE - size);
}

// receive data via RF, rxData must be at least CCx_PACKT_LEN bytes long
int receiveData(struct ccxPacket_t *packet)
{
	uint8_t fifo_len;

	uint8_t rx_bytes;
	uint8_t rx_bytes1;

	// workaround for bug in cc1101 chip
	ccx_read(CCx_RXBYTES, &rx_bytes1);
	do {
		rx_bytes = rx_bytes1;
		ccx_read(CCx_RXBYTES, &rx_bytes1);
	} while (rx_bytes != rx_bytes1);

	ccx_read(CCx_RXFIFO, &fifo_len);
#ifdef DEBUG
	printf("length: %d %d\r\n", fifo_len, rx_bytes);
#endif

	// MD: check buffer size, drop large packets
	// also handles fifo overflow
	if ((rx_bytes != fifo_len + 1 + 2) || (fifo_len > CCx_PACKT_LEN)) {
		errNo = 1;
		cli();
		ccx_idle();
		ccx_strobe(CCx_SFRX); // flush the RX buffer
		ccx_strobe(CCx_SRX);
		sei();
		return ERR;
	}

	packet->len = fifo_len;
	ccx_read_burst(CCx_RXFIFO, packet->frame, fifo_len);
	ccx_read_burst(CCx_RXFIFO, packet->metrics, 2);

	// check checksum ok
	if ((packet->lqi & 0x80) == 0) {
		errNo = 4;
		return ERR;
		//return NOTHING;
	}

	// strip off the CRC bit
	packet->lqi = packet->lqi & 0x7F;
	packet->rssi = ccx_decode_rssi(packet->rssi);

	return OK;
}

// power saving
void sleepNow(uint8_t mode)
{
	/* Now is the time to set the sleep mode. In the Atmega8 datasheet
	 * http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
	 * there is a list of sleep modes which explains which clocks and
	 * wake up sources are available in which sleep modus.
	 *
	 * In the avr/sleep.h file, the call names of these sleep modus are to be found:
	 *
	 * The 5 different modes are:
	 *     SLEEP_MODE_IDLE         -the least power savings
	 *     SLEEP_MODE_ADC
	 *     SLEEP_MODE_PWR_SAVE
	 *     SLEEP_MODE_STANDBY
	 *     SLEEP_MODE_PWR_DOWN     -the most power savings
	 *
	 *  the power reduction management <avr/power.h>  is described in
	 *  http://www.nongnu.org/avr-libc/user-manual/group__avr__power.html
	 */
	set_sleep_mode(mode);   // sleep mode is set here

	sleep_enable();          // enables the sleep bit in the mcucr register
	// so sleep is possible. just a safety pin 

	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();

	sleep_mode();            // here the device is actually put to sleep!!

	// THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
	sleep_disable();         // first thing after waking from sleep:
	// disable sleep...

	power_all_enable();
}

void lowPowerOn()
{
	ccx_write(CCx_WORCTRL, 0x78);  // set WORCRTL.RC_PD to 0 to enable the wakeup oscillator
	ccx_strobe(CCx_SWOR);

	sleepNow(SLEEP_MODE_IDLE);
	//ccx_strobe(CCx_SIDLE);
}
