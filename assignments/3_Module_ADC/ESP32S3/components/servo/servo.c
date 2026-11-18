#include "servo.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

#include <esp_err.h>
#include <esp_log.h>

static const char *TAG = "servo";

#define ROUND_UP_CORRECTION 0.5f

/* Helpers */
static bool    servo_update_status(servo_t *s, int32_t cmd_angle_deg);
static int32_t servo_angle_to_duty_ticks(const servo_t *s, int32_t angle_deg);
static float   duty_to_servo_angle_deg(const servo_t *s, uint32_t duty);
static float   duty_to_pulse_width_ms(const servo_t *s, int32_t duty_ticks);
static float   calc_deviation_percentf(float one, float other);
static inline int32_t val_clamp(int32_t value, int32_t val_min, int32_t val_max);
static inline int32_t map_value(int32_t value, int32_t in_min, int32_t in_max,
                                int32_t out_min, int32_t out_max);
static inline float   map_valuef(float value, float in_min, float in_max,
                                 float out_min, float out_max);

void servo_init(servo_t *s, const servo_config_t *cfg)
{
    s->cfg       = *cfg;
    s->period_us = 1000000U / cfg->freq_hz;                 // 20 000 us at 50 Hz
    s->max_duty  = (1U << cfg->duty_res) - 1U;
    // Pulse widths expressed as duty ticks (rounded to nearest tick)
    s->min_ticks = (cfg->min_pulse_us * s->max_duty + s->period_us / 2U) / s->period_us;
    s->max_ticks = (cfg->max_pulse_us * s->max_duty + s->period_us / 2U) / s->period_us;

    ledc_timer_config_t timer_config = {
        .speed_mode      = cfg->mode,
        .timer_num       = cfg->timer,
        .duty_resolution = cfg->duty_res,
        .freq_hz         = cfg->freq_hz,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .speed_mode = cfg->mode,
        .channel    = cfg->channel,
        .timer_sel  = cfg->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = cfg->gpio,
        .duty       = servo_angle_to_duty_ticks(s, cfg->init_deg),
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

bool servo_set_angle(servo_t *s, int32_t cmd_angle_deg)
{
    if (s == NULL) { ESP_LOGE(TAG, "servo struct init error: servo is null pointer"); return false; }

    if (!servo_update_status(s, cmd_angle_deg)) {  ESP_LOGE(TAG, "status update error, current angle = " PRId32 ", cmd = " PRId32 " deg",  s->status.angle_deg, cmd_angle_deg); return false; }

    //ESP_LOGI(*TAG, "servo idle, at %d deg", s->status.achieved_angle_deg
    // Requires duty cycle, not time ms
    esp_err_t set_err = ledc_set_duty(s->cfg.mode, s->cfg.channel, s->status.duty_ticks);
    return set_err == ESP_OK && ledc_update_duty(s->cfg.mode, s->cfg.channel) == ESP_OK;
}

bool servo_reset_angle(servo_t *s)
{
    if (s == NULL) { ESP_LOGE(TAG, "servo struct init error: servo is null pointer"); return false; }
    return servo_set_angle(s, s->cfg.init_deg);
}

/* ============================ Helpers ============================= */

static bool servo_update_status(servo_t *s, int32_t cmd_angle_deg)
{
    servo_status_t *status = &s->status;
    int32_t achieved_angle_deg =  val_clamp(cmd_angle_deg, s->cfg.min_deg, s->cfg.max_deg);
    int32_t deviation_deg = cmd_angle_deg - achieved_angle_deg; 
    bool buzzer_error_flag = (status->deviation_deg != 0); // was inverted (abs(...) == 0)
    status->deviation_deg = deviation_deg; // != 0: tried to turn past a limit
    status->buzzer_error_flag = buzzer_error_flag;  
    if (buzzer_error_flag) return false; // was the angle clamped?  
    status->angle_deg = achieved_angle_deg;

    int32_t duty_ticks = servo_angle_to_duty_ticks(s, achieved_angle_deg);
    status->duty_ticks     = servo_angle_to_duty_ticks(s, achieved_angle_deg);
    status->pulse_width_ms = duty_to_pulse_width_ms(s, duty_ticks);
    status->angle_dev_pcnt = calc_deviation_percentf(cmd_angle_deg, achieved_angle_deg);
    return true;
}

/* Servo angle (0..range_deg) -> LEDC duty ticks */
static int32_t servo_angle_to_duty_ticks(const servo_t *s, int32_t angle_deg)
{
    // TODO:
    // Mapping to number ticks per second (0-8192)
    // float ratio = (cmd_angle_deg - min_range_deg) / (max_range_deg - min_range_deg);
    // uint32_t duty_ticks = s->min_ticks +
    // (uint32_t)(ratio * (float)(s->max_ticks - s->min_ticks) + ROUND_UP_CORRECTION);

    //Map built-in clamps to allowed degree value
    return map_value(angle_deg, 0, s->cfg.range_deg, (int32_t)s->min_ticks, (int32_t)s->max_ticks);
}

/* Relative deviation of `one` from `other`, % (from lesson 3.5) */
static float calc_deviation_percentf(float one, float other)
{
    return (other != 0.0f)
                ? (one - other) / other * 100.0f
                : 0.0f;
}

static float duty_to_pulse_width_ms(const servo_t *s, int32_t duty_ticks)
{
    return (float)duty_ticks * s->period_us / (s->max_duty + 1 )/ 1000.0f;  // (13 bit @ 50 Hz: 1 ms ~ 205 ticks);
}

static inline int32_t val_clamp(int32_t value, int32_t val_min, int32_t val_max)
{
    if (value < val_min) return val_min;
    if (value > val_max) return val_max;
    return value;
}

static inline int32_t map_value(int32_t value, int32_t in_min, int32_t in_max,
                                int32_t out_min, int32_t out_max)
{
    if (in_min == in_max) return out_max;

    value = val_clamp(value, in_min, in_max);

    return out_min + ((value - in_min) * (out_max - out_min) / (in_max - in_min) + ROUND_UP_CORRECTION);
}

static inline float map_valuef(float value, float in_min, float in_max,
                               float out_min, float out_max)
{
    if (in_min == in_max) return out_max;

    if (value < in_min) value = in_min;
    if (value > in_max) value = in_max;

    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}
