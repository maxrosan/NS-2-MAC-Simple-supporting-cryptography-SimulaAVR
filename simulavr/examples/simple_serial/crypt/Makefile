############################################
#
#  Makefile to compile the AES256 libraries
#
############################################

TARGET=main

ALGORITHM=DES

# Here all the dirs to include to find .h files
DIR= .
DIR+= lib/gf256mul/

# Here C source files
SRC= $(TARGET).c

# Here ASM source files, Make them always end in a capital 'S', ex: asmfile.S
ASRC = lib/gf256mul/gf256mul.S

# frequency in hz
F_CPU=16000000

# Microcontroller to be used
MCU=atmega128

# Optimization 1, 2, 3 and s for size
OPT = s

# You should not have to change anything below here

# Here we add the prefix -I for the compiler ex: '-I ../dir/subdir/'

# Compiler C flags
CFLAGS = -mmcu=$(MCU) -DC$(MCU) -DF_CPU=$(F_CPU)UL -O$(OPT) -ffunction-sections
CFLAGS += -Wl,--section-start=.siminfo=0x900000

LDFLAGS = -lm 
DIR = .

ifeq ($(MCU), atmega328)

	SRC+= serial.c

else ifeq ($(MCU), atmega128)
	
	SRC+= avr-uart/uart.c
	SRC+= serial_atmega128.c
	
	DIR+= avr-uart/

endif

ifeq ($(ALGORITHM), AES)

	SRC+= lib/aes/aes_dec.c
	SRC+= lib/aes/aes_enc.c
	SRC+= lib/aes/aes_sbox.c
	SRC+= lib/aes/aes_invsbox.c
	SRC+= lib/aes/aes_keyschedule.c
	SRC+= lib/aes/aes256_enc.c
	SRC+= lib/aes/aes256_dec.c
	
	DIR+= lib/aes/
	
else ifeq ($(ALGORITHM), RC6)

	SRC+= lib/rc5/rc5.c
	
	DIR+= lib/rc5/
	
else ifeq ($(ALGORITHM), XTEA)

	SRC+= lib/xtea/xtea.c
	
	DIR+= lib/xtea/

	DIR+= lib/
	
else ifeq ($(ALGORITHM), DES)

	SRC+= lib/des/des.c
	
	DIR+= lib/des/

	DIR+= lib/
	
endif

CFLAGS += -D$(ALGORITHM)
IDIR = ${addprefix -I , $(DIR)}

# Compiler
CC      = avr-gcc
AR      = avr-ar
OBJCOPY = avr-objcopy
SIZE    = avr-size

# C files and ASM files to object files
OBJ = $(SRC:%.c=%.o) $(ASRC:%.S=%.o) 

all: $(TARGET).elf

clean:
	rm -rf *.o *.hex *.elf $(OBJ)

do: $(TARGET).elf
	PYTHONPATH=../../../src python ../simple_serial.py $(MCU) $< $(F_CPU)

# Compile: create object files from C and ASM source files.
%.o: %.c
	$(CC) $(CFLAGS) $(IDIR) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(IDIR) -c $< -o $@

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@ 	 program: $(TARGET).hex 	avrdude -p m328p -P /dev/ttyUSB0     -c stk500v1   -b 57600 -U flash:w:$(TARGET).hex 	 # Display the size of generated elf file ELFSIZE = $(SIZE) --mcu=$(MCU) --format=avr $(TARGET).elf size: 	@if test -f $(TARGET).elf; then echo; $(ELFSIZE); 2>/dev/null; echo; fi
