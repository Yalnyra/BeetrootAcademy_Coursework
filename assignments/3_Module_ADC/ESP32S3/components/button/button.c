#include "button.h"

#include <esp_err.h>

void button_init(button_t *btn, const button_config_t *cfg)
{
    btn->cfg = *cfg;

    gpio_config_t io = {
        .pin_bit_mask = 1ULL << cfg->gpio,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,      // polled from the superloop, no ISR
    };
    ESP_ERROR_CHECK(gpio_config(&io));
}

bool button_pressed_event(button_t *btn)
{
    // TODO:
    //  - level = gpio_get_level(btn->cfg.gpio)    (0 = pressed)
    //  - return true only on the 1 -> 0 transition; keep the previous level in a
    //    new field of button_t (not a `static` local, so two buttons don't share it)
    //  - polling every superloop cycle is slower than contact bounce, so this already debounces
    (void)btn;
    return false;
}
