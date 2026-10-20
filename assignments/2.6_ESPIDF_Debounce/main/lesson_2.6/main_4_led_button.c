#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define LED          GPIO_NUM_4
#define BUTTON       GPIO_NUM_7
#define DEBOUNCE_MS  30

static const char *TAG = "BUTTON";

static inline uint32_t millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void app_main(void)
{

    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 0);

    gpio_reset_pin(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);

    bool led_on = false;

    int buttonState = 1;             // Поточний стабільний стан кнопки
    int lastButtonState = 1;         // Попереднє зчитане значення
    uint32_t lastDebounceTime = 0;   // Час останньої зміни стану
    uint32_t counter = 0;            // Кількість зарахованих натискань
    uint32_t stateEnteredAt = 0;     // Час першого фронту від стабільного стану
    uint32_t last_button_time = 0;   // Попередній перший фронт (для логу)

    while (1)
    {
        uint32_t last_counter = counter;
        uint32_t now = millis();
        int reading = gpio_get_level(BUTTON);

        // Перший фронт після стабільного стану
        if (reading != buttonState && lastButtonState == buttonState) {
            stateEnteredAt = now;
        }

        // Якщо стан змінився (через шум або реальне натискання)
        if (reading != lastButtonState) {
            lastDebounceTime = millis(); // Скидаємо таймер
        }

        // Якщо минуло достатньо часу (шум припинився)
        if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
            
            // Якщо стабільний стан дійсно змінився
            if (reading != buttonState) {
                buttonState = reading;
                
                // Тільки якщо кнопка натиснена (перехід в LOW / 0)
                if (buttonState == 0) {
                    led_on = !led_on;
                    gpio_set_level(LED, led_on);
                    // ESP_LOGI(TAG, "LED Status: %" PRIu32, led_on);
                    counter++;

                    if (last_counter != counter){
                        // if counter has incremented, add a new time log:
                        ESP_LOGI(TAG, "Press! Time %" PRIu32, now);
                        ESP_LOGI(TAG, "Count: %" PRIu32, last_counter);
                        ESP_LOGI(TAG, "Interrupt latency: %" PRIu32, stateEnteredAt - last_button_time);
                    }
                    last_button_time = stateEnteredAt; // Check first transition by interrupt
                }
            }
        }

        lastButtonState = reading;

        // Обов'язкова пауза для FreeRTOS, щоб не блокувати процесор
        vTaskDelay(1);
    }
}