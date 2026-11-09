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
| Джерело живлення | 5V зовнішнє |'


### Вихід програми 

#### Опис Дебаггінгу 

1. Writing a single pulse at a time 

First half is scrolling encoder wheel fast, first in cw, then in ccw direction, second half is rotating one step at a time in ccw directio


```
I (766285) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-100.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-110.00 deg (-1100.00%)
I (766285) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (766335) mini_project_3: COUNT DECREASED BY -2
E (766385) mini_project_3: COUNT DECREASED BY -3
E (766435) mini_project_3: COUNT DECREASED BY -3
E (766485) mini_project_3: COUNT DECREASED BY -4
E (766535) mini_project_3: COUNT DECREASED BY -4
E (766585) mini_project_3: COUNT DECREASED BY -2
E (766635) mini_project_3: COUNT DECREASED BY -3
E (766685) mini_project_3: COUNT DECREASED BY -4
E (766735) mini_project_3: COUNT DECREASED BY -5
E (766785) mini_project_3: COUNT DECREASED BY -4
E (766835) mini_project_3: COUNT DECREASED BY -7
E (766885) mini_project_3: COUNT DECREASED BY -4
E (766935) mini_project_3: COUNT DECREASED BY -3
W (767335) mini_project_3: COUNT INCREASED BY 4
W (767385) mini_project_3: COUNT INCREASED BY 2
W (767435) mini_project_3: COUNT INCREASED BY 2
W (767485) mini_project_3: COUNT INCREASED BY 2
W (767535) mini_project_3: COUNT INCREASED BY 2
W (767635) mini_project_3: COUNT INCREASED BY 2
W (767685) mini_project_3: COUNT INCREASED BY 2
W (767785) mini_project_3: COUNT INCREASED BY 2
W (767835) mini_project_3: COUNT INCREASED BY 2
W (767935) mini_project_3: COUNT INCREASED BY 4
W (767985) mini_project_3: COUNT INCREASED BY 2
W (768035) mini_project_3: COUNT INCREASED BY 2
W (768085) mini_project_3: COUNT INCREASED BY 4
W (768135) mini_project_3: COUNT INCREASED BY 6
W (768185) mini_project_3: COUNT INCREASED BY 4
W (768235) mini_project_3: COUNT INCREASED BY 2
I (768285) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-110.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-120.00 deg (-1200.00%)
I (768285) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (768485) mini_project_3: COUNT DECREASED BY -2
E (768535) mini_project_3: COUNT DECREASED BY -3
E (768585) mini_project_3: COUNT DECREASED BY -3
E (768635) mini_project_3: COUNT DECREASED BY -2
E (768685) mini_project_3: COUNT DECREASED BY -3
E (768735) mini_project_3: COUNT DECREASED BY -3
E (768785) mini_project_3: COUNT DECREASED BY -4
E (768835) mini_project_3: COUNT DECREASED BY -1
E (768885) mini_project_3: COUNT DECREASED BY -3
E (768935) mini_project_3: COUNT DECREASED BY -2
E (768985) mini_project_3: COUNT DECREASED BY -2
E (769085) mini_project_3: COUNT DECREASED BY -2
E (769135) mini_project_3: COUNT DECREASED BY -2
E (769185) mini_project_3: COUNT DECREASED BY -1
E (769235) mini_project_3: COUNT DECREASED BY -3
W (769885) mini_project_3: COUNT INCREASED BY 2
W (769935) mini_project_3: COUNT INCREASED BY 2
W (769985) mini_project_3: COUNT INCREASED BY 2
W (770035) mini_project_3: COUNT INCREASED BY 2
W (770185) mini_project_3: COUNT INCREASED BY 1
W (770235) mini_project_3: COUNT INCREASED BY 1
W (770285) mini_project_3: COUNT INCREASED BY 2
I (770285) mini_project_3: ENCODER_PULSES=1, ENCODER_ANGLE=-170.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-180.00 deg (-1800.00%)
I (770285) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (770745) mini_project_3: COUNT DECREASED BY -2
E (770795) mini_project_3: COUNT DECREASED BY -2
E (770895) mini_project_3: COUNT DECREASED BY -1
E (770945) mini_project_3: COUNT DECREASED BY -1
E (770995) mini_project_3: COUNT DECREASED BY -2
W (771345) mini_project_3: COUNT INCREASED BY 2
W (771395) mini_project_3: COUNT INCREASED BY 2
W (771545) mini_project_3: COUNT INCREASED BY 2
W (771595) mini_project_3: COUNT INCREASED BY 2
W (771745) mini_project_3: COUNT INCREASED BY 2
W (771795) mini_project_3: COUNT INCREASED BY 2
E (772195) mini_project_3: COUNT DECREASED BY -1
E (772245) mini_project_3: COUNT DECREASED BY -3
I (772295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-170.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-180.00 deg (-1800.00%)
I (772295) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (772345) mini_project_3: COUNT DECREASED BY -1
E (772395) mini_project_3: COUNT DECREASED BY -1
E (772445) mini_project_3: COUNT DECREASED BY -2
W (773045) mini_project_3: COUNT INCREASED BY 1
W (773095) mini_project_3: COUNT INCREASED BY 3
W (773245) mini_project_3: COUNT INCREASED BY 2
W (773295) mini_project_3: COUNT INCREASED BY 2
E (773695) mini_project_3: COUNT DECREASED BY -1
E (773745) mini_project_3: COUNT DECREASED BY -3
E (773795) mini_project_3: COUNT DECREASED BY -1
E (773845) mini_project_3: COUNT DECREASED BY -3
E (774295) mini_project_3: COUNT DECREASED BY -1
I (774295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-180.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-190.00 deg (-1900.00%)
I (774295) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (774355) mini_project_3: COUNT DECREASED BY -1
E (774405) mini_project_3: COUNT DECREASED BY -2
```

### Worth mentioning 

#### Quadrature 

```

I (100345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-20.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-30.00 deg (-300.00%)
I (100345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
I (102345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-20.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-30.00 deg (-300.00%)
I (102345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (102595) mini_project_3: COUNT INCREASED BY 1
W (102645) mini_project_3: COUNT INCREASED BY 1
W (102695) mini_project_3: COUNT INCREASED BY 2
W (103245) mini_project_3: COUNT INCREASED BY 1
W (103295) mini_project_3: COUNT INCREASED BY 3
W (103845) mini_project_3: COUNT INCREASED BY 2
W (103895) mini_project_3: COUNT INCREASED BY 2
I (103895) mini_project_3: status update error, at 10 deg
I (104345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (104345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (104495) mini_project_3: COUNT INCREASED BY 2
W (104545) mini_project_3: COUNT INCREASED BY 2
I (104545) mini_project_3: status update error, at 10 deg
W (104945) mini_project_3: COUNT INCREASED BY 1
W (104995) mini_project_3: COUNT INCREASED BY 3
I (104995) mini_project_3: status update error, at 10 deg
W (105145) mini_project_3: COUNT INCREASED BY 2
W (105195) mini_project_3: COUNT INCREASED BY 2
I (105195) mini_project_3: status update error, at 10 deg
W (105245) mini_project_3: COUNT INCREASED BY 2
W (105295) mini_project_3: COUNT INCREASED BY 2
I (105295) mini_project_3: status update error, at 10 deg
W (105395) mini_project_3: COUNT INCREASED BY 2
W (105945) mini_project_3: COUNT INCREASED BY 2
I (105945) mini_project_3: status update error, at 10 deg
W (106145) mini_project_3: COUNT INCREASED BY 1
W (106195) mini_project_3: COUNT INCREASED BY 1
W (106245) mini_project_3: COUNT INCREASED BY 2
I (106245) mini_project_3: status update error, at 10 deg
I (106345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (106345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (106445) mini_project_3: COUNT INCREASED BY 1
W (106495) mini_project_3: COUNT INCREASED BY 2
W (106545) mini_project_3: COUNT INCREASED BY 1
I (106545) mini_project_3: status update error, at 10 deg
W (106995) mini_project_3: COUNT INCREASED BY 1
W (107045) mini_project_3: COUNT INCREASED BY 2
W (107095) mini_project_3: COUNT INCREASED BY 1
I (107095) mini_project_3: status update error, at 10 deg
W (107195) mini_project_3: COUNT INCREASED BY 2
W (107245) mini_project_3: COUNT INCREASED BY 2
I (107245) mini_project_3: status update error, at 10 deg
W (107395) mini_project_3: COUNT INCREASED BY 2
W (107445) mini_project_3: COUNT INCREASED BY 2
I (107445) mini_project_3: status update error, at 10 deg
W (107545) mini_project_3: COUNT INCREASED BY 2
W (107595) mini_project_3: COUNT INCREASED BY 2
I (107595) mini_project_3: status update error, at 10 deg
W (108195) mini_project_3: COUNT INCREASED BY 1
W (108245) mini_project_3: COUNT INCREASED BY 3
I (108245) mini_project_3: status update error, at 10 deg
W (108345) mini_project_3: COUNT INCREASED BY 2
I (108345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (108345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (108405) mini_project_3: COUNT INCREASED BY 2
I (108405) mini_project_3: status update error, at 10 deg
W (108655) mini_project_3: COUNT INCREASED BY 1
W (108955) mini_project_3: COUNT INCREASED BY 1
W (109005) mini_project_3: COUNT INCREASED BY 2
I (109005) mini_project_3: status update error, at 10 deg
W (109855) mini_project_3: COUNT INCREASED BY 1
W (109905) mini_project_3: COUNT INCREASED BY 3
I (109905) mini_project_3: status update error, at 10 deg
I (110355) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (110355) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (110655) mini_project_3: COUNT INCREASED BY 2
W (110705) mini_project_3: COUNT INCREASED BY 2
I (110705) mini_project_3: status update error, at 10 deg
W (111805) mini_project_3: COUNT INCREASED BY 2
W (111855) mini_project_3: COUNT INCREASED BY 2
I (111855) mini_project_3: status update error, at 10 deg
W (112155) mini_project_3: COUNT INCREASED BY 2
W (112205) mini_project_3: COUNT INCREASED BY 2
I (112205) mini_project_3: status update error, at 10 deg
I (112355) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (112355) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (112405) mini_project_3: COUNT INCREASED BY 2
W (112455) mini_project_3: COUNT INCREASED BY 2
I (112455) mini_project_3: status update error, at 10 deg
W (113105) mini_project_3: COUNT INCREASED BY 2
W (113155) mini_project_3: COUNT INCREASED BY 2
I (113155) mini_project_3: status update error, at 10 deg
W (113405) mini_project_3: COUNT INCREASED BY 4
I (113405) mini_project_3: status update error, at 10 deg
W (113505) mini_project_3: COUNT INCREASED BY 4
I (113505) mini_project_3: status update error, at 10 deg
W (113555) mini_project_3: COUNT INCREASED BY 5
I (113555) mini_project_3: status update error, at 10 deg
W (113605) mini_project_3: COUNT INCREASED BY 3
I (113605) mini_project_3: status update error, at 10 deg
W (114005) mini_project_3: COUNT INCREASED BY 5
I (114005) mini_project_3: status update error, at 10 deg
W (114055) mini_project_3: COUNT INCREASED BY 5
I (114055) mini_project_3: status update error, at 10 deg
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