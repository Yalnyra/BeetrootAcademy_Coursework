/*
 * button — polled push button (no ISR), active LOW with internal pull-up
 *
 * Reusable driver: the pin comes in through button_config_t.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include <driver/gpio.h>

typedef struct {
    gpio_num_t gpio;            // shorts to GND when pressed
    uint32_t   debounce_ms;     // level must be stable this long to count
    uint32_t   long_press_ms;   // held longer than this -> BTN_EVT_LONG
} button_config_t;


typedef enum { IDLE, DEBOUNCE_PRESS, HELD, DEBOUNCE_RELEASE } button_state_t;
typedef enum { BTN_EVT_NONE, BTN_EVT_SHORT, BTN_EVT_LONG } button_event_t;

typedef struct {
    button_config_t cfg;
    button_state_t  button_state;
    uint32_t        button_press_time;    // set on DEBOUNCE_PRESS -> HELD
    uint32_t        button_release_time;  // set on DEBOUNCE_RELEASE -> IDLE
    uint32_t        state_entered_at;     // debounce timer
    bool            long_fired;           // long action already ran during this press
} button_t;



/* Polled input with pull-up, SW shorts to GND when pressed */
void button_init(button_t *btn, const button_config_t *cfg);

/* step through button state machine based on external signal
 * ts:      current time in ms
 * pressed: true while the pin reads LOW
 * Returns BTN_EVT_LONG once while still held past cfg.long_press_ms,
 * BTN_EVT_SHORT on release if no long event fired during that press,
 * BTN_EVT_NONE otherwise. What each event does is up to the caller. */
button_event_t button_poll(button_t *btn, const uint32_t ts, bool pressed);

/* Return a human-readable name for a button_event_t value. */
const char *button_event_to_str(button_event_t evt);

