/**
 * Baumhaus tree demo firmware
 *
 * by Thee Vanichangkul, Matthias Bock and Long Pham
 */

#include <stdbool.h>
#include <stdint.h>

#include "nrf51.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "neopixel.h"
#include "../sdk/nrfduino.h"
#include "../sdk/clock.h"

// LED strips
#define NUM_STRIPS      1
#define TOTAL_NUM_LEDS  5
neopixel_strip_t strip[NUM_STRIPS];
const uint8_t strip_at_pin[NUM_STRIPS]   = {NRFDUINO_PIN_A0}; //, 28, 2, 0};
const uint8_t leds_per_strip[NUM_STRIPS] = {5};
volatile bool strip_changed[NUM_STRIPS]  = {false}; //, false, false, false};

// get the number of the strip from LED index
const uint8_t led_to_strip[TOTAL_NUM_LEDS]          = {1, 1, 1, 1, 1};
const uint8_t led_index_on_strip[TOTAL_NUM_LEDS]    = {0, 1, 2, 3, 4};


// workaround:
// static memory allocation because malloc is currently not working
#define LED_MEM "\0\0\0" "\0\0\0\0\0\0\0\0"
//#define LED_MEM "\xff\xff\xff" "\xff\xff\xff\xff\xff\xff\xff\xff"
extern uint32_t heap_begin;
uint8_t led_memory[TOTAL_NUM_LEDS * 3];

uint32_t my_mem;

/**
 * @brief Initializes all LED strips
 */
void init_ledstrips()
{
    //neopixel_init(&(strip[0]), 30, 5);

    // initialize with zeroes
//    led_memory = heap_begin; //LED_MEM LED_MEM LED_MEM LED_MEM LED_MEM;
    strip[0].leds = led_memory; //0x20003600; //heap_begin+2000;
    //return;

    for (int strip_num=0; strip_num<NUM_STRIPS; strip_num++)
    {
        neopixel_init(&strip[strip_num], strip_at_pin[strip_num], leds_per_strip[strip_num]);
        //neopixel_clear(&strip[strip_num]);
    }
}

/**
 * @brief Calculates new intensity values for all LEDs, i.e. a new "frame"
 */
void calculate_new_led_values()
{
    uint8_t current_strip = 0;

    for (uint8_t led=0; led<TOTAL_NUM_LEDS; led++)
    {
        uint8_t warmwhite, coldwhite, amber;
        neopixel_get_color(&strip[current_strip], led, &warmwhite, &coldwhite, &amber);
        warmwhite = ((uint32_t) warmwhite + 1) % 256;
        coldwhite = ((uint32_t) coldwhite + 1) % 256;
        amber = ((uint32_t) amber + 1) % 256;
        neopixel_set_color(&strip[current_strip], led, warmwhite, coldwhite, amber);
    }
    strip_changed[current_strip] = true;
}

/**
 * @brief Configures the FPS timer
 */
void setup_fps_timer(void)
{
    // Set the timer to Counter Mode
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;
    // clear the task first to be usable for later
    NRF_TIMER2->TASKS_CLEAR = 1;
    // No shortcuts
    NRF_TIMER2->SHORTS = 0;
    /*
     * Set timing frequency via prescaler: 0-9
     * Higher numbers result in lower lower frequency
     *
     * f = 16 MHz / (2**prescaler)
     *
     * 0 => 16 MHz
     * 6 => 250 kHz
     */
    NRF_TIMER2->PRESCALER = 6;
    // Set timer bit resolution
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    // Set timer compare values
    NRF_TIMER2->CC[0] = 1;

    // Enable interrupt on Timer 2, both for CC[0] and CC[1] compare match events
    NRF_TIMER2->INTENSET =
            (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
          //| (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);
    NVIC_EnableIRQ(TIMER2_IRQn);

    // Start TIMER2
    NRF_TIMER2->TASKS_START = 1;
}


/**
 * @brief FPS Timer ISR
 */
void TIMER2_Handler()
{
    // COMPARE0 event occured ?
    if (NRF_TIMER2->EVENTS_COMPARE[0] && (NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk))
    {
        // clear event
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;

        nrf_gpio_pin_set(NRFDUINO_PIN_LED);

        // update all the strips
        // if they have changed
        for (int i=0; i<NUM_STRIPS; i++)
        {
            if (strip_changed[i])
            {
                neopixel_show(&strip[i]);
                strip_changed[i] = false;
            }
        }

        neopixel_set_color_and_show(&strip[0], 2, 30, 100, 0);

        calculate_new_led_values();

        nrf_gpio_pin_clear(NRFDUINO_PIN_LED);
    }
}


/**
 * @brief Firmware entry point
 */
int main(void)
{
    nrf_gpio_cfg_output(NRFDUINO_PIN_LED);
    nrf_gpio_pin_set(NRFDUINO_PIN_LED);
    nrf_delay_ms(100);

    init_ledstrips();

//    setup_fps_timer();

    // infinite loop
	while(true)
	{
	    nrf_gpio_pin_set(NRFDUINO_PIN_LED);

	    uint8_t i;
	    for (i=0; i<255; i++)
	    {
	        neopixel_set_color_and_show(&(strip[0]), 0, i, i, i);
	        nrf_delay_ms(5);
	    }

	    nrf_gpio_pin_clear(NRFDUINO_PIN_LED);

	    for (i=255; i>0; i--)
        {
            neopixel_set_color_and_show(&(strip[0]), 0, i, i, i);
            nrf_delay_ms(5);
        }
	}
}
