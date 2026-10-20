

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define LED          GPIO_NUM_4
#define BUTTON       GPIO_NUM_16
#define DEBOUNCE_MS  50


volatile bool button_event_flag = false;
volatile uint32_t stateEnteredAt = 0; // Час першого фронту (з ISR), мс
static uint32_t counter = 0;          // Кількість зарахованих натискань

static const char *TAG = "BUTTON";

static void IRAM_ATTR button_isr_handler(void* arg) 
{
    // Ставимо прапорець і запам'ятовуємо час першого фронту
    if (!button_event_flag) {
        stateEnteredAt = (uint32_t)(esp_timer_get_time() / 1000);
    }
    button_event_flag = true;
}

void app_main(void)
{
    // Налаштування світлодіода
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 0);

    // Налаштування кнопки
    gpio_reset_pin(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(BUTTON, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON, button_isr_handler, NULL);

    // Всі змінні для логіки тепер живуть локально і безпечно
    bool led_on = false;
    uint32_t last_button_time = 0; // Попередній перший фронт (для логу)

    // Головний цикл
    while (1)
    {
        uint32_t last_counter = counter;

        // Якщо преривання повідомило про зміну сигналу (навіть про брязкіт)
        if (button_event_flag) {
            
            // Одразу скидаємо прапорець
            button_event_flag = false; 
            
            // Отримуємо поточний час
            uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
            
            // Passthrough: без антибрязкоту, кожен фронт перемикає LED одразу
            led_on = !led_on;
            gpio_set_level(LED, led_on);
            counter++;
            // ESP_LOGI(TAG, "LED Status: %" PRIu32, led_on);

            if (last_counter != counter){
                // if counter has incremented, add a new time log:
                ESP_LOGI(TAG, "Press! Time %" PRIu32, now);
                ESP_LOGI(TAG, "Count: %" PRIu32, last_counter);
                ESP_LOGI(TAG, "Interrupt latency: %" PRIu32, stateEnteredAt - last_button_time);
            }
            last_button_time = stateEnteredAt; // Check first transition by interrupt
        }

        vTaskDelay(1); 
    }
}