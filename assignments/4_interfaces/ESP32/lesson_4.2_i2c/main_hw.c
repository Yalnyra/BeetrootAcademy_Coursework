/** Scroll menu for encoder based control 
 * 
 */
// SSD1306 128x64 по I2C, ESP32-S3, ESP-IDF v5.x (новий драйвер i2c_master)
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "encoder.h"

static const char *TAG = "SSD1306";

#define NAME "SENSOR LOGGER"
#define VERSION "V1.01"
#define AUTHOR "BEETROOT ACADEMY"

// Налаштування апаратної шини I2C
#define I2C_SDA_GPIO   6      // Пін даних SDA (змініть під свою плату)
#define I2C_SCL_GPIO   7      // Пін тактування SCL

#define I2C_OLED_FREQ_HZ        400000 // Частота шини: 100 кГц (Standart Mode) - найвища частота, підтримувана дисплеєм
#define I2C_RTC_FREQ_HZ    100000 // Частота шини: 100 кГц (Standart Mode) - найвища частота, підтримувана таймером
#define SSD1306_ADDR   0x3C   // 7-бітна I2C-адреса дисплея (драйвер сам додасть біт R/W)

#define DS1307_ADDR 0x68 // I2C адреса таймера 
#define ADDR_LENGTH I2C_ADDR_BIT_LEN_7
#define TIME_BYTES 7 

// Роздільна здатність екрана та організація пам'яті
#define OLED_W 128            // Ширина дисплея в пікселях
#define OLED_H 64             // Висота дисплея в пікселях
#define OLED_PAGES (OLED_H / 8) // Кількість сторінок пам'яті (64 / 8 = 8 сторінок по 8 рядків пікселів)

// Font 5×7: glyph dimensions and horizontal advance
#define FONT_GLYPH_W       5     // Ширина одного символу (стовпців)
#define FONT_GLYPH_H       7     // Висота одного символу (рядків)
#define FONT_CHAR_ADVANCE   (FONT_GLYPH_W + 1)  // Крок між символами (гліф + 1 px пробіл)

// Menu item pixel layout
#define MENU_ICON_X         2     // X-відступ іконки від лівого краю
#define MENU_TEXT_X         12    // X-відступ тексту (після іконки + проміжок)

// I2C transaction timeouts (ms)
#define I2C_CMD_TIMEOUT_MS      100
#define I2C_FLUSH_TIMEOUT_MS    1000
#define I2C_PAGE_TIMEOUT_MS     200

// Max command payload for oled_cmds() scratch buffer
#define OLED_CMD_BUF_SIZE   16

// SSD1306 command bytes
#define SSD1306_CMD_DISPLAY_OFF     0xAE
#define SSD1306_CMD_DISPLAY_ON      0xAF
#define SSD1306_CMD_NORMAL_DISPLAY  0xA6
#define SSD1306_CMD_INVERSE_DISPLAY 0xA7
#define SSD1306_CMD_SET_COL_ADDR    0x21
#define SSD1306_CMD_SET_PAGE_ADDR   0x22
#define SSD1306_CMD_START_LINE_BASE 0x40  // OR з номером рядка: 0x40 | (line & 0x3F)
#define SSD1306_START_LINE_MASK     0x3F  // 6-бітна маска для номера рядка (0..63)

// E-ink style transition timing (ms)
#define EINK_FLASH_DURATION_MS  80   // Тривалість інверсійного спалаху
#define EINK_PAUSE_DURATION_MS  50   // Пауза перед завантаженням нового кадру

// Boot screen hold time (ms)
#define BOOT_SCREEN_HOLD_MS     2000

// Main loop encoder polling interval (ms)
#define ENCODER_POLL_MS         10

/* ============================== Encoder ============================== */


// Rotary encoder GPIO pins
#define ENC_A_GPIO     15     // CLK pin
#define ENC_B_GPIO     16     // DT pin
#define ENC_SW_GPIO    17     // Switch (button) pin — optional

/* Decoding mode, chosen at compile time:
 *   0 = X1: one channel, rising edge of A only (lesson 3.6). 1 count per click.
 *   1 = X4: two channels, both edges of A and B. 4 counts per click, bounce cancels in +1/-1 pairs.
 * Build with:  idf.py -DENCODER_X4=ON build   (OFF = X1, see main/CMakeLists.txt)
 * or change the default below. */
#ifndef ENCODER_QUADRATURE_X4
#define ENCODER_QUADRATURE_X4      1
#endif

#if ENCODER_QUADRATURE_X4
#define ENCODER_COUNTS_PER_DETENT  4   // X4: one click = full quadrature cycle = 4 edges (set 2 if one click gives delta=2)
#else
#define ENCODER_COUNTS_PER_DETENT  1   // X1: one click = one rising edge of A
#endif
#define GLITCH_FILTER_DELAY_NS 1000
#define ENCODER_PCNT_HIGH_LIMIT    32767    // Верхня межа лічильника
#define ENCODER_PCNT_LOW_LIMIT     -32768   // Нижня межа лічильника

/* ================ Encoder: Button  ================= */

#define DEBOUNCE_MS        30u
#define SHORT_TIME_LIMIT   800u   // hold longer than this → long press

// Керуючі байти протоколу SSD1306 (перший байт після адреси пристрою)
#define CTRL_CMD  0x00   // Co=0, D/C#=0 -> наступні байти є командами налаштування
#define CTRL_DATA 0x40   // Co=0, D/C#=1 -> наступні байти є графічними даними для відеопам'яті (GDDRAM)

static i2c_master_dev_handle_t oled;             // Дескриптор (handle) зареєстрованого пристрою на шині
static uint8_t fb[OLED_W * OLED_PAGES];          // Локальний кадровий буфер у RAM мікроконтролера (1024 байти)
static uint8_t tx[1 + OLED_W * OLED_PAGES];      // Буфер відправки по I2C: 1 керуючий байт (0x40) + 1024 байти кадру

// RTC handle 
i2c_master_dev_handle_t rtc;  // Дескриптор (handle) зареєстрованого пристрою на шині
static uint8_t time[TIME_BYTES];          // Буфер всього регістра в мікроконтролері 

// Rotary encoder handle
static encoder_t enc;
// ---------- низький рівень (обмін по I2C) ----------

// Відправка однієї однобайтової команди дисплею
static esp_err_t oled_cmd(uint8_t c)
{
    uint8_t buf[2] = {CTRL_CMD, c};
    // Передаємо 2 байти з таймаутом очікування 100 мс
    int64_t t0 = esp_timer_get_time();
    esp_err_t ret = i2c_master_transmit(oled, buf, 2, I2C_CMD_TIMEOUT_MS);
    int64_t dt = esp_timer_get_time() - t0;
    ESP_LOGI(TAG, "i2c CMD  0x%02X | %zu bytes | %lld us", c, (size_t)2, dt);
    return ret;
}

// Відправка команди разом з її аргументами (наприклад, команда + 1 або 2 байти параметрів)
static esp_err_t oled_cmds(const uint8_t *c, size_t n)
{
    uint8_t buf[OLED_CMD_BUF_SIZE];
    buf[0] = CTRL_CMD;          // Повідомляємо контролер дисплея, що далі йдуть команди
    memcpy(&buf[1], c, n);      // Копіюємо послідовність команд у буфер
    int64_t t0 = esp_timer_get_time();
    esp_err_t ret = i2c_master_transmit(oled, buf, n + 1, I2C_CMD_TIMEOUT_MS);
    int64_t dt = esp_timer_get_time() - t0;
    ESP_LOGI(TAG, "i2c CMDS 0x%02X+%zu | %zu bytes | %lld us", c[0], n - 1, n + 1, dt);
    return ret;
}

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

// Початкова ініціалізація контролера SSD1306
static void oled_init(void)
{
    static const uint8_t init_seq[] = {
        0xAE,             // Display OFF — вимкнути екран на час налаштування
        0xD5, 0x80,       // Set Display Clock Divide Ratio — стандартна частота тактування генератора
        0xA8, 0x3F,       // Set Multiplex Ratio — висота екрана (0x3F = 63 для 64 рядків, 0x1F для 32)
        0xD3, 0x00,       // Set Display Offset — вертикальний зсув зображення = 0
        0x40,             // Set Display Start Line — початковий рядок відображення = 0
        0x8D, 0x14,       // Charge Pump Setting — увімкнути внутрішній перетворювач напруги (обов'язково для живлення OLED)
        0x20, 0x00,       // Memory Addressing Mode — горизонтальний режим адресації (автоперехід по стовпцях і сторінках)
        0xA1,             // Segment Re-map — віддзеркалити по горизонталі (стовпець 127 стає SEG0)
        0xC8,             // COM Output Scan Direction — сканування зверху вниз (від COM63 до COM0)
        0xDA, 0x12,       // Set COM Pins Hardware Configuration — апаратна конфігурація виводів (0x12 для 128x64, 0x02 для 128x32)
        0x81, 0xCF,       // Set Contrast Control — яскравість/контрастність (0x00..0xFF)
        0xD9, 0xF1,       // Set Pre-charge Period — період попереднього заряду пікселів
        0xDB, 0x40,       // Set VCOMH Deselect Level — рівень напруги вимкнення пікселів
        0xA4,             // Entire Display ON — виводити зображення з відеопам'яті RAM (0xA5 засвітить усі пікселі)
        0xA6,             // Set Normal Display — звичайний режим (1 = піксель світиться, 0 = вимкнений)
        0xAF              // Display ON — увімкнути екран після завершення налаштувань
    };

    // Розбираємо масив і відправляємо кожну команду (1 байт або 2 байти з аргументом) окремою транзакцією
    size_t i = 0;
    while (i < sizeof(init_seq)) {
        uint8_t c = init_seq[i];
        size_t len = 1;
        // Перелік двобайтових команд, які потребують 1 байт параметра після коду команди
        if (c == 0xD5 || c == 0xA8 || c == 0xD3 || c == 0x8D || c == 0x20 ||
            c == 0xDA || c == 0x81 || c == 0xD9 || c == 0xDB) {
            len = 2;
        }
        ESP_ERROR_CHECK(oled_cmds(&init_seq[i], len));
        i += len;
    }
}

// Відправка всього кадрового буфера (fb) у відеопам'ять дисплея (GDDRAM)
static void oled_flush(void)
{
    const uint8_t col[] = {SSD1306_CMD_SET_COL_ADDR, 0, OLED_W - 1};  // Діапазон стовпців: 0..127
    const uint8_t page[] = {SSD1306_CMD_SET_PAGE_ADDR, 0, OLED_PAGES - 1}; // Діапазон сторінок: 0..7
    oled_cmds(col, sizeof(col));
    oled_cmds(page, sizeof(page));

    tx[0] = CTRL_DATA;                                // Вказуємо, що далі йдуть піксельні дані
    memcpy(&tx[1], fb, sizeof(fb));                   // Копіюємо 1024 байти кадру слідом за керуючим байтом
    // Передаємо весь кадр однією I2C-транзакцією (таймаут 1000 мс, оскільки пакет великий)
    int64_t t0 = esp_timer_get_time();
    ESP_ERROR_CHECK(i2c_master_transmit(oled, tx, sizeof(tx), I2C_FLUSH_TIMEOUT_MS));
    int64_t dt = esp_timer_get_time() - t0;
    ESP_LOGI(TAG, "i2c FLUSH full frame | %zu bytes | %lld us", sizeof(tx), dt);
}

// ---------- графіка (робота з локальним буфером fb) ----------

// Встановлення або очищення одного пікселя з координатами (x, y)
static void px(int x, int y, bool on)
{
    // Ігноруємо координати, що виходять за межі екрана
    if (x < 0 || x >= OLED_W || y < 0 || y >= OLED_H) return;
    // Кожен байт буфера відповідає за вертикальний стовпчик із 8 пікселів (1 сторінка)
    if (on) fb[x + (y / 8) * OLED_W] |=  (1 << (y % 8)); // Вмикаємо потрібний біт
    else    fb[x + (y / 8) * OLED_W] &= ~(1 << (y % 8)); // Вимикаємо потрібний біт
}

// Компактний растровий шрифт 5x7 пікселів (кожен байт — вертикальний стовпець, біт 0 — верхній піксель)
// Повний набір A-Z, 0-9 та спецсимволи для меню
typedef struct { char ch; uint8_t col[5]; } glyph_t;
static const glyph_t font[] = {
    {' ', {0x00,0x00,0x00,0x00,0x00}},
    {'!', {0x00,0x00,0x5F,0x00,0x00}},
    {'.', {0x00,0x60,0x60,0x00,0x00}},
    {':', {0x00,0x36,0x36,0x00,0x00}},
    {'-', {0x08,0x08,0x08,0x08,0x08}},
    {'>', {0x41,0x22,0x14,0x08,0x00}},  // стрілка-курсор для меню
    {'<', {0x0C,0x12,0x24,0x12,0x0C}},
    {'0', {0x3E,0x51,0x49,0x45,0x3E}},
    {'1', {0x00,0x42,0x7F,0x40,0x00}},
    {'2', {0x42,0x61,0x51,0x49,0x46}},
    {'3', {0x21,0x41,0x45,0x4B,0x31}},
    {'4', {0x18,0x14,0x12,0x7F,0x10}},
    {'5', {0x27,0x45,0x45,0x45,0x39}},
    {'6', {0x3C,0x4A,0x49,0x49,0x30}},
    {'7', {0x01,0x71,0x09,0x05,0x03}},
    {'8', {0x36,0x49,0x49,0x49,0x36}},
    {'9', {0x06,0x49,0x49,0x29,0x1E}},
    {'A', {0x7E,0x11,0x11,0x11,0x7E}},
    {'B', {0x7F,0x49,0x49,0x49,0x36}},
    {'C', {0x3E,0x41,0x41,0x41,0x22}},
    {'D', {0x7F,0x41,0x41,0x22,0x1C}},
    {'E', {0x7F,0x49,0x49,0x49,0x41}},
    {'F', {0x7F,0x09,0x09,0x09,0x01}},
    {'G', {0x3E,0x41,0x49,0x49,0x7A}},
    {'H', {0x7F,0x08,0x08,0x08,0x7F}},
    {'I', {0x00,0x41,0x7F,0x41,0x00}},
    {'J', {0x20,0x40,0x41,0x3F,0x01}},
    {'K', {0x7F,0x08,0x14,0x22,0x41}},
    {'L', {0x7F,0x40,0x40,0x40,0x40}},
    {'M', {0x7F,0x02,0x0C,0x02,0x7F}},
    {'N', {0x7F,0x04,0x08,0x10,0x7F}},
    {'O', {0x3E,0x41,0x41,0x41,0x3E}},
    {'P', {0x7F,0x09,0x09,0x09,0x06}},
    {'Q', {0x3E,0x41,0x51,0x21,0x5E}},
    {'R', {0x7F,0x09,0x19,0x29,0x46}},
    {'S', {0x46,0x49,0x49,0x49,0x31}},
    {'T', {0x01,0x01,0x7F,0x01,0x01}},
    {'U', {0x3F,0x40,0x40,0x40,0x3F}},
    {'V', {0x1F,0x20,0x40,0x20,0x1F}},
    {'W', {0x3F,0x40,0x38,0x40,0x3F}},
    {'X', {0x63,0x14,0x08,0x14,0x63}},
    {'Y', {0x07,0x08,0x70,0x08,0x07}},
    {'Z', {0x61,0x51,0x49,0x45,0x43}},
};


// ---------- елементи меню ----------
// Максимум видимих пунктів одночасно (64px / 8px = 8 рядків)
#define MENU_VISIBLE  OLED_PAGES

// Ідентифікатори пунктів меню — порядок визначає позицію на екрані
typedef enum {
    MENU_LIVE,          // Дані сенсора в реальному часі
    MENU_LOG,           // Перегляд журналу записів
    MENU_HEALTH,        // Стан системи (батарея, пам'ять, аптайм)
    MENU_CALIBRATE,     // Калібрування сенсора
    MENU_SETTINGS,      // Налаштування пристрою
    MENU_ABOUT,         // Інформація про прошивку
    MENU_UNASSIGNED0,         // Новий 
    MENU_UNASSIGNED1,         // Новий 
    MENU_UNASSIGNED2,
    MENU_UNASSIGNED3,
    MENU_COUNT,          // Завжди останній — загальна кількість пунктів

} menu_id_t;

// Текстові назви пунктів (відповідають menu_id_t за індексом)
static const char *menu_labels[MENU_COUNT] = {
    [MENU_LIVE]      = "LIVE VIEW",
    [MENU_LOG]       = "LOG BROWSER",
    [MENU_HEALTH]    = "HEALTH",
    [MENU_CALIBRATE] = "CALIBRATE",
    [MENU_SETTINGS]  = "SETTINGS",
    [MENU_ABOUT]     = "ABOUT",
    [MENU_UNASSIGNED0] = "TEST1",
    [MENU_UNASSIGNED1] = "TEST2",
    [MENU_UNASSIGNED2] = "TEST3",
    [MENU_UNASSIGNED3] = "TEST4"
};

// Однобуквені символи-іконки зліва від тексту (прості ASCII-маркери)
// Можна замінити на 8×8 бітмап-іконки, коли буде потрібно
static const char menu_icons[MENU_COUNT] = {
    [MENU_LIVE]      = '>',   // стрілка — живий потік
    [MENU_LOG]       = 'L',   // журнал
    [MENU_HEALTH]    = 'H',   // серце / стан
    [MENU_CALIBRATE] = 'C',   // калібрування
    [MENU_SETTINGS]  = 'S',   // шестерня
    [MENU_ABOUT]     = 'I',   // інфо
    [MENU_UNASSIGNED0] = '!',
    [MENU_UNASSIGNED1] = '<',
    [MENU_UNASSIGNED2] = ':',
    [MENU_UNASSIGNED3] = '.'
};


// Малювання одного символу за координатами верхнього лівого кута (x, y)
static void draw_char(int x, int y, char c)
{
    for (size_t g = 0; g < sizeof(font) / sizeof(font[0]); g++) {
        if (font[g].ch != c) continue;
        // Проходимо по 5 стовпцях і 7 рядках символу
        for (int cx = 0; cx < FONT_GLYPH_W; cx++)
            for (int cy = 0; cy < FONT_GLYPH_H; cy++)
                px(x + cx, y + cy, font[g].col[cx] & (1 << cy));
        return;
    }
}

// Виведення рядка тексту (крок 6 пікселів: 5 пікселів символ + 1 піксель пробіл між літерами)
static void draw_text(int x, int y, const char *s)
{
    for (; *s; s++, x += FONT_CHAR_ADVANCE) draw_char(x, y, *s);
}

// Малювання контуру прямокутника за координатами протилежних кутів
static void draw_rect(int x0, int y0, int x1, int y1)
{
    for (int x = x0; x <= x1; x++) { px(x, y0, 1); px(x, y1, 1); } // Верхня та нижня грані
    for (int y = y0; y <= y1; y++) { px(x0, y, 1); px(x1, y, 1); } // Ліва та права грані
}

// Fast XOR-invert: operates on whole bytes where possible
static void fb_invert_rect_fast(int x0, int y0, int x1, int y1)
{
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= OLED_W) x1 = OLED_W - 1;
    if (y1 >= OLED_H) y1 = OLED_H - 1;

    int page_start = y0 / 8;
    int page_end   = y1 / 8;

    for (int page = page_start; page <= page_end; page++) {
        // Build a bitmask for which bits (rows) in this page to flip
        uint8_t mask = 0xFF;

        if (page == page_start) {
            // Top edge: only flip bits from y0%8 upward
            mask &= (0xFF << (y0 % 8));
        }
        if (page == page_end) {
            // Bottom edge: only flip bits up to y1%8
            mask &= (0xFF >> (7 - (y1 % 8)));
        }

        // XOR across all columns in one pass
        uint8_t *row = &fb[page * OLED_W];
        for (int x = x0; x <= x1; x++) {
            row[x] ^= mask;
        }
    }
}

// ---------- апаратне керування відображенням ----------

// Поточний зсув Display Start Line — який рядок GDDRAM зверху екрана
static uint8_t current_start_line = 0;

// Позиція курсора меню та кількість видимих рядків
static int menu_sel  = 0;    // Індекс обраного пункту
static int menu_top  = 0;    // Індекс першого видимого пункту (для прокрутки)

// Запис одної сторінки (128 байт) у GDDRAM замість повного oled_flush (1024 байти)
// Економить ~7/8 трафіку I2C при оновленні одного рядка меню
static void oled_write_page(uint8_t page)
{
    const uint8_t col_cmd[]  = {SSD1306_CMD_SET_COL_ADDR, 0x00, OLED_W - 1}; // Стовпці 0..127
    const uint8_t page_cmd[] = {SSD1306_CMD_SET_PAGE_ADDR, page, page};       // Лише одна сторінка
    oled_cmds(col_cmd, sizeof(col_cmd));
    oled_cmds(page_cmd, sizeof(page_cmd));

    // Формуємо міні-пакет: 1 керуючий байт + 128 байт даних
    uint8_t buf[1 + OLED_W];
    buf[0] = CTRL_DATA;
    memcpy(&buf[1], &fb[page * OLED_W], OLED_W);
    int64_t t0 = esp_timer_get_time();
    ESP_ERROR_CHECK(i2c_master_transmit(oled, buf, sizeof(buf), I2C_PAGE_TIMEOUT_MS));
    int64_t dt = esp_timer_get_time() - t0;
    ESP_LOGI(TAG, "i2c PAGE %u | %zu bytes | %lld us", page, sizeof(buf), dt);
}

// Зміна Display Start Line — який рядок GDDRAM відображається зверху екрана
// Однобайтова команда: 0x40 | (line & 0x3F). GDDRAM не змінюється — зсувається лише
// апаратний покажчик читання. Обхід циклічний: після рядка 63 йде рядок 0.
static void oled_set_start_line(uint8_t line)
{
    line &= SSD1306_START_LINE_MASK;     // Обмежуємо діапазон 0..63
    oled_cmd(SSD1306_CMD_START_LINE_BASE | line); // Команда Set Display Start Line
    current_start_line = line;           // Зберігаємо для розрахунків позиції
}


// Малювання одного пункту меню в задану сторінку кадрового буфера
// page     — номер сторінки (0..7) у fb, куди малюємо
// idx      — індекс пункту з menu_labels[]
// selected — чи цей пункт обрано (XOR-інверсія)
static void draw_menu_item(uint8_t page, int idx, bool selected)
{
    // 1. Очищаємо рядок
    memset(&fb[page * OLED_W], 0x00, OLED_W);

    // 2. Малюємо іконку-символ ліворуч (x=2) та назву (x=12, після іконки + пробіл)
    int y = page * 8;
    draw_char(MENU_ICON_X, y, menu_icons[idx]);
    draw_text(MENU_TEXT_X, y, menu_labels[idx]);

    // 3. Якщо пункт обрано — інвертуємо весь рядок (білий текст на чорному тлі)
    if (selected) {
        fb_invert_rect_fast(0, y, OLED_W - 1, y + 7);
    }
    // Не робимо flush тут — викликач вирішує коли і що оновити
}

// Перемальовує все видиме вікно меню (до 8 пунктів) і оновлює екран
static void menu_draw_full(void)
{
    for (int i = 0; i < MENU_VISIBLE && (menu_top + i) < MENU_COUNT; i++) {
        int idx = menu_top + i;
        draw_menu_item(i, idx, idx == menu_sel);
    }
    // Якщо пунктів менше ніж рядків — очистити залишок
    // for (int i = MENU_COUNT - menu_top; i < MENU_VISIBLE; i++) {
    //     memset(&fb[i * OLED_W], 0x00, OLED_W); // 
    // }
    oled_flush();
}

// Переміщення виділення на +1 або -1 з мінімальним оновленням
// Повертає true якщо позиція змінилась
static bool menu_move(int dir)
{
    int prev = menu_sel;
    menu_sel += dir;

    // Обмежуємо діапазон
    if (menu_sel < 0) menu_sel = 0;
    if (menu_sel >= MENU_COUNT) menu_sel = MENU_COUNT - 1;
    if (menu_sel == prev) return false;

    // Перевірка: чи потрібно прокрутити вікно
    bool scrolled = false;
    if (menu_sel < menu_top) {
        menu_top = menu_sel;
        scrolled = true;
    }
    if (menu_sel >= menu_top + MENU_VISIBLE) {
        menu_top = menu_sel - MENU_VISIBLE + 1;
        scrolled = true;
    }

    if (scrolled) {
        // Вікно зсунулось — перемалювати все
        menu_draw_full();
    } else {
        // Оновити тільки два рядки: старий і новий виділений
        int prev_page = prev - menu_top;
        int new_page  = menu_sel - menu_top;
        draw_menu_item(prev_page, prev, false);     // Зняти виділення
        draw_menu_item(new_page, menu_sel, true);    // Нове виділення
        oled_write_page(prev_page);                  // Оновити тільки ці 2 сторінки
        oled_write_page(new_page);
    }
    return true;
}

// ---------- e-ink стиль перехід між екранами ----------

// Апаратна інверсія-спалах: весь екран миготить негативом і потім показує новий зміст
// target_fb — вказівник на 1024-байтний буфер з новим кадром (або NULL щоб flush поточний fb)
static void eink_transition(const uint8_t *target_fb)
{
    // Крок 1: Спалах — апаратна інверсія (жодної зміни GDDRAM)
    oled_cmd(SSD1306_CMD_INVERSE_DISPLAY);           // Inverse Display ON
    vTaskDelay(pdMS_TO_TICKS(EINK_FLASH_DURATION_MS)); // Утримуємо негатив

    // Крок 2: Повернення до нормального режиму — показує старий кадр на мить
    oled_cmd(SSD1306_CMD_NORMAL_DISPLAY);            // Normal Display
    vTaskDelay(pdMS_TO_TICKS(EINK_PAUSE_DURATION_MS)); // Пауза перед новим кадром

    // Крок 3: Завантажити новий кадр
    if (target_fb) {
        memcpy(fb, target_fb, sizeof(fb));
    }

    // Крок 4: Відправити на дисплей
    oled_flush();
}

// ---------- головна функція програми ----------
void app_main(void)
{
    // 1. Конфігурація та створення шини I2C Master
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    // 1. Додати девайс  

    ESP_ERROR_CHECK(i2c_master_probe(bus, DS1307_ADDR, I2C_CMD_TIMEOUT_MS));
    ESP_LOGI(TAG, "RTC DS1307_ADDR знайдено за адресою 0x%02X", DS1307_ADDR);
 // 3. Реєстрація  як пристрою на шині
    i2c_device_config_t rtc_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_ADDR,
        .scl_speed_hz = I2C_OLED_FREQ_HZ,
    };
    i2c_master_bus_add_device(bus, &rtc_cfg, &rtc);


    // 2. Перевірка наявності дисплея на шині
    ESP_ERROR_CHECK(i2c_master_probe(bus, SSD1306_ADDR, I2C_CMD_TIMEOUT_MS));
    ESP_LOGI(TAG, "SSD1306 знайдено за адресою 0x%02X", DS1307_ADDR);

    // 3. Реєстрація дисплея як пристрою на шині
    i2c_device_config_t oled_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_ADDR,
        .scl_speed_hz = I2C_OLED_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &oled_cfg, &oled));

    // 4. Ініціалізація дисплея
    oled_init();

    // ====== BOOT SCREEN ======
    // Простий стартовий екран з назвою та рамкою
    memset(fb, 0, sizeof(fb));
    draw_rect(0, 0, OLED_W - 1, OLED_H - 1);
    draw_text(22, 12, NAME);
    draw_text(40, 28, VERSION);
    draw_text(16, 44, AUTHOR);
    oled_flush();

    // Показуємо бут-екран 2 секунди
    vTaskDelay(pdMS_TO_TICKS(BOOT_SCREEN_HOLD_MS));

    // ====== E-INK TRANSITION: boot → menu ======
    // Готуємо кадр меню в тимчасовому буфері, потім переходимо зі спалахом
    uint8_t menu_fb[OLED_W * OLED_PAGES];
    memset(menu_fb, 0, sizeof(menu_fb));

    // Тимчасово підмінюємо fb для draw_menu_item (вони пишуть у глобальний fb)
    // Збережемо бут-кадр, намалюємо меню, потім відновимо для eink_transition
    uint8_t boot_fb[OLED_W * OLED_PAGES];
    memcpy(boot_fb, fb, sizeof(fb));

    // Малюємо початковий стан меню
    menu_sel = 0;
    menu_top = 0;
    for (int i = 0; i < MENU_VISIBLE && i < MENU_COUNT; i++) {
        draw_menu_item(i, i, i == menu_sel);
    }
    // fb тепер містить меню — зберегти
    memcpy(menu_fb, fb, sizeof(fb));

    // Повернути бут-кадр у fb (eink_transition покаже його перед переходом)
    memcpy(fb, boot_fb, sizeof(fb));
    oled_flush();

    // Виконуємо перехід: спалах інверсією → новий кадр меню
    eink_transition(menu_fb);

    ESP_LOGI(TAG, "Меню відображено. Пунктів: %d", MENU_COUNT);

    // ====== ENCODER INIT ======
    static const encoder_config_t enc_cfg = {
        .gpio_a            = ENC_A_GPIO,
        .gpio_b            = ENC_B_GPIO,
        .debug_gpio        = GPIO_NUM_NC,
        .quadrature_x4     = ENCODER_QUADRATURE_X4,
        .counts_per_detent = ENCODER_COUNTS_PER_DETENT,
        .glitch_filter_ns  = GLITCH_FILTER_DELAY_NS,
        .high_limit        = ENCODER_PCNT_HIGH_LIMIT,
        .low_limit         = ENCODER_PCNT_LOW_LIMIT,
    };
    
    encoder_init(&enc, &enc_cfg);
    ESP_LOGI(TAG, "Encoder initialized on GPIO %d/%d", ENC_A_GPIO, ENC_B_GPIO);

    // ====== MAIN LOOP: encoder-driven menu navigation ======
    while (1) {
        int32_t clicks = encoder_update_ticks(&enc);

        if (clicks > 0) {
            for (int32_t i = 0; i < clicks; i++) {
                menu_move(+1);
            }
            ESP_LOGI(TAG, "Обрано: [%d] %s", menu_sel, menu_labels[menu_sel]);
        } else if (clicks < 0) {
            for (int32_t i = 0; i > clicks; i--) {
                menu_move(-1);
            }
            ESP_LOGI(TAG, "Обрано: [%d] %s", menu_sel, menu_labels[menu_sel]);
        }

        // RTC clock byte 
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

        vTaskDelay(pdMS_TO_TICKS(ENCODER_POLL_MS));  // ~100 Hz polling
    }
}