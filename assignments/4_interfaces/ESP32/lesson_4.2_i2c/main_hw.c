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

static const char *TAG = "SSD1306";

// Налаштування апаратної шини I2C
#define I2C_SDA_GPIO   6      // Пін даних SDA (змініть під свою плату)
#define I2C_SCL_GPIO   7      // Пін тактування SCL
#define I2C_FREQ_HZ    400000 // Частота шини: 400 кГц (Fast Mode)
#define SSD1306_ADDR   0x3C   // 7-бітна I2C-адреса дисплея (драйвер сам додасть біт R/W)

// Роздільна здатність екрана та організація пам'яті
#define OLED_W 128            // Ширина дисплея в пікселях
#define OLED_H 64             // Висота дисплея в пікселях
#define OLED_PAGES (OLED_H / 8) // Кількість сторінок пам'яті (64 / 8 = 8 сторінок по 8 рядків пікселів)

// Керуючі байти протоколу SSD1306 (перший байт після адреси пристрою)
#define CTRL_CMD  0x00   // Co=0, D/C#=0 -> наступні байти є командами налаштування
#define CTRL_DATA 0x40   // Co=0, D/C#=1 -> наступні байти є графічними даними для відеопам'яті (GDDRAM)

static i2c_master_dev_handle_t oled;             // Дескриптор (handle) зареєстрованого пристрою на шині
static uint8_t fb[OLED_W * OLED_PAGES];          // Локальний кадровий буфер у RAM мікроконтролера (1024 байти)
static uint8_t tx[1 + OLED_W * OLED_PAGES];      // Буфер відправки по I2C: 1 керуючий байт (0x40) + 1024 байти кадру

// ---------- низький рівень (обмін по I2C) ----------

// Відправка однієї однобайтової команди дисплею
static esp_err_t oled_cmd(uint8_t c)
{
    uint8_t buf[2] = {CTRL_CMD, c};
    // Передаємо 2 байти з таймаутом очікування 100 мс
    return i2c_master_transmit(oled, buf, 2, 100);
}

// Відправка команди разом з її аргументами (наприклад, команда + 1 або 2 байти параметрів)
static esp_err_t oled_cmds(const uint8_t *c, size_t n)
{
    uint8_t buf[16];
    buf[0] = CTRL_CMD;          // Повідомляємо контролер дисплея, що далі йдуть команди
    memcpy(&buf[1], c, n);      // Копіюємо послідовність команд у буфер
    return i2c_master_transmit(oled, buf, n + 1, 100);
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
    const uint8_t col[] = {0x21, 0, OLED_W - 1};      // Задаємо діапазон стовпців: від 0 до 127
    const uint8_t page[] = {0x22, 0, OLED_PAGES - 1}; // Задаємо діапазон сторінок: від 0 до 7
    oled_cmds(col, sizeof(col));
    oled_cmds(page, sizeof(page));

    tx[0] = CTRL_DATA;                                // Вказуємо, що далі йдуть піксельні дані
    memcpy(&tx[1], fb, sizeof(fb));                   // Копіюємо 1024 байти кадру слідом за керуючим байтом
    // Передаємо весь кадр однією I2C-транзакцією (таймаут 1000 мс, оскільки пакет великий)
    ESP_ERROR_CHECK(i2c_master_transmit(oled, tx, sizeof(tx), 1000));
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

// Малювання одного символу за координатами верхнього лівого кута (x, y)
static void draw_char(int x, int y, char c)
{
    for (size_t g = 0; g < sizeof(font) / sizeof(font[0]); g++) {
        if (font[g].ch != c) continue;
        // Проходимо по 5 стовпцях і 7 рядках символу
        for (int cx = 0; cx < 5; cx++)
            for (int cy = 0; cy < 7; cy++)
                px(x + cx, y + cy, font[g].col[cx] & (1 << cy));
        return;
    }
}

// Виведення рядка тексту (крок 6 пікселів: 5 пікселів символ + 1 піксель пробіл між літерами)
static void draw_text(int x, int y, const char *s)
{
    for (; *s; s++, x += 6) draw_char(x, y, *s);
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
    const uint8_t col_cmd[]  = {0x21, 0x00, 0x7F};        // Стовпці 0..127
    const uint8_t page_cmd[] = {0x22, page, page};         // Лише одна сторінка
    oled_cmds(col_cmd, sizeof(col_cmd));
    oled_cmds(page_cmd, sizeof(page_cmd));

    // Формуємо міні-пакет: 1 керуючий байт + 128 байт даних
    uint8_t buf[1 + OLED_W];
    buf[0] = CTRL_DATA;
    memcpy(&buf[1], &fb[page * OLED_W], OLED_W);
    ESP_ERROR_CHECK(i2c_master_transmit(oled, buf, sizeof(buf), 200));
}

// Зміна Display Start Line — який рядок GDDRAM відображається зверху екрана
// Однобайтова команда: 0x40 | (line & 0x3F). GDDRAM не змінюється — зсувається лише
// апаратний покажчик читання. Обхід циклічний: після рядка 63 йде рядок 0.
static void oled_set_start_line(uint8_t line)
{
    line &= 0x3F;                        // Обмежуємо діапазон 0..63
    oled_cmd(0x40 | line);               // Команда Set Display Start Line
    current_start_line = line;           // Зберігаємо для розрахунків позиції
}

// ---------- елементи меню ----------

// Ідентифікатори пунктів меню — порядок визначає позицію на екрані
typedef enum {
    MENU_LIVE,          // Дані сенсора в реальному часі
    MENU_LOG,           // Перегляд журналу записів
    MENU_HEALTH,        // Стан системи (батарея, пам'ять, аптайм)
    MENU_CALIBRATE,     // Калібрування сенсора
    MENU_SETTINGS,      // Налаштування пристрою
    MENU_ABOUT,         // Інформація про прошивку
    MENU_COUNT          // Завжди останній — загальна кількість пунктів
} menu_id_t;

// Текстові назви пунктів (відповідають menu_id_t за індексом)
static const char *menu_labels[MENU_COUNT] = {
    [MENU_LIVE]      = "LIVE VIEW",
    [MENU_LOG]       = "LOG BROWSER",
    [MENU_HEALTH]    = "HEALTH",
    [MENU_CALIBRATE] = "CALIBRATE",
    [MENU_SETTINGS]  = "SETTINGS",
    [MENU_ABOUT]     = "ABOUT",
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
};

// Максимум видимих пунктів одночасно (64px / 8px = 8 рядків)
#define MENU_VISIBLE  OLED_PAGES

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
    draw_char(2, y, menu_icons[idx]);
    draw_text(12, y, menu_labels[idx]);

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
    for (int i = MENU_COUNT - menu_top; i < MENU_VISIBLE; i++) {
        memset(&fb[i * OLED_W], 0x00, OLED_W);
    }
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
    oled_cmd(0xA7);                                  // Inverse Display ON
    vTaskDelay(pdMS_TO_TICKS(80));                   // Утримуємо негатив ~80 мс

    // Крок 2: Повернення до нормального режиму — показує старий кадр на мить
    oled_cmd(0xA6);                                  // Normal Display
    vTaskDelay(pdMS_TO_TICKS(50));                   // Пауза ~50 мс

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

    // 2. Перевірка наявності дисплея на шині
    ESP_ERROR_CHECK(i2c_master_probe(bus, SSD1306_ADDR, 100));
    ESP_LOGI(TAG, "SSD1306 знайдено за адресою 0x%02X", SSD1306_ADDR);

    // 3. Реєстрація дисплея як пристрою на шині
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &oled));

    // 4. Ініціалізація дисплея
    oled_init();

    // ====== BOOT SCREEN ======
    // Простий стартовий екран з назвою та рамкою
    memset(fb, 0, sizeof(fb));
    draw_rect(0, 0, OLED_W - 1, OLED_H - 1);
    draw_text(22, 12, "SENSOR LOGGER");
    draw_text(40, 28, "V1.0");
    draw_text(16, 44, "BEETROOT ACADEMY");
    oled_flush();

    // Показуємо бут-екран 2 секунди
    vTaskDelay(pdMS_TO_TICKS(2000));

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

    // ====== ДЕМО-ЦИКЛ: Автоматична навігація по меню ======
    // (Замінити на реальний ввід від rotary encoder у фінальному проєкті)
    while (1) {
        // Рухаємось вниз до кінця
        for (int i = 0; i < MENU_COUNT - 1; i++) {
            vTaskDelay(pdMS_TO_TICKS(600));
            menu_move(+1);
            ESP_LOGI(TAG, "Обрано: [%d] %s", menu_sel, menu_labels[menu_sel]);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Рухаємось вгору до початку
        for (int i = 0; i < MENU_COUNT - 1; i++) {
            vTaskDelay(pdMS_TO_TICKS(600));
            menu_move(-1);
            ESP_LOGI(TAG, "Обрано: [%d] %s", menu_sel, menu_labels[menu_sel]);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Демо e-ink переходу: меню → бут → меню
        eink_transition(boot_fb);
        vTaskDelay(pdMS_TO_TICKS(1500));
        eink_transition(menu_fb);
    }
}