TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

# application source
C_SOURCE_FILES += main.c
C_SOURCE_FILES += floor.c
#C_SOURCE_FILES += ble_uart.c
#C_SOURCE_FILES += bluetooth.c

C_SOURCE_FILES += nrf_delay.c
#C_SOURCE_FILES += ble_srv_common.c
#C_SOURCE_FILES += ble_advdata.c
#C_SOURCE_FILES += ble_conn_params.c
#C_SOURCE_FILES += softdevice_handler.c
#C_SOURCE_FILES += app_timer.c
#C_SOURCE_FILES += app_uart_fifo_mod.c
#C_SOURCE_FILES += app_fifo.c
#C_SOURCE_FILES += app_gpiote.c
#C_SOURCE_FILES += ble_debug_assert_handler.c

SDK_PATH = nordic/sdk/nrf51822/
#SOFTDEVICE_PATH = ../nordic/softdevice/s110_nrf51822_7.3.0_softdevice.hex

OUTPUT_FILENAME := main

DEVICE_VARIANT := xxaa
#DEVICE_VARIANT := xxab

#USE_SOFTDEVICE := S110
#USE_SOFTDEVICE := S210

CFLAGS := -DDEBUG_NRF_USER -DBLE_STACK_SUPPORT_REQD -DS110

# we do not use heap in this app
ASMFLAGS := -D__HEAP_SIZE=0

# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections

# let linker to dump unused sections
LDFLAGS := -Wl,--gc-sections

INCLUDEPATHS += -I"../"
INCLUDEPATHS += -I"$(SDK_PATH)Include/s110"
INCLUDEPATHS += -I"$(SDK_PATH)Include/ble"
INCLUDEPATHS += -I"$(SDK_PATH)Include/ble/device_manager"
INCLUDEPATHS += -I"$(SDK_PATH)Include/ble/ble_services"
INCLUDEPATHS += -I"$(SDK_PATH)Include/app_common"
INCLUDEPATHS += -I"$(SDK_PATH)Include/sd_common"

C_SOURCE_PATHS += $(SDK_PATH)Source/ble
C_SOURCE_PATHS += $(SDK_PATH)Source/ble/device_manager
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
