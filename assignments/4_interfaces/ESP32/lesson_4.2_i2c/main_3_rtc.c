/**
 * main_3_rtc.c — DS1307 RTC read demo over I2C (ESP-IDF v5.x new driver)
 *
 * Stripped-down version of main_hw.c: only RTC init, probe, and
 * repeated 1 Hz read of all 7 time registers, logged via ESP_LOGI.
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "RTC";

/* ======================== I2C bus config ======================== */
#define I2C_SDA_GPIO        6
#define I2C_SCL_GPIO        7

/* ======================== DS1307 config ========================= */
#define DS1307_ADDR         0x68          // 7-bit I2C address
#define I2C_RTC_FREQ_HZ     100000        // DS1307 max 100 kHz
#define I2C_TIMEOUT_MS       100
#define TIME_BYTES           7            // sec, min, hr, day, date, month, year
#define ADDR_LENGTH          I2C_ADDR_BIT_LEN_7

/* ======================== RTC handle =========================== */
static i2c_master_dev_handle_t rtc;
static uint8_t timebuf[TIME_BYTES];       // receive buffer for 7 time registers

/* ======================== RTC helpers ========================== */

/**
 * Read `len` bytes starting from register `reg`.
 * Uses a single write-then-read transaction (repeated START).
 *
 * Wire: START → 0xD0(W) → reg → REP.START → 0xD1(R) → [len bytes] → STOP
 */
static esp_err_t rtc_read(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(
        rtc,
        &reg, 1,            // write phase: register pointer
        data, len,           // read phase
        I2C_TIMEOUT_MS
    );
}

/**
 * Write `len` bytes starting from register embedded in data[0].
 * data[] = { reg_addr, byte0, byte1, ... }
 *
 * Wire: START → 0xD0(W) → reg → byte0 → byte1 → ... → STOP
 */
static esp_err_t rtc_write(const uint8_t *data, size_t len)
{
    return i2c_master_transmit(rtc, data, len, I2C_TIMEOUT_MS);
}

/**
 * BCD → decimal helper (DS1307 stores time in BCD format)
 */
static inline uint8_t bcd2dec(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

/* ======================== app_main ============================= */
void app_main(void)
{
    /* ---------- 1. Create I2C master bus ---------- */
    i2c_master_bus_config_t bus_cfg = {
        .clk_source              = I2C_CLK_SRC_DEFAULT,
        .i2c_port                = I2C_NUM_0,
        .sda_io_num              = I2C_SDA_GPIO,
        .scl_io_num              = I2C_SCL_GPIO,
        .glitch_ignore_cnt       = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    /* ---------- 2. Probe DS1307 ---------- */
    ESP_ERROR_CHECK(i2c_master_probe(bus, DS1307_ADDR, I2C_TIMEOUT_MS));
    ESP_LOGI(TAG, "DS1307 found at 0x%02X", DS1307_ADDR);

    /* ---------- 3. Register RTC as device on the bus ---------- */
    i2c_device_config_t rtc_cfg = {
        .dev_addr_length = ADDR_LENGTH,
        .device_address  = DS1307_ADDR,
        .scl_speed_hz    = I2C_RTC_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &rtc_cfg, &rtc));

    /* ---------- 4. Check Clock Halt bit (reg 0x00, bit 7) ---------- */
    uint8_t sec_reg;
    ESP_ERROR_CHECK(rtc_read(0x00, &sec_reg, 1));

    if (sec_reg & 0x80) {
        // Oscillator is stopped — clear CH bit to start the clock
        // (leaves seconds at 0, you'd normally set the full time here)
        ESP_LOGW(TAG, "Clock was halted — starting oscillator");
        uint8_t start[] = { 0x00, sec_reg & 0x7F };   // reg 0x00, clear bit 7
        ESP_ERROR_CHECK(rtc_write(start, sizeof(start)));
    } else {
        ESP_LOGI(TAG, "Clock is running");
    }

    /* ---------- 5. Read loop — poll every 1 s ---------- */
    while (1) {
        esp_err_t err = rtc_read(0x00, timebuf, TIME_BYTES);

        if (err == ESP_OK) {
            uint8_t sec   = bcd2dec(timebuf[0] & 0x7F); // mask CH bit
            uint8_t min   = bcd2dec(timebuf[1]);
            uint8_t hr    = bcd2dec(timebuf[2] & 0x3F); // 24-hr mask
            uint8_t dow   = timebuf[3];                  // 1-7
            uint8_t date  = bcd2dec(timebuf[4]);
            uint8_t month = bcd2dec(timebuf[5]);
            uint8_t year  = bcd2dec(timebuf[6]);

            ESP_LOGI(TAG, "%02u:%02u:%02u  %02u/%02u/20%02u  (day %u)",
                     hr, min, sec, date, month, year, dow);
        } else {
            ESP_LOGE(TAG, "RTC read failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
