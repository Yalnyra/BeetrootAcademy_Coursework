/*
 * button — polled push button (no ISR), active LOW with internal pull-up
 *
 * Reusable driver: the pin comes in through button_config_t.
 */
#pragma once

#include <stdbool.h>

#include <driver/gpio.h>

typedef struct {
    gpio_num_t gpio;        // shorts to GND when pressed
} button_config_t;

typedef struct {
    button_config_t cfg;
} button_t;

/* Polled input with pull-up, SW shorts to GND when pressed */
void button_init(button_t *btn, const button_config_t *cfg);

/* Button event pressed? (once per press event)
 * Returns true exactly once per press; holding the button does not repeat. */
bool button_pressed_event(button_t *btn);
