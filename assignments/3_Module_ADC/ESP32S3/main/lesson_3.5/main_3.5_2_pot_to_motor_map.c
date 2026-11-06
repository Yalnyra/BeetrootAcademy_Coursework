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


//CONST
#define ROUND_UP_CORRECTION 0.5f
#define SMA_WINDOW_SIZE 10u

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
#define LED_MAX_PERCENT    100U

// Servo control
#define ADC_MIN_VALUE       0U
#define ADC_MAX_VALUE       4095u
#define POT_ERROR_TRESHOLD (ADC_MAX_VALUE / 100U)

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
#define LED_PERIOD_TICKS    1024U
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
// Servo attitude control via potentiometer.
#define POT_MIN_RANGE_DEG 0.0f 
#define POT_MAX_RANGE_DEG    270.0f   // 315.0f -> 270 deg of real pot travel
// SG90 datasheet speed is ~0.1 s / 60° under no load — commanding angle
// updates faster than that doesn't make it rotate faster, it just drives
// the motor against its own inertia/gearing (extra current, jitter, wear).
#define SERVO_UPDATE_PERIOD_MS   60U    // pot at 0 -> slowest

#define SERVO_MAX_RANGE_DEG  180.0f
#define SERVO_MIN_RANGE_DEG 0.0f
// Pulse widths expressed as duty ticks (rounded to nearest tick, same style as MOTOR_MIN/MAX_TICKS)
#define SERVO_MIN_TICKS \
    ((SERVO_MIN_PULSE_US * SERVO_PERIOD_TICKS + SERVO_PERIOD_US / 2U) / SERVO_PERIOD_US)
#define SERVO_MAX_TICKS \
    ((SERVO_MAX_PULSE_US * SERVO_PERIOD_TICKS + SERVO_PERIOD_US / 2U) / SERVO_PERIOD_US)

// #define SERVO_STEP_PERIOD_MS  2000U
#define LOG_PERIOD_MS       500
#define UPDATE_PERIOD_MS    20

_Static_assert(POT_MIN_RANGE_DEG <= POT_MAX_RANGE_DEG && POT_MAX_RANGE_DEG <= 360U,
               "Invalid potentiometer rotation range");



static const char *TAG = "hw_3_5";
static adc_oneshot_unit_handle_t adc_handle;
// logged values
typedef struct {
    float readings[SMA_WINDOW_SIZE]; 
    float sum;             //ADC value 
    float index;            // the last sample in the window 
} pot_status_t;
typedef struct {
    float    angle_deg;   // clamped servo-space angle actually commanded
    int duty_ticks;       // raw LEDC duty value written
    float    pulse_width_ms;   // same duty, expressed as a pulse width
    float    angle_dev_deg;   // angle deviation from pot space to angle space 
    float    achieved_angle_deg; // angle implied by the integer duty actually written
    float    deviation_deg;      // angle_deg - achieved_angle_deg (quantization error)
} servo_status_t;



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
        .freq_hz = frequency_hz,
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
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, adc_handle));

    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    return adc_oneshot_config_channel(*adc_handle, POT_CHANNEL,
                                             &channel_config);
}

// Utils 
static float map_valuef(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// TODO Update based on degree delta, if delta of potentiometer is larger than x

static float calc_deviation_percentf(float one, float other){
      return (one > 0)
                  ? (one - other) / other * 100.0f
                  : 0.0f; 
}


// Map raw ADC (0..4095) to the potentiometer's real mechanical range in degrees.
// The conversion here is correct 
static float pot_to_servo_degrees(int value)
{
    if (value < (int)ADC_MIN_VALUE) {
        value = (int)ADC_MIN_VALUE;
    } else if (value > (int)ADC_MAX_VALUE) {
        value = (int)ADC_MAX_VALUE;
    }

    return map_valuef((float)value, (float)ADC_MIN_VALUE, (float)ADC_MAX_VALUE,
                       POT_MIN_RANGE_DEG, POT_MAX_RANGE_DEG);
}

// Inverse of the angle -> duty conversion in servo_set_angle_deg
static float duty_to_servo_angle_deg(uint32_t duty)
{
    return map_valuef((float)duty,
                       (float)SERVO_MIN_TICKS, (float)SERVO_MAX_TICKS,
                       SERVO_MIN_RANGE_DEG, SERVO_MAX_RANGE_DEG);
}


// TODO: Change the LED brightness range 
static float pot_to_led_duty(int value)
{
    if (value < 0) {
        value = 0;
    } else if (value > ADC_MAX_VALUE) {
        value = ADC_MAX_VALUE;
    }
    const uint32_t span = LED_MAX_TICKS - LED_MIN_TICKS;
    return LED_MIN_TICKS + ((uint32_t)value * span + ADC_MAX_VALUE / 2U) / ADC_MAX_VALUE;
}

// // TODO the mapping of angle_deg to is not 1-to-1 - its rescaled!!
// Clamp instead of rescaling!!

esp_err_t servo_set_angle_deg(float angle_deg, servo_status_t *status)
{
    float cmd_angle_deg = angle_deg;
    if (cmd_angle_deg < SERVO_MIN_RANGE_DEG) {
        cmd_angle_deg = SERVO_MIN_RANGE_DEG;
    } else if (cmd_angle_deg > SERVO_MAX_RANGE_DEG) {
        cmd_angle_deg = SERVO_MAX_RANGE_DEG;
    }

    // Mapping to number ticks per second (0-8192)
    float ratio = (cmd_angle_deg - SERVO_MIN_RANGE_DEG) /
                  (SERVO_MAX_RANGE_DEG - SERVO_MIN_RANGE_DEG);

    uint32_t duty_ticks = SERVO_MIN_TICKS +
        (uint32_t)(ratio * (float)(SERVO_MAX_TICKS - SERVO_MIN_TICKS) + ROUND_UP_CORRECTION);

    float achieved_angle_deg = duty_to_servo_angle_deg(duty_ticks);
    // Include the clamped range
    float angle_dev_deg = calc_deviation_percentf(angle_deg, achieved_angle_deg);

    if (status != NULL) {
        status->angle_deg      = cmd_angle_deg;
        status->duty_ticks     = duty_ticks;
        // Mapping to ms (0-2ms)
        status->pulse_width_ms = (float)duty_ticks * SERVO_PERIOD_US / SERVO_PERIOD_TICKS / 1000.0f;
        status->achieved_angle_deg = achieved_angle_deg;
        status->angle_dev_deg = angle_dev_deg;
        status->deviation_deg = cmd_angle_deg - achieved_angle_deg;
    }
    // Requires duty cycle, not time ms 
    esp_err_t err = ledc_set_duty(PWM_MODE, SERVO_CHANNEL, duty_ticks);
    if (err != ESP_OK) {
        return err;
    }
    return ledc_update_duty(PWM_MODE, SERVO_CHANNEL);
}
// pot_status_t pot_status = {0};
servo_status_t servo_status = {0};
void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    
    ESP_ERROR_CHECK(gpio_input_enable(SERVO_PIN));

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

    ESP_ERROR_CHECK(ledc_channel_config(&led_channel_config));
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

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POT_CHANNEL, &channel_config));
    
    // Static variables inside 
    static uint32_t led_duty = 0;
    TickType_t last_log = xTaskGetTickCount();
    // Servomotor     
    static TickType_t last_servo_step = 0;
    esp_err_t set_success = 0;
    esp_err_t pot_result = 0;
    while (1) {
        int current_pot_value = 0;
        float pot_angle = 0;
        // int ldr_value = 0;
        pot_result = adc_oneshot_read(adc_handle, POT_CHANNEL, &current_pot_value);
        // pot_status.pot_value (current_pot_value - pot_status.pot_value) <= POT_ERROR_TRESHOLD ? current_pot_value: pot_status.pot_value;
        current_pot_value = pot_result == ESP_OK ? current_pot_value : 0;
        pot_angle = pot_to_servo_degrees((float)current_pot_value);
        // Safeguard against invalid ADC read (>4095)
        led_duty = pot_result == ESP_OK ? pot_to_led_duty(pot_result) : 0;
        ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, LED_CHANNEL, led_duty));
        ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, LED_CHANNEL));

        TickType_t now = xTaskGetTickCount();
        if (now - last_servo_step >= pdMS_TO_TICKS(SERVO_UPDATE_PERIOD_MS)) {
            last_servo_step = now;
            pot_angle = pot_to_servo_degrees(current_pot_value);
            set_success = servo_set_angle_deg(pot_angle, &servo_status);
            
        }

        if ((TickType_t)(now - last_log) >= pdMS_TO_TICKS(LOG_PERIOD_MS)) {
            last_log = now;
            ESP_LOGI(TAG,
                    "POT_GPIO4=%d, POT_ANGLE=%.2f/%.2f deg, SERVO_ANGLE%.2f/%.2f deg,  ANGLE_ERROR=%.2f%%",
                    current_pot_value,
                    pot_angle, 
                    POT_MAX_RANGE_DEG,
                    servo_status.angle_deg, 
                    SERVO_MAX_RANGE_DEG,
                    servo_status.angle_dev_deg);

            ESP_LOGI(TAG,
                    "SERVO_DUTY=%d ticks, SERVO_PULSE=%.3fms, ",
                    
                    servo_status.duty_ticks,
                    servo_status.pulse_width_ms
                );

            if (set_success != ESP_OK) {
                ESP_LOGW(TAG, "servo_set_angle_deg failed: %s", esp_err_to_name(set_success));
            }
            if (pot_result != ESP_OK) {
                ESP_LOGW(TAG, "POT ADC read failed: %s", esp_err_to_name(pot_result));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(UPDATE_PERIOD_MS));
    }
}