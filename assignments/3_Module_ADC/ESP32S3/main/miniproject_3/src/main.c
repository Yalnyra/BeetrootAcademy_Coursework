/*
 * Mini-project 3 — Керування сервоприводом
 *
 * Технічне завдання:
 *  - Керувати сервоприводом за допомогою поворотного енкодера (обробник PCNT в ESP-IDF)
 *  - Один тік енкодера повертає вал сервоприводу на певний кут у відповідному напрямку
 *  - Натискання кнопки енкодера зменшує кут повороту вала сервоприводу удвічі на кожен тік
 *  - Повторне натискання кнопки повертає точність до більшого кута на кожен тік
 *  - Коли вал сервоприводу у крайніх положеннях, програвати короткий сигнал
 *    у разі спроби прокрутити вал за межі
 *
 * Architecture: bare superloop. No tasks or queues (no xTask*, no xQueue*) —
 * each block of the flowchart is a plain function called once per cycle from
 * app_main(). The only FreeRTOS call is vTaskDelay() at the end of the cycle,
 * which also lets the IDLE task run so the Task Watchdog stays quiet.
 *
 * Code layout:
 *   ../../../components/{encoder,button,servo,buzzer}  reusable drivers, configured via structs
 *   ../include/app_config.h                            pins and settings of this project
 *   speed.c                                            k = 2k (mod N) step-size rule
 *   main.c (this file)                                 superloop: wires the drivers together
 *
 * Wiring (ESP32-S3 DevKitC-1): see the pin #defines in app_config.h
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "app_config.h"
#include "button.h"
#include "buzzer.h"
#include "encoder.h"
#include "servo.h"
#include "speed.h"

static const char *TAG = "mini_project_3";

/* ======================= Driver configuration ====================== */

static const encoder_config_t encoder_cfg = {
    .gpio_a            = ENCODER_GPIO_A,
    .gpio_b            = ENCODER_GPIO_B,
    .debug_gpio        = ENCODER_DEBUG_GPIO,
    .quadrature_x4     = ENCODER_QUADRATURE_X4,
    .counts_per_detent = ENCODER_COUNTS_PER_DETENT,
    .glitch_filter_ns  = GLITCH_FILTER_DELAY_NS,
    .high_limit        = ENCODER_PCNT_HIGH_LIMIT,
    .low_limit         = ENCODER_PCNT_LOW_LIMIT,
};

static const button_config_t button_cfg = {
    .gpio = ENCODER_GPIO_SW,
};

static const servo_config_t servo_cfg = {
    .gpio         = SERVO_GPIO,
    .mode         = SERVO_LEDC_MODE,
    .timer        = SERVO_LEDC_TIMER,
    .channel      = SERVO_LEDC_CHANNEL,
    .duty_res     = SERVO_DUTY_RES,
    .freq_hz      = SERVO_FREQ_HZ,
    .min_pulse_us = SERVO_MIN_PULSE_US,
    .max_pulse_us = SERVO_MAX_PULSE_US,
    .range_deg    = SERVO_RANGE_DEG,
    .min_deg      = SERVO_MIN_DEG,
    .max_deg      = SERVO_MAX_DEG,
    .init_deg     = SERVO_INIT_DEG,
};

static const buzzer_config_t buzzer_cfg = {
    .gpio         = BUZZER_GPIO,
    .mode         = BUZZER_LEDC_MODE,
    .timer        = BUZZER_LEDC_TIMER,
    .channel      = BUZZER_LEDC_CHANNEL,
    .duty_res     = BUZZER_DUTY_RES,
    .duty         = BUZZER_DUTY,
    .init_freq_hz = BUZZER_NOTE_HZ,
};

/* Driver handles: hold each driver's state between superloop cycles */
static encoder_t encoder;
static button_t  button;
static servo_t   servo;
static buzzer_t  buzzer;

/* ============================ app_main ============================ */

void app_main(void)
{
    encoder_init(&encoder, &encoder_cfg);
    button_init(&button, &button_cfg);
    servo_init(&servo, &servo_cfg);
    buzzer_init(&buzzer, &buzzer_cfg);
    encoder_debug_init(&encoder);

    int32_t speed_k = SPEED_K_INIT;    // k: step multiplier
    TickType_t last_log = xTaskGetTickCount();

    ESP_LOGI(TAG, "superloop started, delay %d ms", SUPERLOOP_DELAY_MS);

    while (1)
    {

        TickType_t now = xTaskGetTickCount();
        /* 1. Update encoder ticks */
        int32_t ticks = encoder_update_ticks(&encoder);

        /* 2. Button event pressed? (once per press, no auto-repeat) */
        if (button_pressed_event(&button))
        {
            speed_k = speed_update(speed_k);    // Yes: k = 2k (mod N)
        }
        // No: do nothing

        /* 3. Update servo PWM duty cycle
         * Moves the servo by ticks * SERVO_STEP_FINE_DEG * speed_k degrees.
         * ticks == 0: angle did not change, the servo is not touched. */
        bool servo_updated = false;
        if (ticks != 0)
        {
            int32_t cmd_angle_deg = servo.status.angle_deg + ticks * SERVO_STEP_FINE_DEG * speed_k; // Updates based on current speed
            servo_updated = servo_set_angle(&servo, cmd_angle_deg);
        }

        /* 4. Duty cycle out of bounds? */
        buzzer_update(&buzzer);                 // count down a note started in earlier cycles
        // if (servo.status.buzzer_error_flag)
        if (servo_updated)
        {
            buzzer_emit_note(&buzzer, BUZZER_NOTE_HZ, BUZZER_BEEP_CYCLES);  // Yes: play_note for the next two cycles
        }
        // No: do nothing


        if ((TickType_t)(now - last_log) >= pdMS_TO_TICKS(LOG_PERIOD_MS)) {
            last_log = now;
            ESP_LOGI(TAG,
                    "ENCODER_PULSES=%" PRId32 ", ENCODER_ANGLE=%.2f deg, SERVO_ANGLE=%" PRId32 "/%.0f deg, ANGLE_ERROR=%.2f deg (%.2f%%)",
                    ticks,
                    servo.status.angle_deg,
                    servo.status.achieved_angle_deg,
                    SERVO_MAX_RANGE_DEG,
                    servo.status.deviation_deg,
                    servo.status.angle_dev_pcnt);

            ESP_LOGI(TAG,
                    "SERVO_DUTY=%" PRId32 " ticks, SERVO_PULSE=%.3f ms, SPEED_K=%" PRId32,
                    servo.status.duty_ticks,
                    servo.status.pulse_width_ms,
                    speed_k);
        }

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY_MS));
    }
}
