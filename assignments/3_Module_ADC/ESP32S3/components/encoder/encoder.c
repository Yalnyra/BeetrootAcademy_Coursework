#include "encoder.h"

#include <inttypes.h>
#include <stddef.h>

#include <esp_err.h>
#include <esp_log.h>

static const char *TAG = "encoder";

void encoder_init(encoder_t *enc, const encoder_config_t *cfg)
{
    enc->cfg              = *cfg;
    enc->unit             = NULL;
    enc->prev_count       = 0;
    enc->residual         = 0;
    enc->debug_prev_count = 0;

    // Межі лічильника (діапазон 16-бітного знакового числа)
    pcnt_unit_config_t unit_config = {
        .high_limit = cfg->high_limit,    // Верхня межа лічильника
        .low_limit  = cfg->low_limit,     // Нижня межа лічильника
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &enc->unit));

    // Апаратний фільтр завад: імпульси коротші за glitch_filter_ns ігноруються
    pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = cfg->glitch_filter_ns };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(enc->unit, &filter_config));

    if (cfg->quadrature_x4) {
        // X4 quadrature: two channels, each counts both edges of its own signal and
        // uses the other signal as direction. Contact bounce on either line then
        // produces +1/-1 pairs that cancel, unlike X1 (one channel, rising A only),
        // where bounce on A's FALLING edge creates extra rising edges that are
        // counted with B's current level — i.e. in the wrong direction.
        pcnt_channel_handle_t chan_a = NULL;
        pcnt_chan_config_t chan_a_config = {
            .edge_gpio_num  = cfg->gpio_a,
            .level_gpio_num = cfg->gpio_b,
        };
        ESP_ERROR_CHECK(pcnt_new_channel(enc->unit, &chan_a_config, &chan_a));

        pcnt_channel_handle_t chan_b = NULL;
        pcnt_chan_config_t chan_b_config = {
            .edge_gpio_num  = cfg->gpio_b,
            .level_gpio_num = cfg->gpio_a,
        };
        ESP_ERROR_CHECK(pcnt_new_channel(enc->unit, &chan_b_config, &chan_b));

        // Same sign as the old X1 setup (A rising while B high = +1).
        // If CW counts down, swap INCREASE/DECREASE in BOTH edge actions.
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE,     // A rising
            PCNT_CHANNEL_EDGE_ACTION_DECREASE));   // A falling
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,        // B high
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE));   // B low
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
            PCNT_CHANNEL_EDGE_ACTION_DECREASE,     // B rising
            PCNT_CHANNEL_EDGE_ACTION_INCREASE));   // B falling
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,        // A high
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE));   // A low

        // Bare encoders have no pull-ups; KY-040 boards do. Harmless either way.
        gpio_pullup_en(cfg->gpio_a);
        gpio_pullup_en(cfg->gpio_b);
    } else {
        // X1: канал рахує фронти сигналу A, а сигнал B визначає напрямок обертання
        pcnt_channel_handle_t chan_a = NULL;
        pcnt_chan_config_t chan_a_config = {
            .edge_gpio_num  = cfg->gpio_a,
            .level_gpio_num = cfg->gpio_b,
        };
        ESP_ERROR_CHECK(pcnt_new_channel(enc->unit, &chan_a_config, &chan_a));

        // Передній фронт A: +1, задній фронт A: ігнорувати
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE,
            PCNT_CHANNEL_EDGE_ACTION_HOLD));

        // B = 1: напрямок як є, B = 0: інвертувати
        // Якщо обертання враховується у зворотний бік, поміняйте KEEP та INVERSE місцями
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    }
    ESP_ERROR_CHECK(pcnt_unit_enable(enc->unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(enc->unit));
    ESP_ERROR_CHECK(pcnt_unit_start(enc->unit));

    if (cfg->quadrature_x4) {
        ESP_LOGI(TAG, "PCNT X4 initialized, %d counts per detent", cfg->counts_per_detent);
    } else {
        ESP_LOGI(TAG, "PCNT X1 initialized");
    }
}

void encoder_debug_init(encoder_t *enc)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << enc->cfg.debug_gpio,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    ESP_ERROR_CHECK(gpio_set_level(enc->cfg.debug_gpio, 0));
}

int32_t encoder_update_ticks(encoder_t *enc)
{
    int count = 0;
    if (pcnt_unit_get_count(enc->unit, &count) != ESP_OK) {
        return 0;                   // read failed: report no movement, keep prev_count
    }
    int32_t delta = count - enc->prev_count;    // raw PCNT counts (edges in X4)
    enc->prev_count = count;

    if (enc->cfg.quadrature_x4) {
        enc->residual += delta;
        int32_t clicks = enc->residual / enc->cfg.counts_per_detent;   // truncates toward 0: symmetric for +/-
        enc->residual -= clicks * enc->cfg.counts_per_detent;
        if (clicks != 0 ) {
            ESP_LOGD(TAG, "COUNT CHANGED BY %" PRId32, clicks);
        }
        return clicks;
    }
    if (delta > 0 ) {
        ESP_LOGD(TAG, "COUNT CHANGED BY %" PRId32, delta);
    }
    return delta;
}

/* Debug probe: mirrors the glitch-filtered PCNT count on cfg.debug_gpio.
 *
 * The ESP32-S3 PCNT cannot route its filtered input to a GPIO, so this reads
 * the count (which only moves on edges that survived the glitch filter)
 * and drives the pin from software, once per superloop cycle:
 *   count changed since last call -> HIGH (filtered edges arrived)
 *   count unchanged               -> LOW  (encoder at rest)
 * Keeps its own previous count, so it does not disturb encoder_update_ticks().
 *
 * Expected while the servo "runs away": pin LOW, raw count constant. That proves
 * the PCNT is quiet and the runaway is in software (absolute count re-added
 * every cycle), not noise or bounce getting past the filter. */
void encoder_output_ticks(encoder_t *enc)
{
    int count = 0;

    if (pcnt_unit_get_count(enc->unit, &count) != ESP_OK) {
        return;
    }

    int delta = count - enc->debug_prev_count;
    enc->debug_prev_count = count;

    gpio_set_level(enc->cfg.debug_gpio, delta != 0 ? 1 : 0);

    if (delta != 0) {
        ESP_LOGI(TAG, "PCNT filtered: count=%d delta=%+d", count, delta);
    }
}
