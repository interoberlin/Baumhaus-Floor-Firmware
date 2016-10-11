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

#define LED_PIN 28

/**
 * This method is invoked
 * once every measurement interval.
 * It configures and starts a measurement.
 */
void on_measurement_start()
{
    select_next_sensor();
    restart_pulse_counter();
}

/**
 * This method is invoked,
 * when the measurement time has been elapsed.
 * It stops the pulse counter and evaluates the result.
 */
void on_measurement_complete()
{
    stop_pulse_counter();
    get_pulse_count();

    if (is_last_sensor())
    {
        generate_json();
        // TODO:
        //ble_send_json();
    }
}

/**
 * This method is invoked,
 * when a device connects to this microcontroller.
 */
void on_ble_connected()
{
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
}
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

/**
 * Prepare timers and counters
 * for the measurement
 */
void init_measurement()
{
    configure_pulse_counter();
    configure_measurement_timer();
    set_handler_measurement_interval(&on_measurement_start);
    set_handler_measurement_complete(&on_measurement_complete);
}

/**
 * Main firmware loop
 */
int main(void)
{
    printf("Sup\n");

    //init_hfclock();
    ble_init();

    printf("BLE init ferdsch\n");

    init_measurement();

    select_first_sensor();
    measurement_timer_enable();

    // infinite loop
	while(true)
	{
	 //   asm("wfi"); // sleep: wait for interrupt
	    __WFI();
	    __SEV();
	    __WFE();
	}
}
