// Пасивний баззер з використанням PWM-генератора ESP32-S3

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define NOTE_C4  261
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  329
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  493
#define NOTE_C5  523
#define NOTE_MAX 0 // Used to represent a rest/pause

#define BUZZER_GPIO           GPIO_NUM_18
#define LEDC_TIMER            LEDC_TIMER_0
#define LEDC_MODE             LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL          LEDC_CHANNEL_0
#define LEDC_DUTY_RES         LEDC_TIMER_10_BIT // 10-bit resolution (0-1023)
#define LEDC_DUTY             512               // 50% duty cycle (1023 / 2)

void buzzer_init(void) {
    // 1. Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = 1000, // Initial frequency
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 2. Configure the LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .gpio_num       = BUZZER_GPIO,
        .duty           = 0, // Start silent (0% duty)
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void play_note(uint32_t freq_hz, uint32_t duration_ms) {
    if (freq_hz > 0) {
        // Set frequency and turn on duty cycle
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq_hz);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    } else {
        // Rest / Silence
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    }
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // Brief pause/silence between notes
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void app_main(void) {
    buzzer_init();

    // Play a simple scale or melody
    int melody[] = { NOTE_C4, NOTE_C4, NOTE_D4, NOTE_MAX, NOTE_C4, NOTE_C4, NOTE_D4, NOTE_MAX, NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_MAX};
    int durations[] = { 300, 300, 300, 500, 300, 300, 300, 500,  300, 300, 300, 500 };

    while (1) {
        for (int i = 0; i < 8; i++) {
            play_note(melody[i], durations[i]);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}