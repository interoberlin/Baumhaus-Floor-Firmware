GDB=arm-none-eabi-gdb

TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

C_SOURCE_FILES += main.c
C_SOURCE_FILES += floor.c

OUTPUT_FILENAME := main
SDK_PATH = ../nordic/sdk/nrf51822/


#Uncomment correct line if you have s110 programmed on the chip.
DEVICE_VARIANT := xxaa

#USE_SOFTDEVICE := s110

include $(SDK_PATH)Source/templates/gcc/Makefile.common

define LOAD_SOFTDEVICE_COMMAND
target remote localhost:3333
load
monitor reset halt
continue
endef
export LOAD_SOFTDEVICE_COMMAND

# Flash the SoftDevice HEX-file into the chip
flash:
	echo "$$LOAD_SOFTDEVICE_COMMAND" | $(GDB) _build/main_xxaa.hex


