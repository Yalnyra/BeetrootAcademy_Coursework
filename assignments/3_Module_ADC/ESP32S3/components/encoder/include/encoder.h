/*
 * encoder — incremental rotary encoder on the ESP32 PCNT peripheral
 *
 * Reusable driver: no pin numbers or project settings inside, everything
 * comes in through encoder_config_t. Several encoders can run at once,
 * each with its own encoder_t handle.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <driver/gpio.h>
#include <driver/pulse_cnt.h>

typedef struct {
    gpio_num_t gpio_a;              // CLK: counted edges
    gpio_num_t gpio_b;              // DT:  direction level
    gpio_num_t debug_gpio;          // scope/LED probe for encoder_output_ticks()

    /* Decoding mode:
     *   false = X1: one channel, rising edge of A only (lesson 3.6). 1 count per click.
     *   true  = X4: two channels, both edges of A and B. 4 counts per click,
     *               bounce cancels in +1/-1 pairs. */
    bool       quadrature_x4;
    int        counts_per_detent;   // PCNT counts per mechanical click (X1: 1, X4: 4)
    uint32_t   glitch_filter_ns;    // PCNT glitch filter: shorter pulses are ignored
    int        high_limit;          // PCNT counter limits
    int        low_limit;
} encoder_config_t;

typedef struct {
    encoder_config_t   cfg;
    pcnt_unit_handle_t unit;            // Дескриптор апаратного лічильника імпульсів (PCNT)
    int                prev_count;      // encoder_update_ticks(): count at the previous call
    int32_t            residual;        // X4: edges not yet adding up to a full click
    int                debug_prev_count;// encoder_output_ticks(): its own previous count
} encoder_t;

/* Ініціалізація апаратного лічильника для енкодера (з lesson 3.6)
 * Режим X1 або X4 обирається через cfg->quadrature_x4 */
void encoder_init(encoder_t *enc, const encoder_config_t *cfg);

/* Debug output pin for encoder_output_ticks(): plain push-pull output, starts LOW */
void encoder_debug_init(encoder_t *enc);

/* Returns the click delta since the previous call (+ CW / - CCW).
 * X1: 1 count = 1 click, the count delta is returned as is.
 * X4: counts_per_detent counts = 1 click; leftover edges are kept for the next call. */
int32_t encoder_update_ticks(encoder_t *enc);

/* Debug probe: mirrors the glitch-filtered PCNT count on cfg.debug_gpio.
 * See encoder.c for details. */
void encoder_output_ticks(encoder_t *enc);
