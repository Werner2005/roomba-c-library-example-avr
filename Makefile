ARCH := avr-
CC := $(ARCH)gcc
AR := $(ARCH)ar
OBJCOPY := $(ARCH)objcopy
PROG := connector.elf
FILES := connector.c uart.c
OBJS := $(FILES:%.c=%.o)
MCU=atmega8
F_CPU=8000000UL
CFLAGS += -Wall -Werror -ansi -pedantic -g -I$(ROOMBALIBDIR)/includes -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99 -Os -DROOMBA_SENSOR_BUFFER_SIZE=50
ROOMBALIBDIR := ../lib/
ROOMBACONFIG := --enable-short-read --enable-nonblocking --host=avr BUFFER_SIZE=50 CFLAGS=" -mmcu=$(MCU) "
LOADLIBES:=-Llib -lroomba
FORMAT := ihex

HEX := $(PROG:%.elf=%.hex)

all: lib/Makefile $(HEX) 

$(HEX): $(PROG)
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

$(PROG): $(OBJS) 
	make -C lib all
	$(CC) $(CFLAGS) -o $@  $^ $(LOADLIBES)

#$(ROOMBALIBDIR)/libroomba.a:
#	make -C $(ROOMBALIBDIR) CC=$(CC) AR=$(AR) OBJCOPY=$(OBJCOPY) CFLAGS_ADD="-DROOMBA_SENSOR_BUFFER_SIZE=50 -mmcu=$(MCU)"

lib/Makefile:
	-@mkdir lib
	@cd lib;\
	if test -f $(ROOMBALIBDIR)/configure; then \
		$(ROOMBALIBDIR)/configure $(ROOMBACONFIG);\
	elif test -f ../$(ROOMBALIBDIR)/configure; then \
		../$(ROOMBALIBDIR)/configure $(ROOMBACONFIG); \
	else \
		echo "Error: Roomba Library Configure"; \
		exit 1; \
	fi

clean: 
	-rm $(PROG) $(OBJS) $(HEX)
	-rm -R lib

flash: $(HEX)
	avrdude -p m128rfa1 -c avr910 -P /dev/ttyUSB0 -U flash:w:$^
