#ifndef FLOOR_H
#define FLOOR_H

#include "nrf51.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"

void select_first_sensor();
void select_next_sensor();
bool is_last_sensor();

void generate_json();
void ble_send_json();

void configure_pin_for_counting();
void configure_pulse_counter();
void restart_pulse_counter();
void stop_pulse_counter();
void get_pulse_count();

void configure_measurement_timer();
void measurement_timer_enable();
void measurement_timer_disable();

#endif // FLOOR_H