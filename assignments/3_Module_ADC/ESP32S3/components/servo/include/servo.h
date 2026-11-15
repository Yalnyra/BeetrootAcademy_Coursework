/*
 * servo — hobby servo on one LEDC timer + channel (50 Hz PWM)
 *
 * Reusable driver: pin, LEDC resources, pulse range and travel limits all
 * come in through servo_config_t. The driver works in degrees only; it does
 * not know about encoders, ticks or step sizes.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <driver/ledc.h>

typedef struct {
    int              gpio;              // servo signal pin
    ledc_mode_t      mode;
    ledc_timer_t     timer;             // needs its own timer: 50 Hz is not shared
    ledc_channel_t   channel;
    ledc_timer_bit_t duty_res;          // e.g. LEDC_TIMER_13_BIT
    uint32_t         freq_hz;           // pwm update frequency, 50 Hz
    uint32_t         min_pulse_us;      // pulse at 0 deg
    uint32_t         max_pulse_us;      // pulse at range_deg
    int32_t          range_deg;         // physical range: 0..range_deg maps to min..max pulse
    int32_t          min_deg;           // travel limits: trying to go past
    int32_t          max_deg;           // these triggers the beep
    int32_t          init_deg;          // position driven by servo_init()
} servo_config_t;

/* Структура для логування PWM сигналу серво */
typedef struct {
    float    angle_deg;             // current angle set by the encoder
    int32_t  achieved_angle_deg;    // clamped servo-space angle actually commanded
    int32_t  duty_ticks;            // raw LEDC duty value written
    float    pulse_width_ms;        // same duty, expressed as a pulse width
    float    angle_dev_pcnt;        // angle deviation from pot space to angle space
    float    deviation_deg;         // angle_deg - achieved_angle_deg (quantization error)
    int32_t  buzzer_error_flag;
} servo_status_t;

typedef struct {
    servo_config_t cfg;
    servo_status_t status;          // last commanded position, see servo_set_angle()
    uint32_t       period_us;       // derived from cfg in servo_init()
    uint32_t       max_duty;
    uint32_t       min_ticks;       // pulse widths expressed as duty ticks
    uint32_t       max_ticks;
} servo_t;

void servo_init(servo_t *s, const servo_config_t *cfg);

/* Command the servo to cmd_angle_deg (updates s->status, writes the LEDC duty).
 * Returns true if the duty was written; false if the angle did not change
 * or writing failed. */
bool servo_set_angle(servo_t *s, int32_t cmd_angle_deg);
