# === MF2095 Task 3 — Auto-rebuild Makefile (ATmega328P / Arduino Uno) ===

# ---- Project ----
TARGET    = task4
SRC       = $(TARGET).c

# ---- MCU / clock ----
MCU       = atmega328p
F_CPU     = 16000000UL

# ---- Tools ----
CC        = avr-gcc
OBJCOPY   = avr-objcopy
AVRDUDE   = C:/Users/Dator2/AppData/Local/Arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/bin/avrdude.exe
AVRDUDE_CONF = C:/Users/Dator2/AppData/Local/Arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf

# ---- Ports / speeds ----
PORT      = COM3
BOOT_BAUD = 115200         # Optiboot upload speed
SERIAL_BAUD = 9600         # UART print speed in task3.c

# ---- Flags ----
CFLAGS    = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -std=gnu11 -Wall -Wextra
LDFLAGS   = -mmcu=$(MCU)

# ---- Phony targets ----
.PHONY: all clean flash monitor rebuild

# Default: always rebuild
all: rebuild

# Rebuild = clean first, then build fresh
rebuild: clean $(TARGET).hex

$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: $(TARGET).hex
	"$(AVRDUDE)" -C"$(AVRDUDE_CONF)" -v -p$(MCU) -carduino -P$(PORT) -b$(BOOT_BAUD) -D -U flash:w:$<:i

monitor:
	python -m serial.tools.miniterm $(PORT) $(SERIAL_BAUD)

clean:
	- rm -f $(TARGET).elf $(TARGET).hex
