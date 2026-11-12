/** @author Yevhenii Vinokur, Claude 5.5 Opus @ 2026
* @brief Miniproject 2: Світлофор 
* FSM з 7 станів + hardware timer + Обробник подій + Дебрязкіт кнопки 
* Дебрязкіт методом 2 з 2.4 - Рахується затримка через esp_timer
* Обробник FSM таймерів реалізований за допомогою черги FreeRTOS 
*  Два окремих таймери (esp_timer):
*    state_timer - одноразовий (one-shot). При вході в кожен стан перезапускається
*                  з тривалістю саме цього стану (state_duration_ms) -> "змінює період".
*    blink_timer - періодичний, період задається ОДИН раз (BLINK_HALF_PERIOD_MS),
*                вмикається/вимикається лише функцією update_blink_timer().
* Обидва таймери та ISR кнопки тільки кладуть подію в чергу; вся логіка - у switch app_main.
*/
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "TRAFFIC";

// ---------- Налаштування пінів ----------
#define PIN_GREEN   GPIO_NUM_5
#define PIN_YELLOW  GPIO_NUM_6
#define PIN_RED     GPIO_NUM_7
#define PIN_BUTTON  GPIO_NUM_4

// ---------- Тривалості (мілісекунди) ----------
#define T_GREEN_MS          10000UL   // зелений: 10 с
#define T_GREEN_BLINK_MS     3000UL   // зелений миготливий: 3 с
#define T_YELLOW_MS          3000UL   // жовтий: 3 с
#define T_RED_MS            10000UL   // червоний: 10 с
#define T_RED_YELLOW_MS      2000UL   // червоний + жовтий: 2 с
#define BLINK_HALF_PERIOD_MS  500UL   // півперіод миготіння: 0,5 с
#define DEBOUNCE_MS           200UL   // антидребезг кнопки

// esp_timer рахує в мікросекундах (своя база 1 МГц, дільник APB не потрібен)
#define MS_TO_US(ms) ((uint64_t)(ms) * 1000ULL)

// ---------- МАСКА ОБРОБНИК ПОДІЙ ----------
// TODO (IGNORE FOR NOW): замінити чергу на task notification з бітовою маскою,
// дати пріоритет біту кнопки.
#define EVT_TIMEOUT_BIT (1UL << 0)
#define EVT_BLINK_BIT   (1UL << 1)
#define EVT_BUTTON_BIT  (1UL << 2)

// ---------- Стани автомата ----------
typedef enum {
    STATE_GREEN = 0,      // зелений
    STATE_GREEN_BLINK,    // зелений миготливий
    STATE_YELLOW,         // жовтий
    STATE_RED,            // червоний
    STATE_RED_YELLOW,     // червоний + жовтий
    STATE_YELLOW_BLINK    // особливий режим: жовтий миготливий (до наступного натискання)
} TrafficState;

// ---------- Події ----------
typedef enum {
    EVT_STATE_TIMEOUT = 1,  // спрацював таймер тривалості стану
    EVT_BLINK_TICK,         // тік таймера миготіння
    EVT_BUTTON              // натиснута кнопка
} TrafficEvent;

static QueueHandle_t      traffic_queue;
static esp_timer_handle_t state_timer;   // одноразовий, період залежить від стану
static esp_timer_handle_t blink_timer;   // періодичний, період фіксований
static bool               blink_timer_running = false;
static bool               blink_level = false;   // поточна фаза миготіння
static TrafficState       traffic_state = STATE_GREEN;

// ============================================================
//  Джерела подій (ISR / колбеки таймерів) - тільки кладуть подію в чергу
// ============================================================
static BaseType_t enqueue_from_isr(TrafficEvent event,
                                   BaseType_t *higher_priority_task_woken)
{
    return xQueueSendFromISR(traffic_queue, &event, higher_priority_task_woken);
}

static void IRAM_ATTR button_isr_handler(void *arg)
{
    // Антидребезг прямо в ISR: esp_timer_get_time() безпечна для переривань
    static int64_t last_press_us = 0;
    int64_t now = esp_timer_get_time();
    if (now - last_press_us < (int64_t)MS_TO_US(DEBOUNCE_MS)) {
        return;   // брязкіт - ігноруємо
    }
    last_press_us = now;

    BaseType_t woken = pdFALSE;
    enqueue_from_isr(EVT_BUTTON, &woken);
    portYIELD_FROM_ISR(woken);
}

// Колбеки esp_timer виконуються в задачі esp_timer (не в ISR),
// тому тут звичайний xQueueSend без блокування.
static void state_timer_cb(void *arg)
{
    TrafficEvent e = EVT_STATE_TIMEOUT;
    xQueueSend(traffic_queue, &e, 0);
}

static void blink_timer_cb(void *arg)
{
    TrafficEvent e = EVT_BLINK_TICK;
    xQueueSend(traffic_queue, &e, 0);
}

// ============================================================
//  Таймер 1: тривалість стану (період змінюється з кожним станом)
// ============================================================
static uint32_t state_duration_ms(TrafficState state)
{
    switch (state) {
        case STATE_GREEN:        return T_GREEN_MS;
        case STATE_GREEN_BLINK:  return T_GREEN_BLINK_MS;
        case STATE_YELLOW:       return T_YELLOW_MS;
        case STATE_RED:          return T_RED_MS;
        case STATE_RED_YELLOW:   return T_RED_YELLOW_MS;
        case STATE_YELLOW_BLINK: return 0;   // без таймауту - вихід тільки кнопкою
        default:                 return 0;
    }
}

static void restart_state_timer(TrafficState state)
{
    esp_timer_stop(state_timer);   // ESP_ERR_INVALID_STATE, якщо вже не працює - це нормально

    uint32_t ms = state_duration_ms(state);
    if (ms > 0) {
        ESP_ERROR_CHECK(esp_timer_start_once(state_timer, MS_TO_US(ms)));
    }
}

// ============================================================
//  Таймер 2: миготіння (період заданий один раз, лише старт/стоп)
// ============================================================
static void update_blink_timer(TrafficState state)
{
    bool should_blink = (state == STATE_GREEN_BLINK) ||
                        (state == STATE_YELLOW_BLINK);

    if (should_blink && !blink_timer_running) {
        if (esp_timer_start_periodic(blink_timer, MS_TO_US(BLINK_HALF_PERIOD_MS)) == ESP_OK) {
            blink_timer_running = true;
        }
    } else if (!should_blink && blink_timer_running) {
        if (esp_timer_stop(blink_timer) == ESP_OK) {
            blink_timer_running = false;
        }
    }
}

// ============================================================
//  Вихідні сигнали
// ============================================================
static void set_leds(bool green, bool yellow, bool red)
{
    gpio_set_level(PIN_GREEN,  green);
    gpio_set_level(PIN_YELLOW, yellow);
    gpio_set_level(PIN_RED,    red);
}

// Вхід у новий стан: статичні LED + обидва таймери
static void enter_state(TrafficState new_state)
{
    traffic_state = new_state;
    blink_level   = true;   // миготливий стан стартує з увімкненого LED

    switch (new_state) {
        case STATE_GREEN:        set_leds(1, 0, 0); break;
        case STATE_GREEN_BLINK:  set_leds(1, 0, 0); break;
        case STATE_YELLOW:       set_leds(0, 1, 0); break;
        case STATE_RED:          set_leds(0, 0, 1); break;
        case STATE_RED_YELLOW:   set_leds(0, 1, 1); break;
        case STATE_YELLOW_BLINK: set_leds(0, 1, 0); break;
    }

    update_blink_timer(new_state);   // вмикає/вимикає таймер миготіння
    restart_state_timer(new_state);  // новий період таймера стану

    ESP_LOGI(TAG, "-> state %d (%lu ms)", new_state,
             (unsigned long)state_duration_ms(new_state));
}

// Нормальний цикл світлофора
static TrafficState next_state(TrafficState state)
{
    switch (state) {
        case STATE_GREEN:        return STATE_GREEN_BLINK;
        case STATE_GREEN_BLINK:  return STATE_YELLOW;
        case STATE_YELLOW:       return STATE_RED;
        case STATE_RED:          return STATE_RED_YELLOW;
        case STATE_RED_YELLOW:   return STATE_GREEN;
        case STATE_YELLOW_BLINK: return STATE_YELLOW_BLINK;
        default:                 return STATE_RED;   // безпечний стан
    }
}

// ============================================================
//  Обробник подій - головний switch автомата
// ============================================================
static void handle_event(TrafficEvent evt)
{
    switch (evt) {
        case EVT_STATE_TIMEOUT:
            // У YELLOW_BLINK таймаутів немає; запізнілий таймаут, що прийшов
            // одразу після натискання кнопки, тут просто ігнорується.
            if (traffic_state != STATE_YELLOW_BLINK) {
                enter_state(next_state(traffic_state));
            }
            break;

        case EVT_BLINK_TICK:
            blink_level = !blink_level;
            switch (traffic_state) {
                case STATE_GREEN_BLINK:  gpio_set_level(PIN_GREEN,  blink_level); break;
                case STATE_YELLOW_BLINK: gpio_set_level(PIN_YELLOW, blink_level); break;
                default: break;   // запізнілий тік після зупинки таймера - ігноруємо
            }
            break;

        case EVT_BUTTON:
            ESP_LOGI(TAG, "button");
            if (traffic_state == STATE_YELLOW_BLINK) {
                enter_state(STATE_RED);            // вихід з особливого режиму
            } else {
                enter_state(STATE_YELLOW_BLINK);   // вхід в особливий режим
            }
            break;

        default:
            ESP_LOGW(TAG, "unknown event %d", evt);
            break;
    }
}

// ============================================================
//  Ініціалізація
// ============================================================
static void init_gpio(void)
{
    gpio_config_t leds = {
        .pin_bit_mask = (1ULL << PIN_GREEN) | (1ULL << PIN_YELLOW) | (1ULL << PIN_RED),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&leds));

    gpio_config_t btn = {
        .pin_bit_mask = (1ULL << PIN_BUTTON),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,     // кнопка на GND
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,      // натискання = спад
    };
    ESP_ERROR_CHECK(gpio_config(&btn));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_BUTTON, button_isr_handler, NULL));
}

static void init_timers(void)
{
    const esp_timer_create_args_t state_args = {
        .callback = state_timer_cb,
        .name     = "state",
    };
    ESP_ERROR_CHECK(esp_timer_create(&state_args, &state_timer));

    const esp_timer_create_args_t blink_args = {
        .callback = blink_timer_cb,
        .name     = "blink",
    };
    ESP_ERROR_CHECK(esp_timer_create(&blink_args, &blink_timer));
}

void app_main(void)
{
    traffic_queue = xQueueCreate(10, sizeof(TrafficEvent));
    configASSERT(traffic_queue);

    init_gpio();
    init_timers();
    enter_state(STATE_GREEN);

    TrafficEvent evt;
    while (1) {
        // Блокуємося до наступної події - CPU вільний, vTaskDelay не потрібен
        if (xQueueReceive(traffic_queue, &evt, portMAX_DELAY) == pdTRUE) {
            handle_event(evt);
        }
    }
}
