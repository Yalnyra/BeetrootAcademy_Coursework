# <3.4>

> Модуль 3. ЦАП та ШИМ модуляція

> Завдання 3.4 Зворотна дія: перетворюємо число на напругу (DAC): TODO 


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/1d9b4079-1a9c-4b3b-b021-867db9eda888)
[Вивід в консоль](output/3.3_output.log)

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_2.7.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)

- Потенціометр керує яскравістю світлодіода.
- Потенціометр керує швидкістю вала двигуна.
- Двигун та світлодіод не впливають на роботу одне одного.

Двигун замінено на серво (треба замінити мотор)

### Що виконано

Обрано використовувати два окремих таймери, бо LED та Серво працюють на різних частотах: 1024 Гц та 50 Гц відповідно

0. Виправлено помилки з передаванням об'єкту adc_handle для ініціалізації потенціометра (Передача параметру значенням, а не вказівником)

1. Керування положенням серво за допомогою LEDC
Ініціалізація мотора SG90: [Даташит](https://www.friendlywire.com/projects/ne555-servo-safe/SG90-datasheet.pdf)

2. Перевірка потужності пінів 2 та 7
GPIO7  між землею та PWM: 0.17-0.31V від положення 0 до 180 градусів.
Між VCC та GND: 6.07v

2. Оновлення швидкості обертання за допомогою софтверного таймера 
xTaskGetTickCount()

## Набір деталей

| Деталь | Опис |
|------|--------|
| Плата | ESP32-S3 (devkit) |
| Потенціометр | x1 |
| Резистор | x1 220 Ом |
| Світлодіод | x1 Червоний |
| Сервопривід | x1 SG90 (Tower Pro) |
| Джерело живлення | 5V зовнішнє |

### Вихід програми 
```
I (13335) hw_3_3: SERVO SET SUCCESS=1, period=500 ms
I (13775) hw_3_3: POT_GPIO4=0, SERVO_PERIOD=500 ms, SERVO_ANGLE=1440 deg/180, GPIO7_HIGH=102/256, LED_GPIO6=1070162112/1023
I (13835) hw_3_3: SERVO SET SUCCESS=1, period=500 ms
I (14275) hw_3_3: POT_GPIO4=181, SERVO_PERIOD=481 ms, SERVO_ANGLE=1485 deg/180, GPIO7_HIGH=143/256, LED_GPIO6=1070162112/1023
I (14315) hw_3_3: SERVO SET SUCCESS=1, period=477 ms
I (14755) hw_3_3: SERVO SET SUCCESS=1, period=442 ms
I (14775) hw_3_3: POT_GPIO4=546, SERVO_PERIOD=441 ms, SERVO_ANGLE=1575 deg/180, GPIO7_HIGH=225/256, LED_GPIO6=1070162112/1023
I (15175) hw_3_3: SERVO SET SUCCESS=1, period=418 ms
I (15275) hw_3_3: POT_GPIO4=814, SERVO_PERIOD=413 ms, SERVO_ANGLE=1620 deg/180, GPIO7_HIGH=285/256, LED_GPIO6=1070162112/1023
I (15575) hw_3_3: SERVO SET SUCCESS=1, period=398 ms
I (15775) hw_3_3: POT_GPIO4=1031, SERVO_PERIOD=389 ms, SERVO_ANGLE=1665 deg/180, GPIO7_HIGH=334/256, LED_GPIO6=1070162112/1023
I (15955) hw_3_3: SERVO SET SUCCESS=1, period=373 ms
I (16275) hw_3_3: POT_GPIO4=1575, SERVO_PERIOD=331 ms, SERVO_ANGLE=1710 deg/180, GPIO7_HIGH=457/256, LED_GPIO6=1070162112/1023
I (16295) hw_3_3: SERVO SET SUCCESS=1, period=327 ms
I (16595) hw_3_3: SERVO SET SUCCESS=1, period=309 ms
I (16775) hw_3_3: POT_GPIO4=1835, SERVO_PERIOD=303 ms, SERVO_ANGLE=1800 deg/180, GPIO7_HIGH=515/256, LED_GPIO6=1070162112/1023
I (16895) hw_3_3: SERVO SET SUCCESS=1, period=299 ms
I (17175) hw_3_3: SERVO SET SUCCESS=1, period=284 ms
I (17275) hw_3_3: POT_GPIO4=2056, SERVO_PERIOD=279 ms, SERVO_ANGLE=1890 deg/180, GPIO7_HIGH=565/256, LED_GPIO6=1070162112/1023
I (17435) hw_3_3: SERVO SET SUCCESS=1, period=265 ms
I (17695) hw_3_3: SERVO SET SUCCESS=1, period=253 ms
I (17775) hw_3_3: POT_GPIO4=2355, SERVO_PERIOD=247 ms, SERVO_ANGLE=1980 deg/180, GPIO7_HIGH=632/256, LED_GPIO6=1070162112/1023
I (17935) hw_3_3: SERVO SET SUCCESS=1, period=235 ms
I (18155) hw_3_3: SERVO SET SUCCESS=1, period=219 ms
I (18275) hw_3_3: POT_GPIO4=2724, SERVO_PERIOD=207 ms, SERVO_ANGLE=2070 deg/180, GPIO7_HIGH=715/256, LED_GPIO6=1070162112/1023
I (18355) hw_3_3: SERVO SET SUCCESS=1, period=198 ms
I (18535) hw_3_3: SERVO SET SUCCESS=1, period=177 ms
I (18695) hw_3_3: SERVO SET SUCCESS=1, period=152 ms
I (18775) hw_3_3: POT_GPIO4=3352, SERVO_PERIOD=140 ms, SERVO_ANGLE=2205 deg/180, GPIO7_HIGH=857/256, LED_GPIO6=1070162112/1023
I (18835) hw_3_3: SERVO SET SUCCESS=1, period=127 ms
I (18955) hw_3_3: SERVO SET SUCCESS=1, period=108 ms
I (19055) hw_3_3: SERVO SET SUCCESS=1, period=87 ms
I (19135) hw_3_3: SERVO SET SUCCESS=1, period=71 ms
I (19195) hw_3_3: SERVO SET SUCCESS=1, period=61 ms
I (19255) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19275) hw_3_3: POT_GPIO4=4095, SERVO_PERIOD=60 ms, SERVO_ANGLE=2475 deg/180, GPIO7_HIGH=1024/256, LED_GPIO6=1070162112/1023
I (19315) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19375) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19435) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19495) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19555) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19615) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
I (19675) hw_3_3: SERVO SET SUCCESS=1, period=60 ms
```
## Передумови (версії, з якими зібрано проєкт)

| Компонент | Версія / шлях |
|------|--------|
| ESP-IDF | **v6.1.0** (`C:\esp\v6.1\esp-idf`) |
| Target | `esp32s3` (Xtensa) |
| Компілятор | `xtensa-esp32s3-elf-gcc` **15.2.0** (`esp-15.2.0_20251204`) |
| CMake | **4.0.3** (`C:\Espressif\tools\cmake\4.0.3`), мінімум у проєкті — 3.22 |
| Генератор | Ninja (`C:\Strawberry\c\bin\ninja.exe`) |
| Python (venv IDF) | **3.11.15** (`C:\Espressif\tools\python\v6.1\venv`) |
| clangd | esp-clangd **21.1.3** (`esp-21.1.3_20260408`) |
| VS Code розширення | `espressif.esp-idf-extension` |
| ОС | Windows 11 (є також `.devcontainer` з образом `espressif/idf`) |

Версії взято з `sdkconfig`, `build/project_description.json` та `build/CMakeCache.txt`.

## Швидкий старт

**Увага: PlatformIO депрекований, з метою побудови використовується плагін ESP-IDF**

Команди викликаються з палітри VS Code (`Ctrl+Shift+P`). В дужках — еквівалент `idf.py`
для терміналу `ESP-IDF: Open ESP-IDF Terminal`.

0. Вибрати файл для збірки в `main/CMakeLists.txt` — в `SRCS` має бути **існуючий** файл:

   ```cmake
   idf_component_register(SRCS "main_6_miniproject.c"
                          PRIV_REQUIRES esp_driver_gpio esp_timer)
   ```

1. Встановити ESP-IDF
   - `ESP-IDF: Open ESP-IDF Installation Manager` → встановити **v6.1** в `C:\esp\v6.1\esp-idf`
   - `ESP-IDF: Select Current ESP-IDF Version` → v6.1 (записується в `idf.currentSetup`)
   - `ESP-IDF: Set Espressif Device Target` → `esp32s3` (`idf.py set-target esp32s3`)
   - `ESP-IDF: Select Port to Use (COM, tty, usbserial)` → `COM16`

2. Run CMake Clean
   - `ESP-IDF: Full Clean Project` (`idf.py fullclean`) — видаляє `build/`, потрібно після зміни
     `SRCS`, версії IDF або target

3. Запустити перший білд
   - `ESP-IDF: Build your Project` / `Ctrl+E B` (`idf.py build`)
   - Після білду з'являється `build/compile_commands.json` — його читає clangd

4. Встановити flashing метод UART
   - `ESP-IDF: Select Flash Method` → `UART` (записується в `idf.flashType`)

5. Запустити Flash & Monitor task
   - `ESP-IDF: Build, Flash and Start a Monitor on your Device` / `Ctrl+E D`
     (`idf.py -p COM16 flash monitor`)
   - Окремо: Flash — `Ctrl+E F`, Monitor — `Ctrl+E M`; вихід з монітора — `Ctrl+]`
   - Консоль: UART0 @ **115200**, другорядна — USB-Serial-JTAG

### Конфігурація `.vscode`

`.vscode/settings.json` — налаштування розширення ESP-IDF та clangd:

```json
{
  "C_Cpp.intelliSenseEngine": "default",
  "idf.extensionActivationMode": "always",
  "cmake.configureOnOpen": false,
  "cmake.configureOnEdit": false,
  "cmake.automaticReconfigure": false,
  "cmake.autoSelectActiveFolder": false,
  "idf.openOcdConfigs": ["board/esp32s3-builtin.cfg"],
  "idf.port": "COM16",
  "idf.currentSetup": "C:\\esp\\v6.1\\esp-idf",
  "idf.customExtraVars": { "IDF_TARGET": "esp32s3" },
  "idf.flashType": "UART",
  "clangd.path": "C:\\Espressif\\tools\\esp-clangd\\esp-21.1.3_20260408\\esp-clangd\\bin\\clangd.exe",
  "clangd.arguments": [
    "--background-index",
    "--query-driver=**",
    "--compile-commands-dir=d:\\!SOFT\\!Documents\\embed\\training\\beetroot\\assignments\\2.6_ESPIDF_Debounce\\build"
  ]
}
```

- `cmake.*: false` — вимикає CMake Tools, щоб він не конфігурував проєкт паралельно з ESP-IDF
- `idf.openOcdConfigs` — вбудований USB-JTAG ESP32-S3 для налагодження
- `--compile-commands-dir` — абсолютний шлях; при перенесенні проєкту його треба змінити

`.vscode/c_cpp_properties.json` — IntelliSense бере прапорці з `${config:idf.buildPath}/compile_commands.json`.

`.vscode/launch.json` — конфігурація `gdbtarget` / `attach` (Eclipse CDT GDB Adapter) для
налагодження через OpenOCD.

`.clangd` — `Remove: [-f*, -m*]` прибирає специфічні для Xtensa GCC прапорці, які clangd не розуміє.

## Згенерувати плоти 

### Вимоги 
- Python 1.13.13
- pyserial 3.5
- matplotlib==

### Записати лог


Задайте шлях до логів

src\save_serial_output.py

```python
LOG_PATH = Path(__file__).parent / ".." / "dummy.log" # Шлях до логу
SERIAL_PATH = "COM16" # Порт UART (JTAG) на USB32, має співпадати з 
# platformio.ini: monitor_port = COM16
```
Видає тільки ./<logname>.log

## What I would add

- LDR resistor ADC control

## TODO
- [] Add hardware requirements
- [] Add circuit diagram 
- [] Add code function comments
- [] Add debugging log 