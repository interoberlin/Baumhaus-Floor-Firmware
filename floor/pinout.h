#ifndef PINOUT_H
#define PINOUT_H

// active low
#define ATOLED1_XPRO_LED1                   00
#define ATOLED1_XPRO_LED2                   02
#define ATOLED1_XPRO_LED3                   04

// nRF pins to which the sensor outputs are connected
#define sensor_pin(index) (const uint8_t[]){1,4,7,6,5}[index]

// indicates BLE advertising
#define PIN_LED_ADVERTISING                 ATOLED1_XPRO_LED1

// indicates BLE connected
// active high
#define PIN_LED_CONNECTED                   28

#define PIN_LED_RECEIVE                     ATOLED1_XPRO_LED2

// indicates measurement cycle progressing
#define PIN_LED_MEASUREMENT_CYCLE_COMPLETE  ATOLED1_XPRO_LED3

#define PIN_DEBUG_MEASUREMENT_INTERVAL      30

#endif // PINOUT_H
