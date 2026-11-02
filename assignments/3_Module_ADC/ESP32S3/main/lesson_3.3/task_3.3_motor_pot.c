#include <stdint.h>
#include <math.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// Потенціометр: 
#define ADC_UNIT     ADC_UNIT_1        // АЦП1 
#define ADC_CHANNEL  ADC_CHANNEL_5     // GPIO6
#define POT_CHANNEL         ADC_CHANNEL_3  // ADC1, GPIO4
#define LDR_CHANNEL         ADC_CHANNEL_4  // ADC1, GPIO5
// від 60% PWM на мінімумі до 100% на максимумі.
#define MOTOR_MIN_PERCENT    60U
#define MOTOR_MAX_PERCENT    100U

// Power of the LED:
#define LED_MIN_PERCENT    10U
#define LED_MAX_PERCENT    60U

#define SERVO_PIN           7
#define LED_PIN             6
#define PWM_MODE            LEDC_LOW_SPEED_MODE

// TODO: only use single timer 
#define SERVO_TIMER         LEDC_TIMER_1
#define LED_TIMER           LEDC_TIMER_0
#define SERVO_CHANNEL       LEDC_CHANNEL_1
#define LED_CHANNEL         LEDC_CHANNEL_0


#define SERVO_FREQUENCY_HZ   50U
#define LED_FREQUENCY_HZ    1000
// general
#define LED_RESOLUTION      LEDC_TIMER_10_BIT
#define LED_PERIOD_TICKS    (1U << LED_RESOLUTION)
#define LED_MAX_DUTY        (LED_PERIOD_TICKS - 1U)
//led
#define LED_MIN_TICKS     ((LED_PERIOD_TICKS * LED_MIN_PERCENT + 50U) / 100U)
#define LED_MAX_TICKS     ((LED_PERIOD_TICKS * LED_MAX_PERCENT + 50U) / 100U)

//MOTOR 
#define SERVO_RESOLUTION     LEDC_TIMER_13_BIT        // 2^13 = 8192 ticks/period, matches your duty formula
#define SERVO_PERIOD_TICKS    8192
#define SERVO_PERIOD_US      (1000000U / SERVO_FREQUENCY_HZ)  // 20000 us

#define SERVO_MIN_PULSE_US   1000U                    // 1ms -> 0 deg
#define SERVO_MAX_PULSE_US   2000U                    // 2ms -> 180 deg
#define SERVO_ANGLE_MAX_DEG  180.0f
#define SERVO_ANGLE_STEP_DEG 45.0f
// Pulse widths expressed as duty ticks (rounded to nearest tick, same style as MOTOR_MIN/MAX_TICKS)
#define SERVO_MIN_TICKS \
    ((SERVO_MIN_PULSE_US * SERVO_PERIOD_TICKS + SERVO_PERIOD_US / 2U) / SERVO_PERIOD_US)
#define SERVO_MAX_TICKS \
    ((SERVO_MAX_PULSE_US * SERVO_PERIOD_TICKS + SERVO_PERIOD_US / 2U) / SERVO_PERIOD_US)

#define ADC_MAX_VALUE       4095
#define UPDATE_PERIOD_MS    20
// Servo update-rate control via potentiometer.
// SG90 datasheet speed is ~0.1 s / 60° under no load — commanding angle
// updates faster than that doesn't make it rotate faster, it just drives
// the motor against its own inertia/gearing (extra current, jitter, wear).
#define SERVO_UPDATE_PERIOD_MIN_MS   60U     // critical floor: fastest allowed interval
#define SERVO_UPDATE_PERIOD_MAX_MS   500U    // pot at 0 -> slowest
// #define SERVO_STEP_PERIOD_MS  2000U
#define LOG_PERIOD_MS       500

// _Static_assert(MOTOR_MIN_PERCENT <= MOTOR_MAX_PERCENT && MOTOR_MAX_PERCENT <= 100U,
//                "Invalid motor PWM range");



static const char *TAG = "hw_3_3";
static adc_oneshot_unit_handle_t adc_handle;


static esp_err_t pwm_output_init(int pin, ledc_channel_t channel,
                            ledc_timer_t timer, 
                            uint32_t duty_resolution,
                            uint32_t frequency_hz,
                            uint32_t initial_duty)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = PWM_MODE,
        .duty_resolution = duty_resolution,
        .timer_num = timer,
        // .freq_hz = LED_FREQUENCY_HZ, ERROR if duty_resolution == LEDC_TIMER_13_BIT        
        // 2^13 = 8192 ticks/period, doesn't match 80Mhz 

        .clk_cfg = LEDC_APB_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    const ledc_channel_config_t channel_config = {
        .gpio_num = pin,
        .speed_mode = PWM_MODE,
        .channel = channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer,
        .duty = initial_duty,
        .hpoint = 0,
    };
    return ledc_channel_config(&channel_config);
}

static esp_err_t adc_inputs_init(adc_oneshot_unit_handle_t *adc_handle)
{
    const adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));

    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    return adc_oneshot_config_channel(*adc_handle, POT_CHANNEL,
                                             &channel_config);
}

// Step 2. Set constant angle 
esp_err_t servo_set_angle_deg(float angle_deg)
{
    // Step 1: wrap angle into [0, 180) — keeps the "+angle_deg each call" pattern cycling
    float wrapped_deg = fmodf(angle_deg, SERVO_ANGLE_MAX_DEG);
    if (wrapped_deg < 0.0f) {
        wrapped_deg += SERVO_ANGLE_MAX_DEG;
    }
    float ratio = wrapped_deg / SERVO_ANGLE_MAX_DEG;   // 0.0 .. 1.0

    // Step 2 + 3 collapsed: interpolate directly between the precomputed tick
    // bounds (equivalent to computing pulse_us then duty, but skips the
    // intermediate float->us->ticks round trip)
    uint32_t duty = SERVO_MIN_TICKS +
        (uint32_t)(ratio * (float)(SERVO_MAX_TICKS - SERVO_MIN_TICKS) + 0.5f);

    esp_err_t err = ledc_set_duty(PWM_MODE, SERVO_CHANNEL, duty);
    if (err != ESP_OK) {
        return err;
    }
    return ledc_update_duty(PWM_MODE, SERVO_CHANNEL);
}


// TODO: Change the LED brightness range 
static uint32_t pot_to_led_duty(int value)
{
    if (value < 0) {
        value = 0;
    } else if (value > ADC_MAX_VALUE) {
        value = ADC_MAX_VALUE;
    }
    const uint32_t span = LED_MIN_TICKS - LED_MAX_TICKS;
    return LED_MIN_TICKS + ((uint32_t)value * span + ADC_MAX_VALUE / 2U) / ADC_MAX_VALUE;
}


void app_main(void)
{
    ESP_ERROR_CHECK(gpio_input_enable(SERVO_PIN));
    // esp_err_t pwm_success = pwm_output_init(SERVO_PIN, SERVO_CHANNEL, SERVO_TIMER,
                    // SERVO_FREQUENCY_HZ, SERVO_MAX_TICKS);
    const ledc_timer_config_t servo_timer_config = {
        .speed_mode = PWM_MODE,
        .duty_resolution = SERVO_RESOLUTION,
        .timer_num = SERVO_TIMER,
        .freq_hz = SERVO_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&servo_timer_config));

    const ledc_channel_config_t servo_channel_config = {
        .gpio_num = SERVO_PIN,
        .speed_mode = PWM_MODE,
        .channel = SERVO_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_TIMER,
        .duty = SERVO_MAX_TICKS,
        //.duty = SERVO_MAX_TICKS, // TODO
        .hpoint = 0,
    };
    esp_err_t pwm_success = ledc_channel_config(&servo_channel_config);
    ESP_LOGI(TAG, "PWM INIT_SUCCESS", pwm_success == ESP_OK);
    // esp_err_t led_success = pwm_output_init(LED_PIN, LED_CHANNEL, LED_TIMER, LED_FREQUENCY_HZ, 0);
    // ESP_LOGI(TAG, "LED INIT_SUCCESS", led_success == ESP_OK);
    const ledc_timer_config_t led_timer_config = {
        .speed_mode = PWM_MODE,
        .duty_resolution = LED_RESOLUTION,
        .timer_num = LED_TIMER,
        .freq_hz = LED_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&led_timer_config));

    const ledc_channel_config_t led_channel_config = {
        .gpio_num = LED_PIN,
        .speed_mode = PWM_MODE,
        .channel = LED_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_TIMER,
        .duty = LED_MAX_DUTY,
        //.duty = SERVO_MAX_TICKS, // TODO
        .hpoint = 0,
    };

    esp_err_t led_success = ledc_channel_config(&led_channel_config);
    // ESP_LOGI(TAG, "POT INIT_SUCCESS", led_success == ESP_OK);
// ?    esp_err_t pot_success = adc_inputs_init(adc_handle);

    
    const adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));

    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    esp_err_t pot_success = adc_oneshot_config_channel(adc_handle, POT_CHANNEL, &channel_config);

    ESP_LOGI(TAG, "POT GPIO4 -> MOTOR GPIO7: %u..%u%% PWM; LDR GPIO5 -> LED GPIO6",
             (unsigned)MOTOR_MIN_PERCENT, (unsigned)MOTOR_MAX_PERCENT);
    ESP_LOGI(TAG, "Motor %u Hz, LED %u Hz",
             (unsigned)ledc_get_freq(PWM_MODE, SERVO_TIMER),
             (unsigned)ledc_get_freq(PWM_MODE, LED_TIMER));
    // Static variables inside 
    static uint32_t led_duty = 0;
    static float current_angle_deg = 0.0f;
    TickType_t last_log = xTaskGetTickCount();
    // Servomotor     
    static TickType_t last_servo_step = 0;
    while (1) {
        // 1. UNCOMMENT
        int pot_value = 0;
        // int ldr_value = 0;
        esp_err_t pot_result = adc_oneshot_read(adc_handle, POT_CHANNEL, &pot_value);
        // esp_err_t ldr_result = adc_oneshot_read(adc_handle, LDR_CHANNEL, &ldr_value)
        // Потенціометер впливає також на двигун 
        led_duty = pot_result == ESP_OK ? pot_to_led_duty(pot_value) : 0;
        ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, LED_CHANNEL, led_duty));
        ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, LED_CHANNEL));
    
        
        TickType_t now = xTaskGetTickCount();
        if (now - last_servo_step >= pdMS_TO_TICKS(SERVO_STEP_PERIOD_MS)) {
            last_servo_step = now;
            current_angle_deg += SERVO_ANGLE_STEP_DEG;
            esp_err_t set_success = servo_set_angle_deg(current_angle_deg);
            ESP_LOGI(TAG, "SERVO SET SUCCESS=%d", set_success == ESP_OK);
        }
        // 2. TEST UNCOMMENT
        // Фоторезистор впливає лише на LED: темніше -> яскравіше.
        // uint32_t led_duty = ldr_result == ESP_OK ? ldr_to_led_duty(ldr_value) : 0;
        // ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, LED_CHANNEL, led_duty));
        // ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, LED_CHANNEL));

        if ((TickType_t)(now - last_log) >= pdMS_TO_TICKS(LOG_PERIOD_MS)) {
            // Log without LDR, just potent
            ESP_LOGI(TAG, "POT_GPIO4=%d, SERVO_ANGLE (%u%% deg)/180, GPIO7_HIGH=%u/256, LED_GPIO6=%u/1023",
                pot_value, current_angle_deg, (unsigned)led_duty);
            if (pot_result != ESP_OK) {
                ESP_LOGW(TAG, "POT ADCread failed: %s", esp_err_to_name(pot_result));
            }
            // 2. LDR TEST UNCOMMENT 
            // if (ldr_result != ESP_OK) {
            //     ESP_LOGW(TAG, "LDR ADC read failed: %s", esp_err_to_name(ldr_result));
            // }
            last_log = now;
        }
        vTaskDelay(pdMS_TO_TICKS(UPDATE_PERIOD_MS));
    }
}