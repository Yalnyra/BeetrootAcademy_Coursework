# <3.5>

> Модуль 3. ЦАП та ШИМ модуляція

> Завдання 3.5 Точне керування положенням (Серво)  


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/1d9b4079-1a9c-4b3b-b021-867db9eda888)
[Вивід в консоль](output/3.5_output.log)

### Вимоги до програми (Код)

Потенціометр керує положенням валу сервоприводу в пропорції 1:1.
Розбіжність кута повороту сервоприводу та потенціометра обрізаються і використовується тільки та частина діапазону, котра збігатися.
Кут відхилення від крайнього лівого положення логується в консолі.
### Що виконано

1. 0-4095 (POT_GPIO4) -> 0-270 (POT_ANGLE) (Мапа на кут Потенціометра) 
2. 0-270 -> (SERVO_ANGLE) 0-180 (Обрізання до положення сервомотора SG90) -> 
3. 0-180 -> 1-2 мс (Маппінг у довжину PWM пульсу) 
4. 1-2мс (SERVO_PULSE) -> Ширина duty cycle (SERVO_DUTY) 1/50 - 1/25 Гц 
5. Помилка між POT_ANGLE та SERVO_PULSE, конвертований в кут, з функцією duty_to_servo_angle_deg()

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

I (134265) hw_3_5: POT_GPIO4=0, POT_ANGLE=0.00/270.00 deg, SERVO_ANGLE0.00/180.00 deg,  ANGLE_ERROR=0.00%
I (134265) hw_3_5: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001ms, 
I (134765) hw_3_5: POT_GPIO4=0, POT_ANGLE=0.00/270.00 deg, SERVO_ANGLE0.00/180.00 deg,  ANGLE_ERROR=0.00%
I (134765) hw_3_5: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001ms, 
I (135265) hw_3_5: POT_GPIO4=171, POT_ANGLE=11.27/270.00 deg, SERVO_ANGLE11.27/180.00 deg,  ANGLE_ERROR=-1.47%
I (135265) hw_3_5: SERVO_DUTY=436 ticks, SERVO_PULSE=1.064ms, 
I (135765) hw_3_5: POT_GPIO4=731, POT_ANGLE=48.20/270.00 deg, SERVO_ANGLE47.01/180.00 deg,  ANGLE_ERROR=-0.17%
I (135765) hw_3_5: SERVO_DUTY=517 ticks, SERVO_PULSE=1.262ms, 
I (136265) hw_3_5: POT_GPIO4=711, POT_ANGLE=46.88/270.00 deg, SERVO_ANGLE46.35/180.00 deg,  ANGLE_ERROR=0.31%
I (136265) hw_3_5: SERVO_DUTY=515 ticks, SERVO_PULSE=1.257ms, 
I (136765) hw_3_5: POT_GPIO4=1007, POT_ANGLE=66.40/270.00 deg, SERVO_ANGLE66.40/180.00 deg,  ANGLE_ERROR=-0.09%
I (136765) hw_3_5: SERVO_DUTY=561 ticks, SERVO_PULSE=1.370ms, 
I (137265) hw_3_5: POT_GPIO4=1979, POT_ANGLE=130.48/270.00 deg, SERVO_ANGLE128.64/180.00 deg,  ANGLE_ERROR=0.10%
I (137265) hw_3_5: SERVO_DUTY=702 ticks, SERVO_PULSE=1.714ms, 
I (137765) hw_3_5: POT_GPIO4=2941, POT_ANGLE=193.91/270.00 deg, SERVO_ANGLE180.00/180.00 deg,  ANGLE_ERROR=4.73%
I (137765) hw_3_5: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000ms, 
I (138265) hw_3_5: POT_GPIO4=3590, POT_ANGLE=236.70/270.00 deg, SERVO_ANGLE180.00/180.00 deg,  ANGLE_ERROR=31.50%
I (138265) hw_3_5: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000ms, 
I (138765) hw_3_5: POT_GPIO4=3307, POT_ANGLE=218.04/270.00 deg, SERVO_ANGLE180.00/180.00 deg,  ANGLE_ERROR=21.21%
I (138765) hw_3_5: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000ms, 
I (139265) hw_3_5: POT_GPIO4=3179, POT_ANGLE=209.60/270.00 deg, SERVO_ANGLE180.00/180.00 deg,  ANGLE_ERROR=15.75%
I (139265) hw_3_5: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000ms, 


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