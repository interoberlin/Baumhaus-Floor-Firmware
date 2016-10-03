
#include "floor.h"

// index of the currently measured sensor
uint8_t current_sensor = 0;

// the index of the last sensor
const uint8_t last_sensor = 5;

// timer overflow values
const uint32_t measurement_duration = 10;
const uint32_t measurement_interval = 20;

/**
 * Set the currently selected sensor to be the first one
 */
void select_first_sensor()
{
    current_sensor = 1;
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
 * Send the JSON string generated above
 * to the currently connected BLE device
 */
void ble_send_json()
{
    // TODO
}

/**
 * Use Programmable Peripheral Interconnect channels 0 and 1
 * to connect rising edges on a pin to pulse counter input
 */
void configure_pin_for_counting(uint8_t pin)
{
    // GPIOTE setup
    nrf_gpio_cfg_input(INPUT_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_sense_input(INPUT_PIN_NUMBER, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpiote_event_config(0, INPUT_PIN_NUMBER, NRF_GPIOTE_POLARITY_LOTOHI);

    // PPI setup:
    // Pin toggle -> Counter increment
    NRF_PPI->CH[0].EEP = (uint32_t) (&(NRF_GPIOTE->EVENTS_IN[0]));
    NRF_PPI->CH[0].TEP = (uint32_t) (&(PULSE_COUNTER->TASKS_COUNT));
    NRF_PPI->CHEN = (PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos);

    // PPI setup:
    // Timer "overflow" -> Capture counter value
    NRF_PPI->CH[1].EEP = (uint32_t)&TIMER_MEASUREMENT->EVENTS_COMPARE[0];
    NRF_PPI->CH[1].TEP = (uint32_t)&PULSE_COUNTER->TASKS_CAPTURE[0];
    NRF_PPI->CHEN |= (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos);
}

/**
 * Prepare a counter peripheral
 * for usage as pulse counter
 */
void configure_pulse_counter()
{
    // Set the timer to Counter Mode
    PULSE_COUNTER->MODE = TIMER_MODE_MODE_Counter;
    // Set timer bit resolution
    PULSE_COUNTER->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    // No shortcuts
    PULSE_COUNTER->SHORTS = 0;
}

void restart_pulse_counter()
{
    // Clear counter value
    PULSE_COUNTER->TASKS_CLEAR = 1;
    // Enable counting
    PULSE_COUNTER->TASKS_START = 1;
}

void stop_pulse_counter()
{
    // Stop counting
    PULSE_COUNTER->TASKS_STOP = 1;
}

void get_pulse_count()
{
    // request counter value to register
    //PULSE_COUNTER->TASKS_CAPTURE[0] = 1; // moved to PPI
    uint32_t counter_value = PULSE_COUNTER->CC[0];
}

/**
 * Prepare a timer peripheral
 * to provide an interrupt,
 * when 
 */
void configure_measurement_timer()
{
    // Set the timer to Counter Mode
    TIMER_MEASUREMENT->MODE = TIMER_MODE_MODE_Timer;
    // Set timer bit resolution
    TIMER_MEASUREMENT->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    // Set prescaler: 0-9. Higher number gives slower timer. 0 gives 16MHz timer.
    TIMER_MEASUREMENT->PRESCALER = 3;
    // No shortcuts
    TIMER_MEASUREMENT->SHORTS = 0;
    // clear the task first to be usable for later
    TIMER_MEASUREMENT->TASKS_CLEAR = 1;

    // Set timer compare values
    TIMER_MEASUREMENT->CC[0] = measurement_duration;
    TIMER_MEASUREMENT->CC[1] = measurement_interval;

    // Enable interrupts
    TIMER_MEASUREMENT->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
    TIMER_MEASUREMENT->INTENSET = (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);

#ifdef FLOOR_USES_TIMER0
    NVIC_EnableIRQ(TIMER0_IRQn);

#elif FLOOR_USES_TIMER1
    NVIC_EnableIRQ(TIMER1_IRQn);

#elif FLOOR_USES_TIMER2
    NVIC_EnableIRQ(TIMER2_IRQn);
#endif
}

void measurement_timer_enable()
{
    // Reset
    TIMER_MEASUREMENT->TASKS_CLEAR = 1;
    // Start timer
    TIMER_MEASUREMENT->TASKS_START = 1;
}

void measurement_timer_disable()
{
    // Stop timer
    TIMER_MEASUREMENT->TASKS_STOP = 1;
}

/**
 * Interrupts Service Routine (ISR) for the timer,
 * which we use to determine measurement duration
 * and measurement interval
 */
#ifdef FLOOR_USES_TIMER0
void TIMER0_IRQHandler()

#elif FLOOR_USES_TIMER1
void TIMER1_IRQHandler()

#elif FLOOR_USES_TIMER2
void TIMER2_IRQHandler()
#endif
{
    // counter value captured
    if (TIMER_MEASUREMENT->EVENTS_COMPARE[0]                           // compare channel 0 event
    && (TIMER_MEASUREMENT->INTENSET & TIMER_INTENSET_COMPARE0_Msk))    // channel 0 is enabled
    {
        // clear this event
        TIMER_MEASUREMENT->EVENTS_COMPARE[0] = 0;

        isr_measurement_complete();
    }

    // measurement interval
    else if (TIMER_MEASUREMENT->EVENTS_COMPARE[1]                      // compare channel 1 event
    && (TIMER_MEASUREMENT->INTENSET & TIMER_INTENSET_COMPARE1_Msk))    // channel 1 is enabled
    {
        // clear this event
        TIMER_MEASUREMENT->EVENTS_COMPARE[1] = 0;

        isr_measurement_complete();

        // clear and restart counter
        PULSE_COUNTER->TASKS_CLEAR = 1;
        //PULSE_COUNTER->TASKS_START = 1;
    }
}
