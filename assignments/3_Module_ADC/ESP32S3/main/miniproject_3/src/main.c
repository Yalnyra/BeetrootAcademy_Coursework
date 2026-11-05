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
 * Wiring (ESP32-S3 DevKitC-1):
 *   Encoder CLK (A) -> GPIO5
 *   Encoder DT  (B) -> GPIO6
 *   Encoder SW      -> GPIO7   TODO: confirm pin (active LOW, internal pull-up)
 *   Servo signal    -> GPIO14  (servo on external 5 V, common GND)
 *   Passive buzzer  -> GPIO18
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/pulse_cnt.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* ============================== Pins ============================== */
#define ENCODER_GPIO_A          5       // CLK: counted edges
#define ENCODER_GPIO_B          6       // DT:  direction level
#define ENCODER_GPIO_SW         7       // SW:  push button, active LOW
#define SERVO_GPIO              14
#define BUZZER_GPIO             18

/* =========================== Superloop ============================ */
#define SUPERLOOP_DELAY_MS      50      // vTaskDelay per cycle (FREERTOS_HZ=100 -> 5 ticks)

/* ================ Servo: LEDC timer 0 / channel 0 ================= */
#define SERVO_LEDC_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_TIMER        LEDC_TIMER_0
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_0
#define SERVO_FREQ_HZ           50
#define SERVO_PERIOD_US         (1000000 / SERVO_FREQ_HZ)   // 20 000 us
#define SERVO_DUTY_RES          LEDC_TIMER_12_BIT
#define SERVO_MAX_DUTY          ((1 << SERVO_DUTY_RES) - 1)

#define SERVO_RANGE_DEG         180     // physical range: 0..180 deg ...
#define SERVO_MIN_US            500     // ... maps to 500..2500 us pulse
#define SERVO_MAX_US            2500
#define SERVO_MIN_DEG           10      // travel limits: trying to go past
#define SERVO_MAX_DEG           170     // these triggers the beep
#define SERVO_INIT_DEG          90

/* =================== Step size: k = 2k (mod N) ==================== */
#define SERVO_STEP_FINE_DEG     5       // degrees per tick at k = 1
#define SPEED_N                 3       // N, see speed_update()
#define SPEED_K_INIT            2       // start coarse: 2 * 5 = 10 deg/tick

/* =============== Buzzer: LEDC timer 1 / channel 1 ================= */
/* The servo needs its own 50 Hz timer, so the buzzer cannot share timer 0. */
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_TIMER       LEDC_TIMER_1
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_1
#define BUZZER_DUTY_RES         LEDC_TIMER_10_BIT
#define BUZZER_DUTY             512     // 50 % of 10 bit
#define BUZZER_NOTE_HZ          440     // NOTE_A4
#define BUZZER_BEEP_CYCLES      2       // "play_note for the next two cycles"

static const char *TAG = "mini_project_3";

/* ===================== Function declarations ====================== */

/* Init — called once before the superloop */
static void encoder_init(void);
static void button_init(void);
static void servo_init(void);
static void buzzer_init(void);

/* Superloop blocks — one function per flowchart block */
static int32_t  encoder_update_ticks(void);
static bool     button_pressed_event(void);
static uint32_t speed_update(uint32_t k);
static bool     servo_update_duty(int32_t ticks, uint32_t speed_k);
static void     buzzer_emit_note(uint32_t freq_hz, uint32_t cycles);
static void     buzzer_update(void);

/* Helpers */
static uint32_t servo_angle_to_duty(int32_t angle_deg);
static inline int32_t val_clamp(int32_t value, int32_t val_min, int32_t val_max);
static inline int32_t mad_map(int32_t value, int32_t in_min, int32_t in_max,
                              int32_t out_min, int32_t out_max);

/* Дескриптор апаратного лічильника імпульсів (PCNT) */
static pcnt_unit_handle_t pcnt_unit = NULL;

/* ============================ app_main ============================ */

void app_main(void)
{
    encoder_init();
    button_init();
    servo_init();
    buzzer_init();

    uint32_t speed_k = SPEED_K_INIT;    // k: step multiplier

    ESP_LOGI(TAG, "superloop started, delay %d ms", SUPERLOOP_DELAY_MS);

    while (1)
    {
        /* 1. Update encoder ticks */
        int32_t ticks = encoder_update_ticks();

        /* 2. Button event pressed? (once per press, no auto-repeat) */
        if (button_pressed_event())
        {
            speed_k = speed_update(speed_k);    // Yes: k = 2k (mod N)
        }
        // No: do nothing

        /* 3. Update servo PWM duty cycle */
        bool out_of_bounds = servo_update_duty(ticks, speed_k);

        /* 4. Duty cycle out of bounds? */
        buzzer_update();                        // count down a note started in earlier cycles
        if (out_of_bounds)
        {
            buzzer_emit_note(BUZZER_NOTE_HZ, BUZZER_BEEP_CYCLES);  // Yes: play_note for the next two cycles
        }
        // No: do nothing

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY_MS));
    }
}

/* ============================== Init ============================== */

/* Ініціалізація апаратного лічильника для енкодера (з lesson 3.6, режим X1) */
static void encoder_init(void)
{
    // Межі лічильника (діапазон 16-бітного знакового числа)
    pcnt_unit_config_t unit_config = {
        .high_limit = 32767,    // Верхня межа лічильника
        .low_limit  = -32768,   // Нижня межа лічильника
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    // Апаратний фільтр завад: імпульси коротші за 10 мкс ігноруються
    pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 10000 };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    // Канал рахує фронти сигналу A, а сигнал B визначає напрямок обертання
    pcnt_channel_handle_t chan_a = NULL;
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num  = ENCODER_GPIO_A,
        .level_gpio_num = ENCODER_GPIO_B,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &chan_a));

    // Передній фронт A: +1, задній фронт A: ігнорувати
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,
        PCNT_CHANNEL_EDGE_ACTION_HOLD));

    // B = 1: напрямок як є, B = 0: інвертувати
    // Якщо обертання враховується у зворотний бік, поміняйте KEEP та INVERSE місцями
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    ESP_LOGI(TAG, "PCNT X1 initialized");
}

/* Encoder push button: polled input, SW shorts to GND when pressed */
static void button_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << ENCODER_GPIO_SW,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,      // polled from the superloop, no ISR
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
}

static void servo_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode      = SERVO_LEDC_MODE,
        .timer_num       = SERVO_LEDC_TIMER,
        .duty_resolution = SERVO_DUTY_RES,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .speed_mode = SERVO_LEDC_MODE,
        .channel    = SERVO_LEDC_CHANNEL,
        .timer_sel  = SERVO_LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = SERVO_GPIO,
        .duty       = servo_angle_to_duty(SERVO_INIT_DEG),  // 0 until the helper is implemented
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

static void buzzer_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode      = BUZZER_LEDC_MODE,
        .timer_num       = BUZZER_LEDC_TIMER,
        .duty_resolution = BUZZER_DUTY_RES,
        .freq_hz         = BUZZER_NOTE_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .speed_mode = BUZZER_LEDC_MODE,
        .channel    = BUZZER_LEDC_CHANNEL,
        .timer_sel  = BUZZER_LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = BUZZER_GPIO,
        .duty       = 0,                        // start silent
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

/* ======================== Superloop blocks ======================== */

/* 1. Update encoder ticks
 * Returns the tick delta since the previous cycle (+ CW / - CCW). */
static int32_t encoder_update_ticks(void)
{
    // TODO:
    //  - pcnt_unit_get_count(pcnt_unit, &count)
    //  - delta = count - prev_count; prev_count = count   (prev_count: a `static int` local)
    //  - don't pcnt_unit_clear_count() every cycle: ticks that land between
    //    the get and the clear are lost
    return 0;
}

/* 2. Button event pressed? (once per press event)
 * Returns true exactly once per press; holding the button does not repeat. */
static bool button_pressed_event(void)
{
    // TODO:
    //  - level = gpio_get_level(ENCODER_GPIO_SW)    (0 = pressed)
    //  - return true only on the 1 -> 0 transition; keep the previous level in a `static int` local
    //  - polling every SUPERLOOP_DELAY_MS is slower than contact bounce, so this already debounces
    return false;
}

/* 2 -> Yes. Update speed: k = 2k (mod N), N - maximum speed
 * Returns the new k. */
static uint32_t speed_update(uint32_t k)
{
    // TODO: k = 2k (mod N)
    //  - with N = SPEED_N = 3, k alternates 2 -> 1 -> 2 -> ... (2*2 mod 3 = 1, 2*1 mod 3 = 2):
    //    first press halves the step, second press restores it
    //  - careful: with a power-of-two N (e.g. 4) the sequence is 1 -> 2 -> 0 -> 0,
    //    and k = 0 means the servo never moves again
    return k;
}

/* 3. Update servo PWM duty cycle
 * Moves the servo by ticks * SERVO_STEP_FINE_DEG * speed_k degrees, clamped
 * to SERVO_MIN_DEG..SERVO_MAX_DEG.
 * Returns true if this cycle's ticks tried to push the shaft past a limit. */
static bool servo_update_duty(int32_t ticks, uint32_t speed_k)
{
    // TODO:
    //  - target = angle + ticks * SERVO_STEP_FINE_DEG * speed_k
    //    (angle: a `static int32_t` local holding the position, starts at SERVO_INIT_DEG)
    //  - out_of_bounds = target < SERVO_MIN_DEG || target > SERVO_MAX_DEG
    //  - angle = val_clamp(target, SERVO_MIN_DEG, SERVO_MAX_DEG)
    //  - if angle changed: ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, servo_angle_to_duty(angle))
    //                      ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL)
    //  - return out_of_bounds (false when ticks == 0, so a servo resting at a limit stays silent)
    (void)ticks;
    (void)speed_k;
    return false;
}

/* 4 -> Yes. Emit play_note for the next `cycles` superloop cycles
 * Non-blocking version of play_note() from lesson 3.4: starts the tone and
 * returns immediately; buzzer_update() stops it later. */
static void buzzer_emit_note(uint32_t freq_hz, uint32_t cycles)
{
    // TODO:
    //  - ledc_set_freq(BUZZER_LEDC_MODE, BUZZER_LEDC_TIMER, freq_hz)
    //  - ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, BUZZER_DUTY) + ledc_update_duty(...)
    //  - buzzer_cycles_left = cycles   (a file-scope `static uint32_t`, shared with buzzer_update();
    //    re-arming while the note plays keeps the tone on as long as the user keeps turning)
    (void)freq_hz;
    (void)cycles;
}

/* Called once per cycle: counts the note down and silences the buzzer at 0 */
static void buzzer_update(void)
{
    // TODO:
    //  - if buzzer_cycles_left > 0: decrement it
    //  - when it reaches 0: ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0) + ledc_update_duty(...)
}

/* ============================ Helpers ============================= */

/* Servo angle (0..SERVO_RANGE_DEG) -> LEDC duty ticks */
static uint32_t servo_angle_to_duty(int32_t angle_deg)
{
    // TODO:
    //  - pulse_us = mad_map(angle_deg, 0, SERVO_RANGE_DEG, SERVO_MIN_US, SERVO_MAX_US)
    //  - duty     = pulse_us * (SERVO_MAX_DUTY + 1) / SERVO_PERIOD_US   (12 bit @ 50 Hz: 1 ms ~ 205 ticks)
    (void)angle_deg;
    return 0;
}

static inline int32_t val_clamp(int32_t value, int32_t val_min, int32_t val_max)
{
    if (value < val_min) return val_min;
    if (value > val_max) return val_max;
    return value;
}

static inline int32_t mad_map(int32_t value, int32_t in_min, int32_t in_max,
                              int32_t out_min, int32_t out_max)
{
    if (in_min == in_max) return out_max;

    value = val_clamp(value, in_min, in_max);

    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}
