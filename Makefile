TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

# application source
C_SOURCE_FILES += main.c
C_SOURCE_FILES += floor.c

C_SOURCE_FILES += nrf_delay.c

SDK_PATH = nordic/sdk/nrf51822/

OUTPUT_FILENAME := main

DEVICE_VARIANT := xxaa
#DEVICE_VARIANT := xxab

CFLAGS := -DDEBUG_NRF_USER

# we do not use heap in this app
ASMFLAGS := -D__HEAP_SIZE=0

# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections

# let linker to dump unused sections
LDFLAGS := -Wl,--gc-sections

INCLUDEPATHS += -I"../"
INCLUDEPATHS += -I"$(SDK_PATH)Include/app_common"
INCLUDEPATHS += -I"$(SDK_PATH)Include/sd_common"

C_SOURCE_PATHS += $(SDK_PATH)Source/app_common
C_SOURCE_PATHS += $(SDK_PATH)Source/sd_common

include $(SDK_PATH)Source/templates/gcc/Makefile.common

clean:
	rm -rf *.o *.elf _build/
	
flash: all
	arm-none-eabi-gdb _build/main_xxaa.out \
		-ex="target remote localhost:3333" \
		-ex="load" \
		-ex="monitor reset halt" \
		-ex="continue"

