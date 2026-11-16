/*
 * buzzer — passive buzzer on one LEDC timer + channel, non-blocking notes
 *
 * Reusable driver: pin, LEDC resources and duty come in through buzzer_config_t.
 */
#pragma once

#include <stdint.h>

#include <driver/ledc.h>

typedef struct {
    int              gpio;
    ledc_mode_t      mode;
    ledc_timer_t     timer;         // must not share the servo's 50 Hz timer
    ledc_channel_t   channel;
    ledc_timer_bit_t duty_res;      // e.g. LEDC_TIMER_10_BIT
    uint32_t         duty;          // duty while a note plays (512 = 50 % of 10 bit)
    uint32_t         init_freq_hz;  // timer frequency set at init
} buzzer_config_t;

typedef struct {
    buzzer_config_t cfg;
} buzzer_t;

/* Configures the timer and channel; the buzzer starts silent */
void buzzer_init(buzzer_t *bz, const buzzer_config_t *cfg);

/* Emit play_note for the next `cycles` superloop cycles
 * Non-blocking version of play_note() from lesson 3.4: starts the tone and
 * returns immediately; buzzer_update() stops it later. */
void buzzer_emit_note(buzzer_t *bz, int32_t freq_hz, int32_t cycles);

/* Called once per cycle: counts the note down and silences the buzzer at 0 */
void buzzer_update(buzzer_t *bz);
