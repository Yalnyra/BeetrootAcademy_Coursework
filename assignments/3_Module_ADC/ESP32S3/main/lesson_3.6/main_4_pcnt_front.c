#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/pulse_cnt.h"

// ---------------- Налаштування пінів і параметрів енкодера ----------------
#define ENCODER_GPIO_A   5                    // Пін CLK (сигнал A)
#define ENCODER_GPIO_B   6                    // Пін DT (сигнал B)
#define ENCODER_PPR      20                   // Кількість імпульсів на один оберт (PPR) по одному каналу
#define ENCODER_CPR      (ENCODER_PPR * 4)    // Режим X4: 4 відліки на період, тому CPR = PPR * 4 = 80
#define SAMPLE_MS        100                  // Період виводу позиції в лог, мс

// Тег для виводу повідомлень у лог
static const char *TAG = "ENCODER";

// Дескриптор апаратного лічильника імпульсів (PCNT)
static pcnt_unit_handle_t pcnt_unit = NULL;

// Функція ініціалізації апаратного лічильника для енкодера (режим X4)
static void setup_encoder_pcnt(void)
{
    // Межі лічильника (діапазон 16-бітного знакового числа)
    pcnt_unit_config_t unit_config = {
        .high_limit = 32767,    // Верхня межа лічильника
        .low_limit  = -32768,   // Нижня межа лічильника
    };
    // Створюємо блок лічильника PCNT
    pcnt_new_unit(&unit_config, &pcnt_unit);

    // Апаратний фільтр завад: імпульси коротші за 10 мкс ігноруються
    pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 10000 };
    pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);

    // Канал A: рахує обидва фронти сигналу A, а рівень сигналу B визначає напрямок
    pcnt_channel_handle_t chan_a = NULL;
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num  = ENCODER_GPIO_A,   // Пін, за фронтами якого ведеться рахунок
        .level_gpio_num = ENCODER_GPIO_B,   // Пін, за рівнем якого визначається напрямок
    };
    pcnt_new_channel(pcnt_unit, &chan_a_config, &chan_a);

    // Канал B: рахує обидва фронти сигналу B, а рівень сигналу A визначає напрямок
    pcnt_channel_handle_t chan_b = NULL;
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num  = ENCODER_GPIO_B,   // Пін, за фронтами якого ведеться рахунок
        .level_gpio_num = ENCODER_GPIO_A,   // Пін, за рівнем якого визначається напрямок
    };
    pcnt_new_channel(pcnt_unit, &chan_b_config, &chan_b);

    // Дія за фронтом сигналу A:
    // передній фронт (↑) = -1, задній фронт (↓) = +1
    pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE,   // Передній фронт A: зменшити лічильник
        PCNT_CHANNEL_EDGE_ACTION_INCREASE);  // Задній фронт A: збільшити лічильник
    // Дія за рівнем сигналу B (визначає напрямок):
    // B = 1: напрямок не змінюється, B = 0: напрямок інвертується
    pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,      // B = 1: залишити напрямок як є
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE);  // B = 0: інвертувати напрямок

    // Дія за фронтом сигналу B:
    // передній фронт (↑) = +1, задній фронт (↓) = -1
    pcnt_channel_set_edge_action(chan_b,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,   // Передній фронт B: збільшити лічильник
        PCNT_CHANNEL_EDGE_ACTION_DECREASE);  // Задній фронт B: зменшити лічильник
    // Дія за рівнем сигналу A (визначає напрямок):
    // A = 1: напрямок не змінюється, A = 0: напрямок інвертується
    pcnt_channel_set_level_action(chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,      // A = 1: залишити напрямок як є
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE);  // A = 0: інвертувати напрямок

    
    // // Same sign as the old X1 setup (A rising while B high = +1).
    // // If CW counts down, swap INCREASE/DECREASE in BOTH edge actions.
    // ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
    //     PCNT_CHANNEL_EDGE_ACTION_INCREASE,     // A rising
    //     PCNT_CHANNEL_EDGE_ACTION_DECREASE));   // A falling
    // ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
    //     PCNT_CHANNEL_LEVEL_ACTION_KEEP,        // B high
    //     PCNT_CHANNEL_LEVEL_ACTION_INVERSE));   // B low
    // ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
    //     PCNT_CHANNEL_EDGE_ACTION_DECREASE,     // B rising
    //     PCNT_CHANNEL_EDGE_ACTION_INCREASE));   // B falling
    // ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
    //     PCNT_CHANNEL_LEVEL_ACTION_KEEP,        // A high
    //     PCNT_CHANNEL_LEVEL_ACTION_INVERSE));   // A low
    // Якщо обертання враховується у зворотний бік, поміняйте місцями ENCODER_GPIO_A та ENCODER_GPIO_B

    pcnt_unit_enable(pcnt_unit);        // Вмикаємо блок лічильника
    pcnt_unit_clear_count(pcnt_unit);   // Скидаємо лічильник у 0
    pcnt_unit_start(pcnt_unit);         // Запускаємо рахунок

    ESP_LOGI(TAG, "PCNT X4 initialized");
}

void app_main(void)
{
    // Ініціалізуємо енкодер
    setup_encoder_pcnt();

    while (1) {
        int count = 0;
        // Зчитуємо поточне значення лічильника
        pcnt_unit_get_count(pcnt_unit, &count);

        // Кут повороту в градусах відносно початкової позиції
        float angle = count * 360.0f / ENCODER_CPR;

        // Виводимо позицію і кут у лог
        ESP_LOGI(TAG, "pos=%d steps, angle=%.1f deg", (int)count/4, angle);

        // Пауза до наступного виводу
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
    }
}