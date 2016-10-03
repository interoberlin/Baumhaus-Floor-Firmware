/**
 * Main firmware for all the floor sensor controllers
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LED_PIN 28

#define FLOOR_USES_TIMER2
#define PULSE_COUNTER      NRF_TIMER1
#define TIMER_MEASUREMENT  NRF_TIMER2

#include "floor.h"


/**
 * This method is invoked
 * once every measurement interval.
 * It configures and starts a measurement.
 */
void isr_measurement_start()
{
    select_next_sensor();
    restart_pulse_counter();
}

/**
 * This method is invoked,
 * when the measurement time has been elapsed.
 * It stops the pulse counter and evaluates the result.
 */
void isr_measurement_complete()
{
    stop_pulse_counter();
    get_pulse_count();

    if (is_last_sensor())
    {
        generate_json();
        ble_send_json();
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
 * Prepare timers and counters
 * for the measurement
 */
void init_measurement()
{
    configure_pulse_counter(&isr_measurement_complete);
    
    configure_measurement_timer(&isr_measurement_start);
}

/**
 * Main firmware loop
 */
int main(void)
{
    //init_ble();
    init_measurement();

    // infinite loop
	while(true)
	{
	 //   asm("wfi"); // sleep: wait for interrupt
	    __WFI();
	    __SEV();
	    __WFE();
	}
}
