/**
 * Main firmware for all the floor sensor controllers
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//#include "ble_uart.h"
//#include "bluetooth.h"

#include "floor.h"
#include "pinout.h"

/**
 * Prepare LED for measurement indication
 */
void init_led()
{
    nrf_gpio_cfg_output(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
    // active high
    nrf_gpio_pin_clear(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
}

#ifdef FLOOR_H

/**
 * This method is invoked, whenever
 * all sensors have been measured once.
 */
void on_measurement_cycle_complete(volatile uint16_t sensor_values[])
{
    // for comparison, in order to detect pulse count changes, e.g. a person
    static uint16_t previous_sensor_values[SENSOR_COUNT];
    const uint16_t threshold = 15;

    bool detected = false;
    for (uint8_t i=0; i<SENSOR_COUNT; i++)
    {
        if (sensor_values[i] < 42)
            continue;

        if (sensor_values[i] < previous_sensor_values[i]
         && previous_sensor_values[i] - sensor_values[i] > threshold)
        {
            // damp bias in order to adapt to permanent pulse count changes,
            // e.g. obstacles permanently present on the sensor area
            previous_sensor_values[i] -= 8;

            detected = true;
            break;
        }
        else
        {
            // object left sensor area; pulse count returns to base level
            if (sensor_values[i] > previous_sensor_values[i])
                previous_sensor_values[i] = sensor_values[i];
        }
    }

    if (detected)
        nrf_gpio_pin_set(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
    else
        nrf_gpio_pin_clear(PIN_LED_MEASUREMENT_CYCLE_COMPLETE);
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
//    ble_init();
    init_measurement();
    measurement_timer_enable();

    // infinite loop
    while (true)
    {
        asm("wfi");
    }
}
