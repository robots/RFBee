PRG     = rfbee

SRC     = main.c serial.c serial_fifo.c spi.c rfBeeSerial.c rfBeeCore.c ccx.c config.c

#SMPL_DEF += -DMRFI_CC1101
#
#SMPL_INC += -Isimpliciti
#SMPL_INC += -Isimpliciti/Components/bsp
#SMPL_INC += -Isimpliciti/Components/bsp/boards/RFBee
#SMPL_INC += -Isimpliciti/Components/mrfi
#SMPL_INC += -Isimpliciti/Components/simpliciti/nwk
#SMPL_INC += -Isimpliciti/Components/simpliciti/nwk_applications
#
#SMPL_SRC = simpliciti/Components/simpliciti/nwk_applications/nwk_link.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_security.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_ping.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_ioctl.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_join.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_mgmt.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_freq.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk_applications/nwk_pll.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk/nwk_globals.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk/nwk_api.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk/nwk_frame.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk/nwk.c
#SMPL_SRC += simpliciti/Components/simpliciti/nwk/nwk_QMgmt.c
#
#SRC += $(SMPL_SRC)

MCU     = atmega168
LDFLAGS += -L/usr/avr/lib/avr5/

OPT     = s
DEFS    = -DF_CPU=8000000

CC      = avr-gcc
SIZE    = avr-size

CFLAGS  = -std=c99 -Winline -Wall -O$(OPT) -mmcu=$(MCU) $(DEFS)
#$(SMPL_INC)

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

OBJS    = $(addsuffix .o,$(basename $(SRC)))

all: hex
	$(SIZE) $(PRG).elf

$(PRG).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(PRG).elf $(PRG).hex

hex:  $(PRG).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

load: $(PRG).hex
	avrdude -c arduino -P /dev/ttyUSB0 -b 19200 -p $(MCU) -U flash:w:$(PRG).hex
