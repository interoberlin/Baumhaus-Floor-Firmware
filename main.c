/**
 * Floor sensor demo firmware
 *
 * by Thee Vanichangkul, Matthias Bock and Long Pham
 */

#include <stdbool.h>
#include <stdint.h>

#include "nrf51.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"

#define LED_PIN 28
#define INPUT_PIN_NUMBER 0
#define INDICATION_PIN 29

volatile uint32_t cycle_count = 0;
#define averaging_cycles 200
volatile uint32_t average_pulse_count = 0;

#define pulse_count_margin 5

#define counter     NRF_TIMER1
#define timer_1ms   NRF_TIMER2
#define timer_irq   TIMER2_IRQn

/**
 * This function configures the timers we require
 */
void setup_timers(void)
{
    // Set the timer to Counter Mode
    counter->MODE = TIMER_MODE_MODE_Counter;
    // clear the task first to be usable for later
    counter->TASKS_CLEAR = 1;
    // No shortcuts
    counter->SHORTS = 0;
    // Set timer bit resolution
    counter->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    // Enable counter
    counter->TASKS_START = 1;

    // Set the timer to Counter Mode
    timer_1ms->MODE = TIMER_MODE_MODE_Timer;
    // clear the task first to be usable for later
    timer_1ms->TASKS_CLEAR = 1;
    // No shortcuts
    timer_1ms->SHORTS = 0;
    // Set prescaler: 0-9. Higher number gives slower timer. 0 gives 16MHz timer.
    timer_1ms->PRESCALER = 3;
    // Set timer bit resolution
    timer_1ms->BITMODE = TIMER_BITMODE_BITMODE_16Bit;

    // Set timer compare values
    timer_1ms->CC[0] = 20000;
    // Enable interrupt
    timer_1ms->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
    NVIC_EnableIRQ(timer_irq);

    // Start timer
    timer_1ms->TASKS_START = 1;
}

/**
 * Interrupts Service Routine (ISR) for the timer
 */
void TIMER2_IRQHandler()
{
    if (timer_1ms->EVENTS_COMPARE[0] && (timer_1ms->INTENSET & TIMER_INTENSET_COMPARE0_Msk))
    {
        // stop counter
        //counter->TASKS_STOP = 1;

        // clear this event
        timer_1ms->EVENTS_COMPARE[0] = 0;
        // restart timer
        timer_1ms->TASKS_CLEAR = 1;

        // request counter value to register
        //counter->TASKS_CAPTURE[0] = 1; // moved to PPI
        uint32_t counter_value = counter->CC[0];
        //printf("%d\n", counter_value);

        // still averaging?
        if (cycle_count < 65535) // prevent overflow
            cycle_count++;
        if (cycle_count == averaging_cycles)
        {
            nrf_gpio_pin_clear(LED_PIN);
            printf("%d\n", average_pulse_count);
        }
        if (cycle_count < averaging_cycles)
        {
            average_pulse_count = (average_pulse_count + counter_value) / 2;
        }
        else
        {
            if (counter_value < average_pulse_count - pulse_count_margin)
            {
                nrf_gpio_pin_set(LED_PIN);
                nrf_gpio_pin_set(INDICATION_PIN);
            }
            else if (counter_value >= average_pulse_count)
            {
                nrf_gpio_pin_clear(LED_PIN);
                nrf_gpio_pin_clear(INDICATION_PIN);
            }

        }

        // clear and restart counter
        counter->TASKS_CLEAR = 1;
        //counter->TASKS_START = 1;
    }
}

int main(void)
{
    printf("Hello world!\nAverage pulse count: ");

    // indicate averaging by turning LED on
    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpio_pin_set(LED_PIN);

    // configure pin for detection indication
    nrf_gpio_cfg_output(INDICATION_PIN);
    nrf_gpio_pin_clear(INDICATION_PIN);

    // GPIOTE setup
    nrf_gpio_cfg_input(INPUT_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_sense_input(INPUT_PIN_NUMBER, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpiote_event_config(0, INPUT_PIN_NUMBER, NRF_GPIOTE_POLARITY_LOTOHI);

    // PPI setup:
    // Pin toggle -> Counter increment
    NRF_PPI->CH[0].EEP = (uint32_t) (&(NRF_GPIOTE->EVENTS_IN[0]));
    NRF_PPI->CH[0].TEP = (uint32_t) (&(counter->TASKS_COUNT));
    NRF_PPI->CHEN = (PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos);

    // PPI setup:
    // Timer "overflow" -> Capture counter value
    NRF_PPI->CH[1].EEP = (uint32_t)&timer_1ms->EVENTS_COMPARE[0];
    NRF_PPI->CH[1].TEP = (uint32_t)&counter->TASKS_CAPTURE[0];
    NRF_PPI->CHEN |= (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos);

    setup_timers();

    // infinite loop
	while(true)
	{
	 //   asm("wfi"); // sleep: wait for interrupt
	    __WFI();
	    __SEV();
	    __WFE();
	}
}
