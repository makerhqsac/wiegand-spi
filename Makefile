# --------------------------------------------------------------
# Wiegand-SPI fimreware for ATTINY25/45/85
#
# (c) 2016 Sam Thompson <git@samt.us>
# The MIT License
DEVICE     = attiny85
CLOCK      = 1000000
PROGRAMMER = -c raspid_v1 -b 250000 -C +avrdude.conf
OBJECTS    = main.o
BUILD_DIR  = build

# Use this to calcuate fuses: http://www.engbedded.com/fusecalc/
FUSES      = -U hfuse:w:0xDF:m -U lfuse:w:0x62:m

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $(BUILD_DIR)/$@

flash:	all
	$(AVRDUDE) -U flash:w:$(BUILD_DIR)/main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID $(BUILD_DIR)/main.hex

clean:
	rm -f $(BUILD_DIR)/*

main.elf: $(OBJECTS)
	$(COMPILE) -o $(BUILD_DIR)/main.elf $(addprefix $(BUILD_DIR)/, $(OBJECTS))

main.hex: main.elf
	rm -f $(BUILD_DIR)/main.hex
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/main.elf $(BUILD_DIR)/main.hex
	avr-size --format=avr --mcu=$(DEVICE) $(BUILD_DIR)/main.elf

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d $(BUILD_DIR)/main.elf

cpp:
	$(COMPILE) -E main.c
