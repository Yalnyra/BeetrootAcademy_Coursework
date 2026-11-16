# Міні-проєкт 3 — Керування сервоприводом енкодером
> Модуль 3. ЦАП та ШИМ модуляція


Основна частина проєкту - вивчити як підібрати аналогові елементи в RC фільтрі, під довжину брязкоту, використовуючи логічний аналізатор 

Більше про лог розробки та аналіз дебансингу енкодера в папці [/docs](../docs/debouncing.md)


<!-- Чернетка допису:

-->


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/5414e24e-41d8-4ac3-a82e-da22955947e3)
[Вивід в консоль](..\..\..\output\miniproject_output.log)

### Вимоги до програми (Код)

Завдання - Керування сервоприводом

Керувати сервоприводом за допомогою енкодера
Один тік енкодера повертає вал сервоприводу на певний кут у відповідному напрямку
Натискання кнопки енкодера зменшує кут повороту вала сервоприводу удвічі на кожен тік.
Повторне натискання кнопки повертає точність до більшого кута на кожен тік.
Коли вал сервоприводу у крайніх положеннях, програвати короткий сигнал у разі спроби прокрутити вал за межі.
Опційно: довге утримання кнопки переводить вал сервоприводу у центральне положення.

### Як працює програма 

[Блок-схема програми](https://www.tldraw.com/f/smqRW6pEFIte72JPZ8eqK)
0. Ініціалізується ticks, speed_k, button_state, структура servo_status 
ticks - це віднос

speed_k - Це мультиплікатор обертів сервомотора відносно обертів енкодера

Структура servo_status_t - зберігає проміжні змінні для обрахування кута серво на поточному етапі 

1. Кожен цикл CPU Оновлюється значення ticks; Якщо ticks не нуль - лише тоді оновлюється положення серво через модифікацію servo_status, інакше CPU пропускає дію

3. Конвертація з положення енкодера у duty cycle, враховуючи обмеження обертів серво у 180 градусів

4. Кнопка поки що нічого не робить

5. Якщо серво оновилось та більше за крайні значення, баззер грає звук 

6. Раз у 2 секунди викликається лог значень записаних у servo_status

7.  Цикл повторюється

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

Описано детально у [експериментальному звіті за дебаунсінг:](../docs/debouncing.md)

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