#include "buzzer.h"

#include <esp_err.h>

void buzzer_init(buzzer_t *bz, const buzzer_config_t *cfg)
{
    bz->cfg = *cfg;

    ledc_timer_config_t timer_config = {
        .speed_mode      = cfg->mode,
        .timer_num       = cfg->timer,
        .duty_resolution = cfg->duty_res,
        .freq_hz         = cfg->init_freq_hz,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .speed_mode = cfg->mode,
        .channel    = cfg->channel,
        .timer_sel  = cfg->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = cfg->gpio,
        .duty       = 0,                        // start silent
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

void buzzer_emit_note(buzzer_t *bz, int32_t freq_hz, int32_t cycles)
{
    // TODO:
    //  - ledc_set_freq(bz->cfg.mode, bz->cfg.timer, freq_hz)
    //  - ledc_set_duty(bz->cfg.mode, bz->cfg.channel, bz->cfg.duty) + ledc_update_duty(...)
    //  - bz->cycles_left = cycles   (a new field of buzzer_t, shared with buzzer_update();
    //    re-arming while the note plays keeps the tone on as long as the user keeps turning)
    (void)bz;
    (void)freq_hz;
    (void)cycles;
}

void buzzer_update(buzzer_t *bz)
{
    // TODO:
    //  - if bz->cycles_left > 0: decrement it
    //  - when it reaches 0: ledc_set_duty(bz->cfg.mode, bz->cfg.channel, 0) + ledc_update_duty(...)
    (void)bz;
}
