#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"     
#include "soc/adc_channel.h"
#include "esp_log.h"

#define ADC_UNIT     ADC_UNIT_1        // АЦП1 
#define ADC_CHANNEL  ADC_CHANNEL_5     // GPIO6
#define ADC_RANGE 4095.0 // Max 12-bit ADC range
#define ADC_NAIVE_INPUT 3300.0 // Assumes input 3.3V 
static const char *TAG = "ADC_LAB";

/** 
Підключити потенціометр (~10 кОм) у вигляді подільника напруги.
візьміть до уваги опорну напругу, атенюацію та розрядність. Можна додати їх до таблиці також для інформації.
Налаштувати АЦП та зчитувати значення кожні ~100 мс.
Зробити кілька вимірювань. Для кожного вимірювання:
зчитати RAW значення АЦП та обчислити напругу за формулою
зчитати напругу за допомогою каліброваного АЦП;
обчислити похибку (%) між обчисленою та каліброваною напругою.
 */
/* *
Needed components:
- Thermistor 10k
- LDR - (Was stuck somewhere with the resistor bag )
- Lower & upper bounds should be outside moving average standart deviation
*/

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;               // хендл блока АЦП1 — потрібен для всіх подальших викликів
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);    // реєструємо блок АЦП1

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,                // типова роздільна здатність (12 біт на класичному ESP32)
        .atten    = ADC_ATTEN_DB_12,                      // дозволяє читати повний діапазон ~0-3.3В (детально — Крок 2)
    };

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT,
        .chan = ADC_CHANNEL,
        .bitwidth = ADC_BITWIDTH_DEFAULT,                // типова роздільна здатність (12 біт на класичному ESP32)
        .atten    = ADC_ATTEN_DB_12,                      // дозволяє читати повний діапазон ~0-3.3В (детально — Крок 2)
    };

    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_config);

    adc_cali_handle_t cali_handle = 0;  
    esp_err_t err = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);

    // ... after creating the scheme:
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Calibration init failed: %s", esp_err_to_name(err));
        cali_handle = NULL;
    }
    // ESP_LOGI(TAG, cali_handle)

    while (1) {
        int raw = 0;
        adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw); // ОДИН замір: дискретизація + квантування + кодування за один виклик

         int u_cali_mv = 0;
    if (cali_handle) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &u_cali_mv));
    }

    float u_naive_mv = (raw / ADC_RANGE) * ADC_NAIVE_INPUT;
    // Handle the new
    float err_pct = (u_cali_mv > 0)
                  ? (u_naive_mv - u_cali_mv) / (float)u_cali_mv * 100.0f
                  : 0.0f;

        ESP_LOGI(TAG, "RAW %4d | U_naive %7.1f mV | U_cali %4d mV | Error %+6.2f %%",
             raw, u_naive_mv, u_cali_mv, err_pct);

        // printf(">raw:%d\n>u_naive_mv:%.1f\n>u_cali_mv:%d\n>error_pct:%.2f\n",
        //    raw, u_naive_mv, u_cali_mv, err_pct);     

        vTaskDelay(pdMS_TO_TICKS(200));                   // пауза 200 мс — щоб монітор встигав, і задача не займала весь CPU
    }
}