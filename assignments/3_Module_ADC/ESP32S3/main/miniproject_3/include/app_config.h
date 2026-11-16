/*
 * Mini-project 3 — project settings
 *
 * Everything board- or assignment-specific lives here. The drivers in
 * ../../../components receive these values through their config structs
 * (see main.c), so the drivers themselves contain no pin numbers.
 */
#pragma once

#include <driver/ledc.h>

/* ============================== Pins ============================== */
#define ENCODER_GPIO_A          4       // CLK: counted edges
#define ENCODER_GPIO_B          5       // DT:  direction level
#define ENCODER_GPIO_SW         6       // SW:  push button, active LOW
#define SERVO_GPIO              7
#define BUZZER_GPIO             16
#define ENCODER_DEBUG_GPIO      15      // scope/LED probe: HIGH while the filtered PCNT count is moving

/* =========================== Superloop ============================ */
#define SUPERLOOP_DELAY_MS      50     // vTaskDelay per cycle (FREERTOS_HZ=100 -> 5 ticks)
#define LOG_PERIOD_MS           2000

/* ============================== Encoder ============================== */

/* Decoding mode, chosen at compile time:
 *   0 = X1: one channel, rising edge of A only (lesson 3.6). 1 count per click.
 *   1 = X4: two channels, both edges of A and B. 4 counts per click, bounce cancels in +1/-1 pairs.
 * Build with:  idf.py -DENCODER_X4=ON build   (OFF = X1, see main/CMakeLists.txt)
 * or change the default below. */
#ifndef ENCODER_QUADRATURE_X4
#define ENCODER_QUADRATURE_X4      0
#endif

#if ENCODER_QUADRATURE_X4
#define ENCODER_COUNTS_PER_DETENT  4   // X4: one click = full quadrature cycle = 4 edges (set 2 if one click gives delta=2)
#else
#define ENCODER_COUNTS_PER_DETENT  1   // X1: one click = one rising edge of A
#endif
#define GLITCH_FILTER_DELAY_NS 1000
#define ENCODER_PCNT_HIGH_LIMIT    32767    // Верхня межа лічильника
#define ENCODER_PCNT_LOW_LIMIT     -32768   // Нижня межа лічильника

/* ================ Encoder: Button  ================= */

#define DEBOUNCE_MS        30u
#define SHORT_TIME_LIMIT   800u   // hold longer than this → long press

/* ================ Servo: LEDC timer 0 / channel 0 ================= */
#define SERVO_LEDC_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_TIMER        LEDC_TIMER_0
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_0
#define SERVO_MIN_PULSE_US      500U        // pulse at 0 deg
#define SERVO_MAX_PULSE_US      2500U       // pulse at 180 deg
#define SERVO_FREQ_HZ           50          // pwm update frequency
#define SERVO_DUTY_RES          LEDC_TIMER_13_BIT

#define SERVO_RANGE_DEG         180     // physical range: 0..180 deg ...
#define SERVO_MIN_DEG           00      // travel limits: trying to go past
#define SERVO_MAX_DEG           180     // these triggers the beep
#define SERVO_INIT_DEG          0

#define SERVO_MAX_RANGE_DEG  180.0f     // used in the log line

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
