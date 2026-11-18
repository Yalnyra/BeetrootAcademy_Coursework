/**
 * @file main_5_12hr_menu_lvgl.c
 * @brief DS1307 RTC clock with 12h/24h format selector via LVGL roller + encoder.
 *
 * Hardware
 * --------
 *  SSD1306 128×64 OLED  — I2C addr 0x3C, shared bus GPIO 6/7
 *  DS1307 RTC           — I2C addr 0x68, same bus (100 kHz limit)
 *  Rotary encoder       — CLK=GPIO15, DT=GPIO16, SW=GPIO17 (active-low)
 *
 * UX flow
 * -------
 *  Power-on  → Clock screen (24h default)
 *  SW press  → Settings screen: "TIME FORMAT" + roller (24H / 12H)
 *              Encoder scroll selects; SW press confirms and returns.
 *
 * Build
 * -----
 *  In CMakeLists.txt set:
 *      SRCS "main_5_12hr_menu_lvgl.c"
 *      REQUIRES esp_driver_gpio esp_driver_i2c esp_timer log freertos encoder
 *
 * Encoder enable/disable
 * ----------------------
 *  Set CONFIG_ENCODER_ENABLED to 1 to activate the rotary encoder.
 *  Set to 0 (default) to compile without encoder — the settings screen is
 *  still built and reachable in the future, but no PCNT or GPIO is touched.
 *
 *  Root cause of lv_indev_create / lv_indev_set_type errors
 *  ---------------------------------------------------------
 *  Those functions are LVGL 9.x API.  This project uses LVGL 8.4.0.
 *  In LVGL 8 an input device is registered via the driver struct pattern:
 *
 *      lv_indev_drv_t drv;
 *      lv_indev_drv_init(&drv);
 *      drv.type    = LV_INDEV_TYPE_ENCODER;
 *      drv.read_cb = my_read_cb;
 *      lv_indev_t *indev = lv_indev_drv_register(&drv);
 *
 *  The 9.x setter functions (lv_indev_create, lv_indev_set_type,
 *  lv_indev_set_read_cb) do not exist in 8.x and cause the errors seen.
 *  The encoder block below uses the correct LVGL 8 API.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

/* ------------------------------------------------------------------ */
/*  Feature flag — set to 1 when the encoder hardware is wired up      */
/* ------------------------------------------------------------------ */
#define CONFIG_ENCODER_ENABLED  0   /* 0 = encoder OFF (safe default)  */
                                    /* 1 = encoder ON  (LVGL 8 API)    */

#if CONFIG_ENCODER_ENABLED
#include "encoder.h"
#endif

static const char *TAG = "clock5";

/* ------------------------------------------------------------------ */
/*  Pin / bus config                                                    */
/* ------------------------------------------------------------------ */
#define I2C_PORT        I2C_NUM_0
#define PIN_SDA         GPIO_NUM_6
#define PIN_SCL         GPIO_NUM_7
#define I2C_FREQ_HZ     100000      /* DS1307 max 100 kHz */

#define OLED_ADDR       0x3C
#define OLED_W          128
#define OLED_H          64

#define DS1307_ADDR     0x68
#define DS1307_REG_SEC  0x00

#define ENC_A_GPIO      GPIO_NUM_15
#define ENC_B_GPIO      GPIO_NUM_16
#define ENC_SW_GPIO     GPIO_NUM_17

/* ------------------------------------------------------------------ */
/*  Roller option indices                                               */
/* ------------------------------------------------------------------ */
#define FMT_IDX_24H     0
#define FMT_IDX_12H     1

/* ------------------------------------------------------------------ */
/*  Globals                                                             */
/* ------------------------------------------------------------------ */
static i2c_master_bus_handle_t  s_bus;
static i2c_master_dev_handle_t  s_rtc;
#if CONFIG_ENCODER_ENABLED
static encoder_t                s_enc;
#endif

/* LVGL UI objects */
static lv_obj_t *s_scr_clock;      /* clock face screen               */
static lv_obj_t *s_scr_settings;   /* format-picker screen            */
static lv_obj_t *s_lbl_time;       /* HH:MM:SS (or HH:MM AM/PM)       */
static lv_obj_t *s_lbl_date;       /* Www DD.MM.YYYY                  */
static lv_obj_t *s_roller_fmt;     /* 24H / 12H roller                */

/* State */
static bool      s_use_24h      = true;   /* current format choice    */
static bool      s_in_settings  = false;  /* which screen is active   */
static bool      s_btn_prev     = false;  /* last button level        */

/* ================================================================== */
/*  BCD helpers                                                         */
/* ================================================================== */
static inline uint8_t bcd2bin(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }
static inline uint8_t bin2bcd(uint8_t v) { return ((v / 10) << 4) | (v % 10); }

/* ================================================================== */
/*  I2C bus                                                             */
/* ================================================================== */
static void i2c_bus_init(void)
{
    i2c_master_bus_config_t cfg = {
        .i2c_port                = I2C_PORT,
        .sda_io_num              = PIN_SDA,
        .scl_io_num              = PIN_SCL,
        .clk_source              = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt       = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &s_bus));
}

/* ================================================================== */
/*  DS1307 RTC                                                          */
/* ================================================================== */
static esp_err_t rtc_write_time(const struct tm *t)
{
    uint8_t buf[8] = {
        DS1307_REG_SEC,
        bin2bcd(t->tm_sec)  & 0x7F,    /* CH=0  → oscillator running  */
        bin2bcd(t->tm_min),
        bin2bcd(t->tm_hour) & 0x3F,    /* bit6=0 → 24-hour mode       */
        bin2bcd(t->tm_wday + 1),
        bin2bcd(t->tm_mday),
        bin2bcd(t->tm_mon  + 1),
        bin2bcd(t->tm_year - 100),      /* 2000-based                  */
    };
    return i2c_master_transmit(s_rtc, buf, sizeof(buf), 100);
}

/**
 * Read 7 timekeeping registers from DS1307.
 * Returns ESP_ERR_INVALID_STATE when the oscillator is halted (CH=1).
 */
static esp_err_t rtc_read_time(struct tm *t)
{
    uint8_t reg = DS1307_REG_SEC;
    uint8_t d[7];

    esp_err_t err = i2c_master_transmit(s_rtc, &reg, 1, 200);
    if (err != ESP_OK) return err;

    err = i2c_master_receive(s_rtc, d, sizeof(d), 200);
    if (err != ESP_OK) return err;

    if (d[0] & 0x80) return ESP_ERR_INVALID_STATE;  /* CH bit set */

    memset(t, 0, sizeof(*t));
    t->tm_sec  = bcd2bin(d[0] & 0x7F);
    t->tm_min  = bcd2bin(d[1] & 0x7F);
    t->tm_hour = bcd2bin(d[2] & 0x3F); /* always 24h from chip */
    t->tm_mday = bcd2bin(d[4] & 0x3F);
    t->tm_mon  = bcd2bin(d[5] & 0x1F) - 1;
    t->tm_year = bcd2bin(d[6]) + 100;

    /* Recompute wday / yday */
    struct tm tmp = *t;
    tmp.tm_isdst = -1;
    mktime(&tmp);
    t->tm_wday = tmp.tm_wday;
    t->tm_yday = tmp.tm_yday;
    return ESP_OK;
}

static void rtc_set_from_build_time(void)
{
    static const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char mon[4] = {0};
    int d, y, hh, mm, ss;
    sscanf(__DATE__, "%3s %d %d", mon, &d, &y);
    sscanf(__TIME__, "%d:%d:%d", &hh, &mm, &ss);

    struct tm t = {
        .tm_sec  = ss, .tm_min = mm, .tm_hour = hh,
        .tm_mday = d,
        .tm_mon  = (strstr(months, mon) - months) / 3,
        .tm_year = y - 1900,
        .tm_isdst = -1,
    };
    mktime(&t);
    ESP_ERROR_CHECK(rtc_write_time(&t));
    ESP_LOGW(TAG, "RTC halted — set from build time: %s %s", __DATE__, __TIME__);
}

static void rtc_init(void)
{
    i2c_device_config_t dev = {
        .dev_addr_length  = I2C_ADDR_BIT_LEN_7,
        .device_address   = DS1307_ADDR,
        .scl_speed_hz     = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_bus, &dev, &s_rtc));

    struct tm t;
    if (rtc_read_time(&t) == ESP_ERR_INVALID_STATE) {
        rtc_set_from_build_time();
    }
}

/* ================================================================== */
/*  Encoder LVGL input-device — LVGL 8.x API                           */
/*  Guarded by CONFIG_ENCODER_ENABLED.  When 0 this whole block is     */
/*  compiled out; the button GPIO is also left unconfigured.            */
/* ================================================================== */
#if CONFIG_ENCODER_ENABLED

/* LVGL 8 requires a static driver struct that outlives registration */
static lv_indev_drv_t s_enc_drv;

static void encoder_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->enc_diff = (int16_t)encoder_update_ticks(&s_enc);
    data->state    = gpio_get_level(ENC_SW_GPIO) == 0
                     ? LV_INDEV_STATE_PRESSED
                     : LV_INDEV_STATE_RELEASED;
}

static void encoder_indev_init(void)
{
    /* SW button: input, pull-up, no interrupt */
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << ENC_SW_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);

    /* PCNT quadrature encoder */
    encoder_config_t enc_cfg = {
        .gpio_a            = ENC_A_GPIO,
        .gpio_b            = ENC_B_GPIO,
        .debug_gpio        = GPIO_NUM_NC,
        .quadrature_x4     = true,
        .counts_per_detent = 4,
        .glitch_filter_ns  = 1000,
        .high_limit        = 30000,
        .low_limit         = -30000,
    };
    encoder_init(&s_enc, &enc_cfg);

    /* LVGL 8 input-device registration — driver struct pattern */
    lv_indev_drv_init(&s_enc_drv);
    s_enc_drv.type    = LV_INDEV_TYPE_ENCODER;
    s_enc_drv.read_cb = encoder_read_cb;
    lv_indev_t *indev = lv_indev_drv_register(&s_enc_drv);

    /* Default group: roller on settings screen auto-registers itself */
    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(indev, g);
}

#endif /* CONFIG_ENCODER_ENABLED */

/* ================================================================== */
/*  Display init (SSD1306 via esp_lcd + esp_lvgl_port)                  */
/* ================================================================== */
static lv_disp_t *display_init(void)
{
    /* I2C panel IO */
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_i2c_config_t io_cfg = {
        .dev_addr            = OLED_ADDR,
        .scl_speed_hz        = I2C_FREQ_HZ,
        .control_phase_bytes = 1,
        .dc_bit_offset       = 6,
        .lcd_cmd_bits        = 8,
        .lcd_param_bits      = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(s_bus, &io_cfg, &io));

    /* SSD1306 panel driver */
    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_ssd1306_config_t ssd_cfg = { .height = OLED_H };
    esp_lcd_panel_dev_config_t panel_cfg = {
        .bits_per_pixel  = 1,
        .reset_gpio_num  = GPIO_NUM_NC,
        .vendor_config   = &ssd_cfg,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    /* LVGL port */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle    = io,
        .panel_handle = panel,
        .buffer_size  = OLED_W * OLED_H,
        .double_buffer = true,
        .hres         = OLED_W,
        .vres         = OLED_H,
        .monochrome   = true,
        .rotation     = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
    };
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);
    ESP_ERROR_CHECK(disp ? ESP_OK : ESP_FAIL);
    return disp;
}

/* ================================================================== */
/*  UI creation — two screens                                           */
/* ================================================================== */
static void ui_create(lv_disp_t *disp)
{
    lvgl_port_lock(0);

    /* ── Screen 1: Clock face ─────────────────────────────────────── */
    s_scr_clock = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_scr_clock, lv_color_black(), 0);
    lv_obj_set_style_bg_opa (s_scr_clock, LV_OPA_COVER, 0);

    s_lbl_time = lv_label_create(s_scr_clock);
    lv_obj_set_style_text_font (s_lbl_time, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(s_lbl_time, lv_color_white(), 0);
    lv_label_set_text(s_lbl_time, "--:--:--");
    lv_obj_align(s_lbl_time, LV_ALIGN_TOP_MID, 0, 6);

    s_lbl_date = lv_label_create(s_scr_clock);
    lv_obj_set_style_text_font (s_lbl_date, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_lbl_date, lv_color_white(), 0);
    lv_label_set_text(s_lbl_date, "--- --.--.----");
    lv_obj_align(s_lbl_date, LV_ALIGN_BOTTOM_MID, 0, -6);

    /* Small hint so the user knows a button does something */
    lv_obj_t *lbl_hint = lv_label_create(s_scr_clock);
    lv_obj_set_style_text_font (lbl_hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_hint, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_label_set_text(lbl_hint, "[SET]");
    lv_obj_align(lbl_hint, LV_ALIGN_BOTTOM_RIGHT, -2, -6);

    /* ── Screen 2: Settings (format picker) ──────────────────────── */
    s_scr_settings = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_scr_settings, lv_color_black(), 0);
    lv_obj_set_style_bg_opa (s_scr_settings, LV_OPA_COVER, 0);

    /* Title */
    lv_obj_t *lbl_title = lv_label_create(s_scr_settings);
    lv_obj_set_style_text_font (lbl_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_white(), 0);
    lv_label_set_text(lbl_title, "TIME FORMAT");
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 4);

    /* Roller: "24H\n12H", normal (non-wrapping) mode */
    s_roller_fmt = lv_roller_create(s_scr_settings);
    lv_roller_set_options(s_roller_fmt, "24H\n12H", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(s_roller_fmt, 2);   /* show both rows */
    lv_obj_set_width(s_roller_fmt, 56);
    lv_obj_align(s_roller_fmt, LV_ALIGN_CENTER, 0, 8);

    /* Monochrome style: white unselected, inverted selected */
    lv_obj_set_style_text_font (s_roller_fmt, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_roller_fmt, lv_color_white(),       LV_PART_MAIN);
    lv_obj_set_style_bg_color  (s_roller_fmt, lv_color_black(),       LV_PART_MAIN);
    lv_obj_set_style_bg_opa    (s_roller_fmt, LV_OPA_COVER,           LV_PART_MAIN);
    lv_obj_set_style_border_width(s_roller_fmt, 1,                    LV_PART_MAIN);
    lv_obj_set_style_border_color(s_roller_fmt, lv_color_white(),     LV_PART_MAIN);

    lv_obj_set_style_bg_color  (s_roller_fmt, lv_color_white(), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa    (s_roller_fmt, LV_OPA_COVER,     LV_PART_SELECTED);
    lv_obj_set_style_text_color(s_roller_fmt, lv_color_black(), LV_PART_SELECTED);

    /* Confirm hint */
    lv_obj_t *lbl_ok = lv_label_create(s_scr_settings);
    lv_obj_set_style_text_font (lbl_ok, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_ok, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_label_set_text(lbl_ok, "[OK]");
    lv_obj_align(lbl_ok, LV_ALIGN_BOTTOM_RIGHT, -2, -4);

    /* Start on the clock screen */
    lv_disp_load_scr(s_scr_clock);

    lvgl_port_unlock();
}

/* ================================================================== */
/*  Clock face update                                                   */
/* ================================================================== */
static void display_update(const struct tm *t)
{
    char time_str[16];
    char date_str[24];

    if (s_use_24h) {
        strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
    } else {
        /* "%I:%M %p" fits 128 px with Montserrat 24; drop seconds in 12h */
        strftime(time_str, sizeof(time_str), "%I:%M %p", t);
    }
    strftime(date_str, sizeof(date_str), "%a %d.%m.%Y", t);

    if (lvgl_port_lock(100)) {
        lv_label_set_text(s_lbl_time, time_str);
        lv_label_set_text(s_lbl_date, date_str);
        lvgl_port_unlock();
    }
}

/* ================================================================== */
/*  Button edge detection + screen switch                               */
/* ================================================================== */
static void handle_button(void)
{
#if !CONFIG_ENCODER_ENABLED
    /* Encoder disabled — button pin not configured, nothing to do */
    return;
#endif
    bool btn_now = (gpio_get_level(ENC_SW_GPIO) == 0);

    if (btn_now && !s_btn_prev) {           /* falling edge = press */
        if (s_in_settings) {
            /* ── Confirm: read roller → save → return to clock ── */
            if (lvgl_port_lock(100)) {
                uint16_t sel = lv_roller_get_selected(s_roller_fmt);
                s_use_24h    = (sel == FMT_IDX_24H);
                lv_disp_load_scr(s_scr_clock);
                lvgl_port_unlock();
            }
            s_in_settings = false;
            ESP_LOGI(TAG, "Format saved: %s", s_use_24h ? "24H" : "12H");
        } else {
            /* ── Enter settings: sync roller to current choice ── */
            if (lvgl_port_lock(100)) {
                lv_roller_set_selected(s_roller_fmt,
                                       s_use_24h ? FMT_IDX_24H : FMT_IDX_12H,
                                       LV_ANIM_OFF);
                lv_disp_load_scr(s_scr_settings);
                lvgl_port_unlock();
            }
            s_in_settings = true;
        }
    }

    s_btn_prev = btn_now;
}

/* ================================================================== */
/*  app_main                                                            */
/* ================================================================== */
void app_main(void)
{
    /* ── Hardware init ── */
    i2c_bus_init();
    rtc_init();

    /* ── LVGL display ── */
    lv_disp_t *disp = display_init();

    /* ── Encoder + LVGL input device (call after lvgl_port_init) ── */
#if CONFIG_ENCODER_ENABLED
    encoder_indev_init();
#endif

    /* ── Build UI ── */
    ui_create(disp);

    /* ── Main loop ── */
    struct tm now, last = { .tm_sec = -1 };

    while (1) {
        handle_button();

        /* Only update the clock face when it is visible */
        if (!s_in_settings) {
            esp_err_t err = rtc_read_time(&now);
            if (err == ESP_OK) {
                if (now.tm_sec != last.tm_sec) {
                    display_update(&now);
                    last = now;
                }
            } else {
                ESP_LOGE(TAG, "RTC read error: %s", esp_err_to_name(err));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); /* 20 Hz — fast enough for button debounce */
    }
}
