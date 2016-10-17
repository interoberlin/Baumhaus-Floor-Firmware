/**
 * Main firmware for all the floor sensor controllers
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ble_uart.h"
#include "bluetooth.h"

#include "clock.h"

#include "floor.h"

#define PIN_LED 28

char* send_string = "";
uint8_t send_length = 0;

#ifdef FLOOR_H

/**
 * This method is invoked,
 * when five sensors have been measured.
 */
void on_measurement_cycle_complete(volatile uint16_t* sensor_values)
{
    // TODO
}

/**
 * This method is invoked,
 * when a device connects to this microcontroller.
 */
void on_ble_connected()
{
    nrf_gpio_pin_set(PIN_LED);
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
    measurement_timer_disable();

    nrf_gpio_pin_clear(PIN_LED);
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


#ifdef CLOCK_H

/**
 * Make sure, the high frequency clock is started
 */
void init_hfclock()
{
    // configure 16MHz crystal frequency
    CLOCK_XTALFREQ = 0xFF;

    // according to the Reference Manual the RADIO requires the crystal as clock source
    CLOCK_HFCLKSTAT = 1;

    // start high frequency clock
    if (!CLOCK_EVENT_HFCLKSTARTED)
    {
        CLOCK_TASK_HFCLKSTART = 1;
        while (!CLOCK_EVENT_HFCLKSTARTED)
            asm("nop");
    }
}

#endif // #ifdef CLOCK_H

/**
 * Main firmware loop
 */
int main(void)
{
    printf("Sup'\n");

    nrf_gpio_cfg_output(PIN_LED);
    nrf_gpio_pin_clear(PIN_LED);

    //init_hfclock();
    ble_init();

    init_measurement();

    // infinite loop
	while (true)
	{
//        asm("wfi");
	    nrf_delay_ms(1000);
	    if (send_length > 0)
	    {
	        ble_attempt_to_send(send_string, send_length);
	    }
	}
}
