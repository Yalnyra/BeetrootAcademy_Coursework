#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"     
#include "esp_log.h"

#define ADC_UNIT     ADC_UNIT_1        // АЦП1 
#define ADC_CHANNEL  ADC_CHANNEL_5     // GPIO6
static const char *TAG = "ADC_LAB";

/** 
 * TODO translate to UA
 * Realize Moving average filter 
 * from stackoverflow issue:
 * https://stackoverflow.com/questions/10990618/calculate-rolling-moving-average-in-c
* You need a circular array (circular buffer) of 1000 elements, where you add the element to the previous element and store it.
* It becomes an increasing sum, where you can always get the sum between any two pairs of elements, and divide by the number of elements between them, to yield the average
 */
/* *
Needed components:
- Thermistor 10k
- LDR - (Was stuck somewhere with the resistor bag )
- Lower & upper bounds should be outside moving average standart deviation
*/


static inline uint32_t millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc1_handle;               // хендл блока АЦП1 — потрібен для всіх подальших викликів
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);    // реєструємо блок АЦП1

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,                // типова роздільна здатність (12 біт на класичному ESP32)
        .atten    = ADC_ATTEN_DB_12,                      // дозволяє читати повний діапазон ~0-3.3В (детально — Крок 2)
    };

    // adc_cali_curve_fitting_config_t = {
    //     .bitwidth = ADC_BITWIDTH_DEFAULT,                // типова роздільна здатність (12 біт на класичному ESP32)
    //     .atten    = ADC_ATTEN_DB_12,                      // дозволяє читати повний діапазон ~0-3.3В (детально — Крок 2)
    // }

    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &chan_config);
    adc_cali_check_scheme( )
    while (1) {
        int raw = 0;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL, &raw); // ОДИН замір: дискретизація + квантування + кодування за один виклик
        ESP_LOGI(TAG, "Сире значення: %d", raw);

        vTaskDelay(pdMS_TO_TICKS(200));                   // пауза 200 мс — щоб монітор встигав, і задача не займала весь CPU
    }
}