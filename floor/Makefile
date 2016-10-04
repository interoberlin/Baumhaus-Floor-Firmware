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
