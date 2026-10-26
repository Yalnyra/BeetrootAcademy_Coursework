

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define LED          GPIO_NUM_4
#define BUTTON       GPIO_NUM_16
#define POLL_MS 5 
#define DEBOUNCE_MS  10

static const char *TAG = "BUTTON";

// Task 4: Polling + debounce as a state machine (no interrupts)
// Poll the pin every POLL_MS; a 4-state FSM filters bouncing on both press and release.
/*Dynamic global variables */
typedef enum { IDLE, DEBOUNCE_PRESS, HELD, DEBOUNCE_RELEASE } BtnState;
static BtnState btnState = IDLE;
uint32_t counter = 0;
uint32_t last_button_time = 0; // Measure delay between button exiting IDLE and LED switching on
uint32_t stateEnteredAt = 0;
uint32_t lastPollAt = 0;

// Task 4: Polling + debounce (without interrupts)
void pollButton(uint32_t ts) {

    bool pressed = !gpio_get_level(BUTTON); // LOW = 0 - pressed

    switch (btnState) {
        case IDLE:
            if (pressed) {
                btnState = DEBOUNCE_PRESS;
                stateEnteredAt = ts;
                    last_button_time = stateEnteredAt; // Check first transition by interrupt

            }
            break;

        case DEBOUNCE_PRESS:
            if (!pressed) {
                btnState = IDLE;
            } else if (ts - stateEnteredAt >= DEBOUNCE_MS) {
                counter++;
                btnState = HELD;
            }
            break;

        case HELD:
            if (!pressed) {
                btnState = DEBOUNCE_RELEASE;
                stateEnteredAt = ts;
            }
            break;

        case DEBOUNCE_RELEASE:
            if (pressed) {
                btnState = HELD;
            } else if (ts - stateEnteredAt >= DEBOUNCE_MS) {
                btnState = IDLE;
            }
            break;
    }
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
    
    // Remove any interrupts

    // Всі змінні для логіки тепер живуть локально і безпечно
    bool led_on = false;
    uint32_t last_button_time = 0; // Час останнього успішного натискання

    // Головний цикл
    while (1)
    {
        uint32_t last_counter = counter;
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - lastPollAt < POLL_MS) continue; // Пропускаємо цикл 
        lastPollAt = now;
        pollButton(now); // Оброблюємо натискання кнопки
        if (btnState == HELD){

            // Це реальне натискання, а не шум!
            led_on = true;
            // ESP_LOGI(TAG, "LED Status: %" PRIu32, led_on);
            gpio_set_level(LED, led_on);
   
            
            if (last_counter != counter){
                // if counter has incremented, add a new time log:
            ESP_LOGI(TAG, "Press! Time %" PRIu32, now);

            ESP_LOGI(TAG, "Count: %" PRIu32, last_counter);

            ESP_LOGI(TAG, "Interrupt latency: %" PRIu32, stateEnteredAt - last_button_time);

            }
            last_button_time = stateEnteredAt; // Check first transition by interrupt
        }
        else if (btnState == IDLE){
            // Відпускаємо кнопку
            led_on = false;
            // ESP_LOGI(TAG, "LED Status: %" PRIu32, led_on);
            gpio_set_level(LED, led_on);   
                       gpio_set_level(LED, led_on);   
        }
        
        vTaskDelay(pdMS_TO_TICKS(POLL_MS)); 
    }
}