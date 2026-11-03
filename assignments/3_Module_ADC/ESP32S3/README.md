# <3>

> Модуль 3. ЦАП та ШИМ модуляція


> Завдання 3.2 Перетворюємо "сирі" дані ADC: [main\lesson_3.1\main_2_potentiotemer_adc_calbiration.c](.\main_2_potentiotemer_adc_calbiration)



> Завдання 3.3 Керування потужністю та швидкістю: як змінювати яскравість світла та оберти двигуна: [main\lesson_3.1\main_2_potentiotemer_adc_calbiration.c](.\task_3.3_motor_pot.c)


> Завдання 3.4 Зворотна дія: перетворюємо число на напругу (DAC): TODO 


> Завдання 3.5 Точне керування положенням (Серво): [main\lesson_3.5\main_3.5_2_pot_to_motor_map.c](.\task_3.3_motor_pot.c)


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
