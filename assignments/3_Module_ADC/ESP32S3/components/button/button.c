#include "button.h"

#include <esp_err.h>

void button_init(button_t *btn, const button_config_t *cfg)
{
    btn->cfg                 = *cfg;
    btn->button_state        = IDLE;
    btn->button_press_time   = 0;
    btn->button_release_time = 0;
    btn->state_entered_at    = 0;
    btn->long_fired          = false;

    gpio_config_t io = {
        .pin_bit_mask = 1ULL << cfg->gpio,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,      // polled from the superloop, no ISR
    };
    ESP_ERROR_CHECK(gpio_config(&io));
}

// Polling + debounce (no interrupts). pressed = true when pin is LOW
button_event_t button_poll(button_t *btn, uint32_t ts, bool pressed)
{
    button_event_t evt = BTN_EVT_NONE;

    switch (btn->button_state) {
    case IDLE:
        if (pressed) {
            btn->button_state     = DEBOUNCE_PRESS;
            btn->state_entered_at = ts;
        }
        break;

    case DEBOUNCE_PRESS:
        if (!pressed) {
            btn->button_state = IDLE;                     // just a glitch
        } else if (ts - btn->state_entered_at >= btn->cfg.debounce_ms) {
            btn->button_state      = HELD;                // real press
            btn->button_press_time = ts;
            btn->long_fired        = false;
        }
        break;

    case HELD:
        if (!pressed) {
            btn->button_state     = DEBOUNCE_RELEASE;
            btn->state_entered_at = ts;
        } else if (!btn->long_fired &&
                   ts - btn->button_press_time > btn->cfg.long_press_ms) {
            btn->long_fired = true;                       // fire once, while still held
            evt = BTN_EVT_LONG;
        }
        break;

    case DEBOUNCE_RELEASE:
        if (pressed) {
            btn->button_state = HELD;                     // bounce, press continues
        } else if (ts - btn->state_entered_at >= btn->cfg.debounce_ms) {
            btn->button_state        = IDLE;              // real release
            btn->button_release_time = ts;
            if (!btn->long_fired) {
                evt = BTN_EVT_SHORT;                      // no long event this press
            }
        }
        break;
    }
    return evt;
}
