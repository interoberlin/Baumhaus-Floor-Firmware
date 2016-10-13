
#include "floor.h"


// index of the currently measured sensor
uint8_t current_sensor = 0;

// local pointers to event handlers
handler_t handler_measurement_complete = 0;
handler_t handler_measurement_interval = 0;


/**
 * Use GPIO Tasks and Events (GPIOTE) channel 0
 * to generate an event, when the selected pin encounters a rising edge
 */
void configure_pin_for_counting(uint8_t pin)
{
    nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpiote_event_config(0, pin, NRF_GPIOTE_POLARITY_LOTOHI);
}

/**
 * Set the currently selected sensor to be the first one
 */
void select_first_sensor()
{
    current_sensor = 1;
    configure_pin_for_counting(sensor_pin(0));
}

/**
 * Select the next sensor to be measured
 */
void select_next_sensor()
{
    current_sensor++;

    if (is_last_sensor())
    {
        select_first_sensor();
    }
    else
    {
        configure_pin_for_counting(sensor_pin(current_sensor));
    }
}

/**
 * Determine, whether the current sensor
 * is the last sensor
 */
bool is_last_sensor()
{
    return (current_sensor >= last_sensor);
}

/**
 * Generate a JSON string,
 * which contains the values of all attached sensors
 */
void generate_json()
{
    // TODO
}

/**
 * Prepare a counter peripheral for usage as pulse counter and
 * use Programmable Peripheral Interconnect (PPI) channels 0 and 1
 * to connect rising edges on a pin to pulse counter input
 */
void configure_pulse_counter()
{
    // set the timer to counter mode
    PULSE_COUNTER->MODE = TIMER_MODE_MODE_Counter;
    // set timer bit resolution
    PULSE_COUNTER->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    // no shortcuts
    PULSE_COUNTER->SHORTS = 0;

    // PPI setup:
    // pin toggled => increment counter
    NRF_PPI->CH[0].EEP = (uint32_t) (&(NRF_GPIOTE->EVENTS_IN[0]));
    NRF_PPI->CH[0].TEP = (uint32_t) (&(PULSE_COUNTER->TASKS_COUNT));
    NRF_PPI->CHEN = (PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos);

    // PPI setup:
    // measurement complete => capture counter value
    NRF_PPI->CH[1].EEP = (uint32_t)&TIMER_MEASUREMENT->EVENTS_COMPARE[0];
    NRF_PPI->CH[1].TEP = (uint32_t)&PULSE_COUNTER->TASKS_CAPTURE[0];
    NRF_PPI->CHEN |= (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos);
}

void restart_pulse_counter()
{
    // clear counter value
    PULSE_COUNTER->TASKS_CLEAR = 1;
    // enable counting
    PULSE_COUNTER->TASKS_START = 1;
}

void stop_pulse_counter()
{
    // stop counting
    PULSE_COUNTER->TASKS_STOP = 1;
}

uint32_t get_pulse_count()
{
    // request counter value to register
    //PULSE_COUNTER->TASKS_CAPTURE[0] = 1; // value capturing has been moved to PPI
    return PULSE_COUNTER->CC[0];
}

/**
 * Prepare a timer peripheral
 * to throw an interrupt,
 * both when a measurement shall be started
 * and when it shall be stopped
 */
void configure_measurement_timer()
{
    // set the timer to Counter Mode
    TIMER_MEASUREMENT->MODE = TIMER_MODE_MODE_Timer;
    // set timer bit resolution
    TIMER_MEASUREMENT->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    /*
     * set prescaler: 0-9
     * fTIMER = 16 MHz / (2**PRESCALER)
     *
     * 16 MHz / 2**3 = 2 MHz
     * 16 MHz / 2**9 = 31250 Hz
     * 16 MHz / 2**7 = 125 kHz
     * 16 MHz / 2**6 = 250 kHz
     */
    TIMER_MEASUREMENT->PRESCALER = 6;
    // no shortcuts
    TIMER_MEASUREMENT->SHORTS = 0;
    // clear the task first to be usable for later
    TIMER_MEASUREMENT->TASKS_CLEAR = 1;

    // set timer compare values
    TIMER_MEASUREMENT->CC[0] = measurement_duration;
    TIMER_MEASUREMENT->CC[1] = measurement_interval;

    // enable interrupt upon compare event
    TIMER_MEASUREMENT->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos)
                                | (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);

    // configure debug pin as output
    #ifdef PIN_DEBUG_MEASUREMENT_INTERVAL
        nrf_gpio_cfg_output(PIN_DEBUG_MEASUREMENT_INTERVAL);
    #endif

    // enable appropriate timer interrupt
#ifdef FLOOR_USES_TIMER0
    NVIC_EnableIRQ(TIMER0_IRQn);

#elifdef FLOOR_USES_TIMER1
    NVIC_EnableIRQ(TIMER1_IRQn);

//#elifdef FLOOR_USES_TIMER2
#else
    NVIC_EnableIRQ(TIMER2_IRQn);
#endif

    // manual setting for testing
    NVIC_EnableIRQ(TIMER2_IRQn);
}

void measurement_timer_enable()
{
    // reset timer
    TIMER_MEASUREMENT->TASKS_CLEAR = 1;
    // start timer
    TIMER_MEASUREMENT->TASKS_START = 1;
}

void measurement_timer_disable()
{
    // stop timer
    TIMER_MEASUREMENT->TASKS_STOP = 1;
}

void set_handler_measurement_complete(handler_t handler)
{
    handler_measurement_complete = handler;
}

void set_handler_measurement_interval(handler_t handler)
{
    handler_measurement_interval = handler;
}

/**
 * Interrupts Service Routine (ISR) for the timer,
 * which we use to determine measurement duration
 * and measurement interval
 */
#ifdef FLOOR_USES_TIMER0
    #define TIMER_ISR() void TIMER0_IRQHandler
#elifdef FLOOR_USES_TIMER1
    #define TIMER_ISR() void TIMER1_IRQHandler
#elifdef FLOOR_USES_TIMER2
    #define TIMER_ISR() void TIMER2_IRQHandler
#else
    #define TIMER_ISR() void unused
#endif

//TIMER_ISR()()
// manual setting for testing
//void TIMER2_IRQHandler()
//{
//    /*
//     * a measurement is complete:
//     * the counter value has been captured
//     */
//    if (TIMER_MEASUREMENT->EVENTS_COMPARE[0]                           // compare channel 0 event
//    && (TIMER_MEASUREMENT->INTENSET & TIMER_INTENSET_COMPARE0_Msk))    // channel 0 is enabled
//    {
//        // clear this event
//        TIMER_MEASUREMENT->EVENTS_COMPARE[0] = 0;
//
//        // stop pulse counter
//        stop_pulse_counter();
//
//        #ifdef PIN_DEBUG_MEASUREMENT_INTERVAL
//            nrf_gpio_pin_clear(PIN_DEBUG_MEASUREMENT_INTERVAL);
//        #endif
//
//        if (handler_measurement_complete != 0)
//        {
//            (*handler_measurement_complete)();
//        }
//    }
//
//    /*
//     * once every measurement interval
//     */
//    if (TIMER_MEASUREMENT->EVENTS_COMPARE[1]                           // compare channel 1 event
//    && (TIMER_MEASUREMENT->INTENSET & TIMER_INTENSET_COMPARE1_Msk))    // channel 1 is enabled
//    {
//        // clear this event
//        TIMER_MEASUREMENT->EVENTS_COMPARE[1] = 0;
//
//        // clear/restart measurement timer
//        TIMER_MEASUREMENT->TASKS_CLEAR = 1;
//
//        // clear/restart counter
//        restart_pulse_counter();
//
//        #ifdef PIN_DEBUG_MEASUREMENT_INTERVAL
//            nrf_gpio_pin_set(PIN_DEBUG_MEASUREMENT_INTERVAL);
//        #endif
//
//        if (handler_measurement_interval != 0)
//        {
//            (*handler_measurement_interval)();
//        }
//    }
//}
