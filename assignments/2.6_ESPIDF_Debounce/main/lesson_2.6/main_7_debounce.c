

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define LED          GPIO_NUM_4
#define BUTTON       GPIO_NUM_16
#define DEBOUNCE_MS  50


volatile bool button_event_flag = false;

static const char *TAG = "BUTTON";

static void IRAM_ATTR button_isr_handler(void* arg) 
{
    // Просто ставимо прапорець. Ніяких обчислень часу!
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
    

    // Всі змінні для логіки тепер живуть локально і безпечно
    bool led_on = false;
    uint32_t last_button_time = 0; // Час останнього успішного натискання

    // Головний цикл
    while (1)
    {
        // Якщо преривання повідомило про зміну сигналу (навіть про брязкіт)
        if (button_event_flag) {
            
            // Одразу скидаємо прапорець
            button_event_flag = false; 
            
            // Отримуємо поточний час
            uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
            
            // Перевіряємо антибрязкіт тут, у безпечній зоні
            if (now - last_button_time > DEBOUNCE_MS) {
                
                // Це реальне натискання, а не шум!
                led_on = !led_on;
                ESPI_LOGI(TAG, "LED Status: ", led_on);
                gpio_set_level(LED, led_on);
                
                // Запам'ятовуємо час цього дійсного натискання
                last_button_time = now;
            }
            // Якщо пройшло менше 50 мс, if просто не виконається,
            // шум буде проігноровано, а прапорець ми вже скинули.
        }

        vTaskDelay(1); 
    }
}