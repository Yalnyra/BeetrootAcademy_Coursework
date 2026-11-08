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
```I (242) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (248) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (255) main_task: Started on CPU0
I (255) main_task: Calling app_main()
I (265) mini_project_3: PCNT X1 initialized
I (265) mini_project_3: superloop started, delay 20 ms
I (265) mini_project_3: PCNT filtered: count=1 delta=+1
I (295) mini_project_3: servo idle, at 10 deg
I (315) mini_project_3: servo idle, at 10 deg
I (335) mini_project_3: servo idle, at 10 deg
I (355) mini_project_3: servo idle, at 10 deg
I (375) mini_project_3: servo idle, at 10 deg
I (395) mini_project_3: servo idle, at 10 deg
I (415) mini_project_3: servo idle, at 10 deg
I (435) mini_project_3: servo idle, at 10 deg
I (455) mini_project_3: servo idle, at 10 deg
I (475) mini_project_3: servo idle, at 10 deg
I (495) mini_project_3: servo idle, at 10 deg
I (515) mini_project_3: servo idle, at 10 deg
I (535) mini_project_3: servo idle, at 10 deg
I (555) mini_project_3: servo idle, at 10 deg
I (575) mini_project_3: servo idle, at 10 deg
I (595) mini_project_3: servo idle, at 10 deg
I (615) mini_project_3: servo idle, at 10 deg
I (635) mini_project_3: servo idle, at 10 deg
I (655) mini_project_3: servo idle, at 10 deg
I (675) mini_project_3: servo idle, at 10 deg
I (695) mini_project_3: servo idle, at 10 deg
I (715) mini_project_3: servo idle, at 10 deg
I (735) mini_project_3: servo idle, at 10 deg
I (755) mini_project_3: servo idle, at 10 deg
I (775) mini_project_3: servo idle, at 10 deg
I (775) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=10.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (775) mini_project_3: SERVO_DUTY=432 ticks, SERVO_PULSE=1.055 ms, SPEED_K=2
I (805) mini_project_3: servo idle, at 10 deg
I (825) mini_project_3: servo idle, at 10 deg
I (845) mini_project_3: servo idle, at 10 deg
I (865) mini_project_3: servo idle, at 10 deg
I (885) mini_project_3: servo idle, at 10 deg
I (905) mini_project_3: servo idle, at 10 deg
I (925) mini_project_3: servo idle, at 10 deg
I (945) mini_project_3: servo idle, at 10 deg
I (965) mini_project_3: servo idle, at 10 deg
I (985) mini_project_3: servo idle, at 10 deg
I (1005) mini_project_3: servo idle, at 10 deg
I (1025) mini_project_3: servo idle, at 10 deg
I (1045) mini_project_3: servo idle, at 10 deg
I (1065) mini_project_3: servo idle, at 10 deg
I (1085) mini_project_3: servo idle, at 10 deg
I (1105) mini_project_3: servo idle, at 10 deg
I (1125) mini_project_3: servo idle, at 10 deg
I (1145) mini_project_3: servo idle, at 10 deg
I (1165) mini_project_3: servo idle, at 10 deg
I (1185) mini_project_3: servo idle, at 10 deg
I (1205) mini_project_3: servo idle, at 10 deg
I (1225) mini_project_3: servo idle, at 10 deg
I (1245) mini_project_3: servo idle, at 10 deg
I (1265) mini_project_3: servo idle, at 10 deg
I (1285) mini_project_3: servo idle, at 10 deg
I (1285) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=10.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (1285) mini_project_3: SERVO_DUTY=432 ticks, SERVO_PULSE=1.055 ms, SPEED_K=2
I (1315) mini_project_3: servo idle, at 10 deg
I (1335) mini_project_3: servo idle, at 10 deg
I (1355) mini_project_3: servo idle, at 10 deg
I (1375) mini_project_3: servo idle, at 10 deg
I (1395) mini_project_3: servo idle, at 10 deg
I (1415) mini_project_3: servo idle, at 10 deg
I (1435) mini_project_3: servo idle, at 10 deg
I (1455) mini_project_3: servo idle, at 10 deg
I (1475) mini_project_3: servo idle, at 10 deg
I (1495) mini_project_3: servo idle, at 10 deg
I (1515) mini_project_3: servo idle, at 10 deg
I (1535) mini_project_3: servo idle, at 10 deg
I (1555) mini_project_3: servo idle, at 10 deg
I (1575) mini_project_3: servo idle, at 10 deg
I (1595) mini_project_3: servo idle, at 10 deg
I (1615) mini_project_3: servo idle, at 10 deg
I (1635) mini_project_3: servo idle, at 10 deg
I (1655) mini_project_3: servo idle, at 10 deg
I (1675) mini_project_3: servo idle, at 10 deg
I (1695) mini_project_3: servo idle, at 10 deg
I (1715) mini_project_3: servo idle, at 10 deg
I (1735) mini_project_3: servo idle, at 10 deg
I (1755) mini_project_3: servo idle, at 10 deg
I (1775) mini_project_3: servo idle, at 10 deg
I (1795) mini_project_3: servo idle, at 10 deg
I (1795) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=10.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (1795) mini_project_3: SERVO_DUTY=432 ticks, SERVO_PULSE=1.055 ms, SPEED_K=2
I (1825) mini_project_3: servo idle, at 10 deg
I (1845) mini_project_3: servo idle, at 10 deg
I (1865) mini_project_3: servo idle, at 10 deg
I (1885) mini_project_3: servo idle, at 10 deg
I (1905) mini_project_3: servo idle, at 10 deg
I (1925) mini_project_3: servo idle, at 10 deg
I (1945) mini_project_3: servo idle, at 10 deg
I (1965) mini_project_3: servo idle, at 10 deg
I (1985) mini_project_3: servo idle, at 10 deg
I (2005) mini_project_3: servo idle, at 10 deg
I (2025) mini_project_3: servo idle, at 10 deg
I (2045) mini_project_3: servo idle, at 10 deg
I (2065) mini_project_3: servo idle, at 10 deg
I (2085) mini_project_3: servo idle, at 10 deg
I (2105) mini_project_3: servo idle, at 10 deg
I (2125) mini_project_3: servo idle, at 10 deg
I (2145) mini_project_3: servo idle, at 10 deg
I (2165) mini_project_3: servo idle, at 10 deg
I (2185) mini_project_3: servo idle, at 10 deg
I (2205) mini_project_3: servo idle, at 10 deg
I (2225) mini_project_3: servo idle, at 10 deg
I (2245) mini_project_3: servo idle, at 10 deg
I (2265) mini_project_3: servo idle, at 10 deg
I (2285) mini_project_3: servo idle, at 10 deg
I (2305) mini_project_3: servo idle, at 10 deg
I (2305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=10.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (2305) mini_project_3: SERVO_DUTY=432 ticks, SERVO_PULSE=1.055 ms, SPEED_K=2
I (2335) mini_project_3: servo idle, at 10 deg
I (2355) mini_project_3: servo idle, at 10 deg
I (2375) mini_project_3: servo idle, at 10 deg
I (2395) mini_project_3: servo idle, at 10 deg
I (2415) mini_project_3: servo idle, at 10 deg
I (2435) mini_project_3: servo idle, at 10 deg
I (2455) mini_project_3: servo idle, at 10 deg
I (2475) mini_project_3: servo idle, at 10 deg
I (2495) mini_project_3: servo idle, at 10 deg
I (2515) mini_project_3: servo idle, at 10 deg
I (2535) mini_project_3: servo idle, at 10 deg
I (2555) mini_project_3: servo idle, at 10 deg
I (2575) mini_project_3: servo idle, at 10 deg
I (2595) mini_project_3: servo idle, at 10 deg
I (2615) mini_project_3: servo idle, at 10 deg
I (2635) mini_project_3: servo idle, at 10 deg
I (2655) mini_project_3: servo idle, at 10 deg
I (2675) mini_project_3: servo idle, at 10 deg
I (2695) mini_project_3: servo idle, at 10 deg
I (2715) mini_project_3: servo idle, at 10 deg
I (2735) mini_project_3: servo idle, at 10 deg
I (2755) mini_project_3: servo idle, at 10 deg
I (2775) mini_project_3: servo idle, at 10 deg
I (2795) mini_project_3: servo idle, at 10 deg
I (2815) mini_project_3: servo idle, at 10 deg
I (2815) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=10.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (2815) mini_project_3: SERVO_DUTY=432 ticks, SERVO_PULSE=1.055 ms, SPEED_K=2
I (2845) mini_project_3: servo idle, at 10 deg
I (2865) mini_project_3: servo idle, at 10 deg
I (2885) mini_project_3: servo idle, at 10 deg
I (2905) mini_project_3: servo idle, at 10 deg
I (2925) mini_project_3: servo idle, at 10 deg
I (2945) mini_project_3: servo idle, at 10 deg
I (2965) mini_project_3: servo idle, at 10 deg
I (2985) mini_project_3: servo idle, at 10 deg
I (3005) mini_project_3: servo idle, at 10 deg
I (3025) mini_project_3: servo idle, at 10 deg
I (3045) mini_project_3: servo idle, at 10 deg
I (3065) mini_project_3: servo idle, at 10 deg
I (3085) mini_project_3: servo idle, at 10 deg
I (3105) mini_project_3: servo idle, at 10 deg
I (3125) mini_project_3: PCNT filtered: count=2 delta=+1
I (3145) mini_project_3: servo idle, at 20 deg
I (3165) mini_project_3: servo idle, at 20 deg
I (3185) mini_project_3: servo idle, at 20 deg
I (3205) mini_project_3: servo idle, at 20 deg
I (3225) mini_project_3: servo idle, at 20 deg
I (3245) mini_project_3: servo idle, at 20 deg
I (3265) mini_project_3: servo idle, at 20 deg
I (3285) mini_project_3: servo idle, at 20 deg
I (3305) mini_project_3: servo idle, at 20 deg
I (3325) mini_project_3: servo idle, at 20 deg
I (3325) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=20.00 deg, SERVO_ANGLE=20/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (3325) mini_project_3: SERVO_DUTY=455 ticks, SERVO_PULSE=1.111 ms, SPEED_K=2
I (3355) mini_project_3: servo idle, at 20 deg
I (3375) mini_project_3: servo idle, at 20 deg
I (3395) mini_project_3: servo idle, at 20 deg
I (3415) mini_project_3: servo idle, at 20 deg
I (3435) mini_project_3: servo idle, at 20 deg
I (3455) mini_project_3: servo idle, at 20 deg
I (3475) mini_project_3: servo idle, at 20 deg
I (3495) mini_project_3: servo idle, at 20 deg
I (3515) mini_project_3: servo idle, at 20 deg
I (3535) mini_project_3: servo idle, at 20 deg
I (3555) mini_project_3: servo idle, at 20 deg
I (3575) mini_project_3: PCNT filtered: count=4 delta=+2
I (3595) mini_project_3: servo idle, at 40 deg
I (3615) mini_project_3: servo idle, at 40 deg
I (3635) mini_project_3: servo idle, at 40 deg
I (3655) mini_project_3: servo idle, at 40 deg
I (3675) mini_project_3: servo idle, at 40 deg
I (3695) mini_project_3: servo idle, at 40 deg
I (3715) mini_project_3: servo idle, at 40 deg
I (3735) mini_project_3: servo idle, at 40 deg
I (3755) mini_project_3: servo idle, at 40 deg
I (3775) mini_project_3: servo idle, at 40 deg
I (3795) mini_project_3: servo idle, at 40 deg
I (3815) mini_project_3: servo idle, at 40 deg
I (3835) mini_project_3: servo idle, at 40 deg
I (3835) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=40.00 deg, SERVO_ANGLE=40/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (3835) mini_project_3: SERVO_DUTY=500 ticks, SERVO_PULSE=1.221 ms, SPEED_K=2
I (3865) mini_project_3: servo idle, at 40 deg
I (3885) mini_project_3: PCNT filtered: count=5 delta=+1
I (3905) mini_project_3: servo idle, at 50 deg
I (3925) mini_project_3: servo idle, at 50 deg
I (3945) mini_project_3: servo idle, at 50 deg
I (3965) mini_project_3: servo idle, at 50 deg
I (3985) mini_project_3: servo idle, at 50 deg
I (4005) mini_project_3: servo idle, at 50 deg
I (4025) mini_project_3: servo idle, at 50 deg
I (4045) mini_project_3: servo idle, at 50 deg
I (4065) mini_project_3: servo idle, at 50 deg
I (4085) mini_project_3: servo idle, at 50 deg
I (4105) mini_project_3: servo idle, at 50 deg
I (4125) mini_project_3: servo idle, at 50 deg
I (4145) mini_project_3: servo idle, at 50 deg
I (4165) mini_project_3: servo idle, at 50 deg
I (4185) mini_project_3: servo idle, at 50 deg
I (4205) mini_project_3: servo idle, at 50 deg
I (4225) mini_project_3: servo idle, at 50 deg
I (4245) mini_project_3: servo idle, at 50 deg
I (4265) mini_project_3: PCNT filtered: count=6 delta=+1
I (4285) mini_project_3: servo idle, at 60 deg
I (4305) mini_project_3: servo idle, at 60 deg
I (4325) mini_project_3: servo idle, at 60 deg
I (4345) mini_project_3: servo idle, at 60 deg
I (4345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=60.00 deg, SERVO_ANGLE=60/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (4345) mini_project_3: SERVO_DUTY=546 ticks, SERVO_PULSE=1.333 ms, SPEED_K=2
I (4375) mini_project_3: servo idle, at 60 deg
I (4395) mini_project_3: servo idle, at 60 deg
I (4415) mini_project_3: servo idle, at 60 deg
I (4435) mini_project_3: PCNT filtered: count=7 delta=+1
I (4455) mini_project_3: servo idle, at 70 deg
I (4475) mini_project_3: servo idle, at 70 deg
I (4495) mini_project_3: servo idle, at 70 deg
I (4515) mini_project_3: servo idle, at 70 deg
I (4535) mini_project_3: servo idle, at 70 deg
I (4555) mini_project_3: servo idle, at 70 deg
I (4575) mini_project_3: servo idle, at 70 deg
I (4595) mini_project_3: servo idle, at 70 deg
I (4615) mini_project_3: servo idle, at 70 deg
I (4635) mini_project_3: servo idle, at 70 deg
I (4655) mini_project_3: servo idle, at 70 deg
I (4675) mini_project_3: servo idle, at 70 deg
I (4695) mini_project_3: servo idle, at 70 deg
I (4715) mini_project_3: servo idle, at 70 deg
I (4735) mini_project_3: servo idle, at 70 deg
I (4755) mini_project_3: servo idle, at 70 deg
I (4775) mini_project_3: servo idle, at 70 deg
I (4795) mini_project_3: servo idle, at 70 deg
I (4815) mini_project_3: servo idle, at 70 deg
I (4835) mini_project_3: servo idle, at 70 deg
I (4855) mini_project_3: servo idle, at 70 deg
I (4855) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=70.00 deg, SERVO_ANGLE=70/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (4855) mini_project_3: SERVO_DUTY=569 ticks, SERVO_PULSE=1.389 ms, SPEED_K=2
I (4885) mini_project_3: servo idle, at 70 deg
I (4905) mini_project_3: servo idle, at 70 deg
I (4925) mini_project_3: servo idle, at 70 deg
I (4945) mini_project_3: servo idle, at 70 deg


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