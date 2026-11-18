# Final project — Sensor/device logger bridge: research & design notes

_Last updated: 2026-11-15_

## 0. Scope and requirements

Firmware on a microcontroller (ESP32-S3 **or** STM32F401CEU6) that acts as a logger/bridge for one sensor or storage/audio device. Preferred buses to learn: **I2C and SPI** (avoid UART for the device link).

| ID | Requirement |
|---|---|
| FR-COMM | Check that communication works: device is responsive to commands, data is received successfully (chip ID, status polling, config readback, error counters). |
| FR-BOUNDS (optional) | Sanity check: readings within datasheet min/max bounds; out-of-bounds flagged in the record. |
| FR-CAL (optional) | Calibration test (self-test where the chip has one, otherwise reference comparison / rotation calibration). |
| FR-UI | Rotary encoder + LCD/OLED: browse stored logs without a PC, show live readings, health and calibration screens. |
| FR-STORE | Log to on-board flash (internal partition or SPI NOR). External SD/USB storage optional. |
| FR-ID | Recognise multiple variants of a sensor with different IDs (see §2). |

---

## 1. Candidate devices

### 1.1 BME280 — temperature / humidity / pressure (I2C or SPI)

- Bus: I2C (≤3.4 MHz, address 0x76/0x77 via SDO) or SPI (≤10 MHz, 3- or 4-wire). CSB low at power-up selects SPI. **4-pin modules are I2C-only; get the 6-pin module for SPI.**
- Health checks:
  - `0xD0` chip ID = **0x60** (0x58 = BMP280, a common "fake BME280"; 0xFF = bad address/dead bus).
  - Soft reset: write 0xB6 to `0xE0`, poll `0xF3` status (`im_update` clears, `measuring` toggles).
  - Write `ctrl_hum 0xF2`, `ctrl_meas 0xF4`, `config 0xF5` and read back (ctrl_hum only takes effect after a ctrl_meas write).
  - Trim data `0x88–0xA1`, `0xE1–0xE7` must not be all 0x00/0xFF.
- Control: oversampling per channel (skip, ×1…×16), sleep/forced/normal mode, IIR filter, standby time.
- Log: raw ADC values (`0xF7–0xFE`) and compensated T/H/P.
- Bounds: −40…+85 °C, 0…100 %RH, 300…1100 hPa. Accuracy ±1 °C (0–65 °C), ±3 %RH, ±1 hPa.
- Calibration: no built-in self-test → compare with a reference (thermometer, local weather-station pressure, saturated-salt humidity test).

### 1.2 GY-273 magnetometer (I2C only)

Identify the chip with an I2C scan (0x1E HMC5883L, 0x0D QMC5883L, 0x2C QMC5883P — see §2).

| | HMC5883L | QMC5883L |
|---|---|---|
| ID | `0x0A–0x0C` = 0x48 0x34 0x33 ("H43") | `0x0D` = 0xFF |
| Status | RDY, LOCK (`0x09`) | DRDY, OVL, DOR (`0x06`) |
| Control | gain, data rate, averaging, single/continuous | OSR 64–512, ±2/±8 G, ODR, soft reset, DRDY interrupt |
| Extra | — | relative temperature `0x07/0x08` (offset not compensated) |
| Self-test | **Yes**: CRA = 0x11 (+bias) / 0x12 (−bias); GN=5 expects 243…575 LSB | none documented |

- Bounds: Earth field ≈ 25–65 µT total magnitude; OVL set or far-off magnitude = fault or magnet nearby.
- Calibration: rotate through all orientations for the hard-iron offset (min/max per axis), optionally soft-iron. On HMC, use the self-test result to correct temperature scale drift.
- Driver note: DRDY/OVL are usually cleared only by reading the data registers — always read the data even on overflow.

### 1.3 Audio / storage device

**GPD2846A — rejected.** Button-only (key pads P1–P6), no serial interface; play/pause share one pin, so playback state cannot be read. The linked arduino.ua board doesn't name its chip and offers UART with an undocumented protocol and no datasheet.

| Alternative | Bus | Log / control |
|---|---|---|
| **VS1053 module with SD slot** (recommended) | SPI (XCS + XDCS + DREQ, SD on the same bus) | SCI registers: MODE, STATUS (version), DECODE_TIME (s), HDAT0/HDAT1 (bitrate, sample rate, format), VOL. Built-in SCI/memory/sine tests. SD: init, size, read errors. |
| MCU + SD (SPI) + I2S DAC/amp (MAX98357A, PCM5102) | SPI + I2S | Software MP3 decoding; log SD init/CSD/CID/CRC errors, throughput, audio buffer underruns. Best on ESP32-S3. |
| DFPlayer Mini | UART | Documented: status 0x42, volume 0x43, EQ 0x44, file counts, error codes, BUSY pin. Easy but UART-only. |

### 1.4 Recommendation

- Easiest SPI project: **BME280 (6-pin) over SPI**.
- Best calibration story on I2C: **GY-273**, if it turns out to be an HMC5883L.
- Most ambitious: **VS1053 + SD on ESP32-S3** (device + storage on one SPI bus).
- Combined option: sensor on SPI + magnetometer/OLED on I2C → the logger exercises both buses.

---

## 2. Sensor variants and identification

### 2.1 Bosch barometric family — the ID register moves between generations, and values collide

| Chip | ID reg → value | Calibration / compensation differences |
|---|---|---|
| BMP180 | 0xD0 → 0x55 | Older register map, different integer compensation. Separate driver. |
| BMP280 | 0xD0 → 0x58 (samples 0x56/0x57) | Same T/P trim as BME280, **no humidity**. |
| **BME280** | 0xD0 → **0x60** | + 6 humidity trim values (H4/H5 packed 12-bit across nibbles at 0xE4–0xE6 — classic parsing bug). |
| BME680 / BME688 | 0xD0 → 0x61; variant `0xF0`: 0x00 = 680, 0x01 = 688 | Gas heater; different gas-resistance formula per variant. |
| BMP388 | 0x00 → 0x50 | New map; NVM trim from 0x31 converted to float coefficients. |
| **BMP390** | **0x00 → 0x60** | Same driver as BMP388; **same ID value as BME280, different register**. |
| BMP580/581 | 0x01 → 0x50 (Bosch API also accepts 0x51) | New map again. |

None of these has a user-usable built-in self-test (only the BME68x gas-heater self-test) → calibration is always against a reference; only the trim layout and compensation maths differ per chip.

### 2.2 Magnetometers on GY-271/GY-273 boards — at least five chips, addresses and IDs collide

| Chip | Addr | ID check | Notes |
|---|---|---|---|
| HMC5883L (Honeywell, discontinued) | 0x1E | `0x0A–0x0C` = "H43" | Self-test; big-endian X, Z, Y data order. |
| **HMC5983** | 0x1E | **same "H43"** | + temperature compensation, 220 Hz, SPI. Distinguish by temperature-sensor mode. |
| QMC5883L "DB" | 0x0D | `0x0D` = 0xFF | Little-endian X, Y, Z; relative temperature only. |
| QMC5883L "DA" | **0x1E** | undocumented | Collides with HMC address → address alone is not proof. |
| **QMC5883P** (now common on new boards) | 0x2C | `0x00` = 0x80 | Chip ID first, data from 0x01; different register map again. |
| VCM5883L | 0x0C | poorly documented | Clone; supported by DFRobot's library. |

Calibration differences:
- **Axis orientation**: QMC north along +X, HMC along −Y → heading maths and hard/soft-iron data stored per variant.
- **Scale**: HMC5883L 1090 LSB/G at default gain; QMC5883L 12000 LSB/G at ±2 G, 3000 LSB/G at ±8 G.
- **Self-test**: only HMC5883L/HMC5983. On QMC chips, fall back to rotation calibration + Earth-field magnitude check.
- **Temperature drift**: HMC5983 compensates in hardware; HMC5883L via self-test scale factor; QMC temperature only useful as a relative value.

### 2.3 Requirement FR-ID — device identification and variant support

1. At boot, and after a communication error persists after retries, scan a **probe table** of `{bus, address, id_reg, expected, mask, driver}`. Identify by **register and value together**, never by address alone.
2. Resolve collisions with a **secondary check**: BME68x variant register `0xF0`, HMC5983 temperature-mode test, factory trim data not all 0x00/0xFF.
3. Minimum supported variants: BMP280 + BME280; HMC5883L + QMC5883L + QMC5883P. Each has its own driver with a common interface (`probe / init / read / self_test / calibrate`) and declares capabilities (`has_humidity`, `has_self_test`, `has_temp`).
4. A device that ACKs but isn't recognised is reported, not driven blindly: Health screen + log show `unknown @0xNN id=0xNN`.
5. Each log session header records the detected variant. Calibration data (mag offsets, axis orientation, bounds) is stored **per variant** and isn't applied if the variant changes.
6. Acceptance: swap at least two variants on the same board (BMP280 ↔ BME280, or QMC5883L ↔ QMC5883P); firmware identifies each, hides unsupported features, passes health checks, no rebuild.

---

## 3. User interface and storage

### 3.1 Encoder
- KY-040 module or bare EC11 with push switch (rotate / click / long-press).
- Decode in hardware, not in a GPIO ISR:
  - ESP32-S3: **PCNT** in quadrature mode with glitch filter (ESP-IDF rotary-encoder example).
  - STM32F401: **timer encoder mode** (TIM2–TIM5), input filter for debounce; read CNT.
- Button: GPIO interrupt + 20–50 ms debounce timer; short vs long press.

### 3.2 Display
| Display | Bus | Notes |
|---|---|---|
| **SSD1306/SH1106 OLED 128×64** (recommended) | I2C (0x3C) | 1 KB frame ≈ 25 ms at 400 kHz; shares I2C with the sensor → bus mutex. |
| ST7789/ST7735 TFT | SPI | Colour graphs; 240×240 RGB565 = 115 KB/frame → DMA + partial updates. |
| 1602/2004 LCD + PCF8574 | I2C | Text only. |

ESP-IDF: `esp_lcd` (SSD1306 driver, ST7789), optional LVGL. STM32: own framebuffer + font/line routines.

### 3.3 Storage
- Fixed-size records (timestamp, raw values, status flags, error code) in a **ring buffer**.
- ESP32-S3 N16R8: custom data partition, raw `esp_partition_*` writes, 4 KB erase sectors (or LittleFS).
- STM32F401CEU6 (512 KB flash): reserve the last 128 KB sector; buffer in RAM, erase rarely.
- **W25Qxx SPI NOR** (optional): JEDEC ID check (0x9F), status register, 4 KB sectors, read-back verify — fits FR-COMM well.
- SD card: optional export path.
- Timestamps: STM32 RTC + LSE + coin cell on VBAT; ESP32-S3 uptime + boot counter or SNTP.

### 3.4 Screens
1. **Live** — values + sparkline; rotate to switch channel.
2. **Log browser** — rotate = scroll, click = details/zoom, long-press = back; out-of-bounds records highlighted, jump-to-next-fault.
3. **Health** — detected chip/variant, chip ID OK, last good read, bus errors (NACK/timeout), records stored, storage status.
4. **Calibration** — self-test (only if the driver reports `has_self_test`), hard-iron routine, set bounds, pass/fail.

Firmware structure (FreeRTOS on either MCU): sensor task, logger task, UI task; mutex on the shared I2C bus.

---

## 4. Parts list

| # | Part | Qty | Notes |
|---|---|---|---|
| 1 | ESP32-S3-DevKitC-1 (N16R8) **or** WeAct STM32F401CEU6 Black Pill | 1 | Pin plans for both in §5. |
| 2 | BME280 module, **6-pin** (I2C+SPI) | 1–2 | Verify chip ID 0x60 on arrival; a BMP280 (0x58) is useful for the FR-ID swap test. |
| 3 | GY-273 / GY-271 magnetometer | 1–2 | Expect QMC5883L or QMC5883P; two different variants cover the FR-ID acceptance test. |
| 4 | OLED 128×64 I2C (SSD1306 0.96″ or SH1106 1.3″) | 1 | 3.3 V, address 0x3C. |
| 5 | (alt.) ST7789 1.54″ 240×240 SPI TFT | 0–1 | If you want colour graphs. |
| 6 | Rotary encoder EC11 / KY-040 with push switch | 1 | 10 k pull-ups (on KY-040) + optional 10–100 nF RC filter. |
| 7 | W25Q64/W25Q128 SPI NOR (SOIC-8 breakout or module) | 0–1 | Optional; WeAct Black Pill has a footprint for one (check your board revision). |
| 8 | microSD SPI module, 3.3 V-native | 0–1 | Optional export. Avoid 5 V modules with level-shifter buffers that don't release MISO on a shared bus. |
| 9 | VS1053 module with SD slot | 0–1 | Audio option (SPI). |
| 10 | (alt.) MAX98357A I2S amp + 4–8 Ω speaker | 0–1 | Audio option with software decoding (ESP32-S3). |
| 11 | CR2032 + holder | 0–1 | STM32 VBAT for RTC timestamps. |
| 12 | 4.7 kΩ resistors, 100 nF caps, breadboard, Dupont wires | — | Extra I2C pull-ups if the bus is long or a module lacks them. |
| 13 | USB logic analyzer (8 ch, 24 MHz, sigrok/PulseView) | 1 | Strongly recommended for I2C/SPI debugging and for showing protocol traces in the defence. |

---

## 5. Pin / bus allocation

### 5.1 ESP32-S3-DevKitC-1 (N16R8)

**Do not use:** GPIO 26–32 (flash), 33–37 (octal PSRAM on R8), 19/20 (native USB — used for USB-Serial-JTAG), 43/44 (UART0 console), strapping pins 0, 3, 45, 46. RGB LED is on GPIO48 (v1.0) or GPIO38 (v1.1).

| Function | Signal | GPIO | Peripheral |
|---|---|---|---|
| I2C bus | SDA / SCL | 8 / 9 | I2C0, 400 kHz — OLED 0x3C, magnetometer, BME280 (I2C mode) |
| SPI bus | SCLK / MOSI / MISO | 12 / 11 / 13 | SPI2 (FSPI) |
| | CS BME280 (SPI mode) | 10 | |
| | CS W25Q flash | 14 | |
| | CS microSD (optional) | 21 | |
| Encoder | A / B | 4 / 5 | PCNT unit 0 (quadrature, glitch filter) |
| | SW | 6 | GPIO interrupt |
| Sensor interrupt | DRDY (QMC/HMC) | 7 | GPIO interrupt |
| Audio — VS1053 option | XCS / XDCS / DREQ / XRST | 15 / 16 / 17 / 18 | on SPI2 |
| Audio — I2S option (instead of VS1053) | BCLK / LRCK / DOUT | 15 / 16 / 17 | I2S0 |
| TFT option (instead of OLED) | CS / DC / RST / BL | 39 / 40 / 41 / 42 | on SPI2. 39–42 are the external-JTAG pads — free while debugging over built-in USB-JTAG. |

All peripherals route through the GPIO matrix, so these can be moved; keep high-speed SPI on the IO_MUX-capable defaults if you push the clock above ~40 MHz.

### 5.2 STM32F401CEU6 (WeAct Black Pill)

**Reserved:** PA11/PA12 (USB), PA13/PA14 (SWD), PC13 (LED), PA0 (KEY button), PC14/PC15 (LSE), PH0/PH1 (HSE), PB2 (BOOT1), PA9/PA10 (USART1 debug console). **PB11 doesn't exist** on this package.

| Function | Signal | Pin | Peripheral / AF |
|---|---|---|---|
| I2C bus | SCL / SDA | PB8 / PB9 | I2C1, AF4 — OLED, magnetometer, BME280 (I2C mode) |
| SPI bus | SCK / MISO / MOSI | PA5 / PA6 / PA7 | SPI1, AF5 |
| | CS W25Q flash | PA4 | matches the Black Pill's optional flash footprint (check your board's schematic) |
| | CS BME280 (SPI mode) | PB0 | |
| | CS microSD (optional) | PB1 | |
| Encoder | A / B | PB4 / PB5 | TIM3 CH1/CH2, AF2, encoder mode (PB4 = NJTRST, free with SWD) |
| | SW | PA15 | EXTI15 (JTDI, free with SWD) |
| Sensor interrupt | DRDY | PA8 | EXTI8 |
| Audio — VS1053 option | XCS / XDCS / DREQ / XRST | PA1 / PA2 / PA3 / PB3 | on SPI1; DREQ on EXTI3; PB3 = SWO (lose SWO trace) |
| TFT option (instead of OLED) | SCK / MOSI / CS / DC / RST | PB13 / PB15 / PB12 / PB14 / PB10 | SPI2, AF5 (MISO unused, PB14 used as GPIO) |
| Audio — I2S option | CK / SD / WS | PB13 / PB15 / PB12 | I2S2 on SPI2 — **conflicts with the TFT option**: pick OLED + I2S or TFT + VS1053 |
| RTC | VBAT | VBAT pin | CR2032 for timestamps |

For audio on the STM32, prefer the VS1053 (hardware decoding); software MP3 decoding plus display buffers is tight in the F401's 96 KB RAM.

---

## 6. Open decisions

- [ ] MCU: ESP32-S3 (more flash/RAM, PCNT, I2S, `esp_lcd`) vs STM32F401 (register-level timers/SPI, RTC).
- [ ] Device: BME280 / GY-273 / VS1053 audio — or sensor + magnetometer on both buses.
- [ ] Display: OLED (I2C) vs TFT (SPI).
- [ ] Storage: internal flash only vs W25Q vs SD export.
- [ ] Confirm actual chips on arrival (I2C scan + ID registers) before writing drivers.

---

## Sources

- BME280/BMP280 chip IDs and module pinouts: [esp32.co.uk BMP280 pinout](https://esp32.co.uk/?p=20261017017), [tinkered.ai BME280](https://tinkered.ai/components/bme280), [Switch Science BME280](https://international.switch-science.com/catalog/2236/)
- BMP388/BMP390 IDs: [ESPHome bmp3xx](https://api-docs.esphome.io/bmp3xx__base_8h_source); BMP580: [LKML thread](https://lkml.iu.edu/hypermail/linux/kernel/2301.1/00055.html)
- BME680/688 variant register: [Bosch community](https://community.bosch-sensortec.com/t5/MEMS-sensors-forum/How-to-distinguish-BME680-from-BME688-in-firmware/m-p/73973)
- HMC5883L self-test: [HMC5883L datasheet p.17](https://elcodis.com/parts/2240936/HMC5883L-TR_p17.html); HMC5983: [Linux iio commit](https://mirrors.sustech.edu.cn/git/daimiaomiao/test/-/commit/5a059bd268a79376ebf9ea539c5bb645c5a854d5)
- QMC5883L registers: [RIOT driver](https://doc.riot-os.org/qmc5883l__internal_8h_source.html), [ESPHome QMC5883L](https://esphome.io/components/sensor/qmc5883l/); DA/DB versions: [Paparazzi](https://docs.paparazziuav.org/latest/module__mag_qmc5883l.html)
- QMC5883P: [PX4 forum](https://discuss.px4.io/t/walksnail-ws-m181-compass-not-recognized/47994), [Arduino forum](https://forum.arduino.cc/t/are-all-new-gy-270-board-magnetometers-now-fake/1379276); VCM5883L/orientation: [B4X](https://b4x.com/android/forum/threads/157604)
- GPD2846A: [Arduino forum](https://forum.arduino.cc/t/has-anyone-ever-used-gpd2846a-tf-card-mp3/345185), [manuals.plus](https://manuals.plus/asin/B07M9PHJCY); linked board: [arduino.ua](https://arduino.ua/prod3147-mp3-modyl-progravacha-z-pidtrimkou-sd-kartki-pamyati-ta-usb-nakopichyvacha)
- VS1053 registers: [VS1053 datasheet](https://courses.washington.edu/cp105/_downloads/VS1053_DataSheet.pdf); DFPlayer: [DFRobot wiki](https://github.com/jimaobian/DFRobotWiki/wiki/DFPlayer_Mini_SKU_DFR0299)
