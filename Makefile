PRG            = rfbee
OBJ            = RFBee_v1_1.o serial.o serial_fifo.o spi.o rfBeeSerial.o rfBeeCore.o ccx.o config.o
MCU_TARGET     = atmega168
OPTIMIZE       = -Os -DF_CPU=8000000
LDFLAGS += -L/usr/avr/lib/avr5/

# You should not have to change anything below here.

CC             = avr-gcc
SIZE           = avr-size

# Override is only needed by avr-lib build system.

override CFLAGS = -std=c99 -Winline -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: hex
	$(SIZE) $(PRG).elf

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(PRG).elf $(PRG).hex

hex:  $(PRG).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

load: $(PRG).hex
	avrdude -c arduino -P /dev/ttyUSB0 -b 19200 -p $(MCU_TARGET) -U flash:w:$(PRG).hex
