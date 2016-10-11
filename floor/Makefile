GDB=arm-none-eabi-gdb

TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

C_SOURCE_FILES += main.c
C_SOURCE_FILES += floor.c

OUTPUT_FILENAME := main
SDK_PATH = ../nordic/sdk/nrf51822/
SOFTDEVICE_PATH = ../nordic/softdevice/s110_nrf51822_7.3.0_softdevice.hex


#Uncomment correct line if you have s110 programmed on the chip.
DEVICE_VARIANT := xxaa

USE_SOFTDEVICE := s110

include $(SDK_PATH)Source/templates/gcc/Makefile.common

define GDB_LOAD
target remote localhost:3333
load
monitor reset halt
endef
export GDB_LOAD

define GDB_LOAD_CONTINUE
target remote localhost:3333
load
monitor reset halt
continue
endef
export GDB_LOAD_CONTINUE

softdevice:
	echo "$$GDB_LOAD" | $(GDB) $(SOFTDEVICE_PATH)

flash:
	echo "$$GDB_LOAD_CONTINUE" | $(GDB) _build/main_xxaa.hex
