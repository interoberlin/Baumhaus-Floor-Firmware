/**
 * Main firmware for all the floor sensor controllers
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ble_uart.h"
#include "bluetooth.h"

#include "floor.h"
#include "pinout.h"

/**
 * Prepare LED for measurement indication
 */
void init_led()
{
    nrf_gpio_cfg_output(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
    // active low
    nrf_gpio_pin_set(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
}

#ifdef FLOOR_H

/**
 * This method is invoked,
 * when five sensors have been measured.
 */
void on_measurement_cycle_complete(volatile uint16_t* sensor_values)
{
    // TODO

    // active low
    nrf_gpio_pin_toggle(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
}

/**
 * This method is invoked,
 * when a device connects to this microcontroller.
 */
void on_ble_connected()
{
    // begin measurement cycle
    select_first_sensor();
    measurement_timer_enable();
}

/**
 * This method is invoked,
 * when a device disconnects from this microcontroller
 * or the microcontroller drops a connection.
 */
void on_ble_disconnected()
{
    // stop measuring the sensors
    measurement_timer_disable();
}

/**
 * Prepare timers and counters
 * for the measurement
 */
void init_measurement()
{
    configure_pulse_counter();
    configure_measurement_timer();
}

#endif // #ifdef FLOOR_H


/**
 * Main firmware loop
 */
int main(void)
{
    printf("Sup'\n");

    init_led();
    ble_init();
    init_measurement();

    // infinite loop
	while (true)
	{
        asm("wfi");
	}
}
