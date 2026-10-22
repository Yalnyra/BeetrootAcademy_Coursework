# <NN_ProjectName>

> EMB_25 Завдання <N.N>: <тема>
> Handout: [`assignments/EMB_25_HW<NN>.pdf`](../../assignments/EMB_25_HW<NN>.pdf)

### Схемотехніка

[Посилання на Діаграму](<https://app.cirkitdesigner.com/project/...>)
[Діаграма машини станів](<посилання>)
[Вивід в консоль](output/<N.N>_output.log)

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_<N.N>.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)

<Що вимагає завдання — один короткий абзац.>

- <вимога 1>
- <вимога 2>

## Набір деталей

| Деталь | Опис |
|------|--------|
| Плата | ESP32-S3 N16R8 (16 MB flash, 8 MB PSRAM) |
| <Деталь> | <кількість, номінал> |
| Кабель | USB Type-C ↔ ПК |

### Підключення

| Сигнал | GPIO | Пін периферії |
|--------|------|----------------|
| <сигнал> | <GPIOxx> | <пін> |

### Вихід програми
```
I (<ms>) <TAG>: <повідомлення>
```

## Передумови (версії, з якими зібрано проєкт)

| Компонент | Версія / шлях |
|------|--------|
| ESP-IDF | **<v6.1.0>** (`C:\esp\v6.1\esp-idf`) |
| Target | `esp32s3` (Xtensa) |
| Компілятор | `xtensa-esp32s3-elf-gcc` **<15.2.0>** |
| CMake | **<4.0.3>**, мінімум у проєкті — 3.22 |
| Генератор | Ninja |
| Python (venv IDF) | **<3.11.x>** (`C:\Espressif\tools\python\v6.1\venv`) |
| clangd | esp-clangd **<21.1.3>** |
| VS Code розширення | `espressif.esp-idf-extension` |
| ОС | Windows 11 |

Версії взято з `sdkconfig`, `build/project_description.json` та `build/CMakeCache.txt`.

## Швидкий старт

Команди викликаються з палітри VS Code (`Ctrl+Shift+P`). В дужках — еквівалент `idf.py`
для терміналу `ESP-IDF: Open ESP-IDF Terminal`.

0. Вибрати файл для збірки в `main/CMakeLists.txt` — в `SRCS` має бути **існуючий** файл:

   ```cmake
   idf_component_register(SRCS "<main_file>.c"
                          PRIV_REQUIRES esp_driver_gpio esp_timer)
   ```

1. Встановити ESP-IDF
   - `ESP-IDF: Open ESP-IDF Installation Manager` → встановити **v6.1** в `C:\esp\v6.1\esp-idf`
   - `ESP-IDF: Select Current ESP-IDF Version` → v6.1 (записується в `idf.currentSetup`)
   - `ESP-IDF: Set Espressif Device Target` → `esp32s3` (`idf.py set-target esp32s3`)
   - `ESP-IDF: Select Port to Use (COM, tty, usbserial)` → `<COMxx>`

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
     (`idf.py -p <COMxx> flash monitor`)
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
  "idf.port": "<COMxx>",
  "idf.currentSetup": "C:\\esp\\v6.1\\esp-idf",
  "idf.customExtraVars": { "IDF_TARGET": "esp32s3" },
  "idf.flashType": "UART",
  "clangd.path": "C:\\Espressif\\tools\\esp-clangd\\<version>\\esp-clangd\\bin\\clangd.exe",
  "clangd.arguments": [
    "--background-index",
    "--query-driver=**",
    "--compile-commands-dir=<абсолютний шлях до проєкту>\\build"
  ]
}
```

- `cmake.*: false` — вимикає CMake Tools, щоб він не конфігурував проєкт паралельно з ESP-IDF
- `idf.openOcdConfigs` — вбудований USB-JTAG ESP32-S3 для налагодження
- `--compile-commands-dir` — абсолютний шлях; при копіюванні шаблону його треба змінити

`.vscode/c_cpp_properties.json` — IntelliSense бере прапорці з `${config:idf.buildPath}/compile_commands.json`.

`.vscode/launch.json` — конфігурація `gdbtarget` / `attach` (Eclipse CDT GDB Adapter) для
налагодження через OpenOCD.

`.clangd` — `Remove: [-f*, -m*]` прибирає специфічні для Xtensa GCC прапорці, які clangd не розуміє.

## Записати лог

### Вимоги
- Python <3.x>
- pyserial 3.5

Задайте шлях до логу та порт у `src/save_serial_output.py`:

```python
LOG_PATH = Path(__file__).parent / ".." / "<logname>.log"  # Шлях до логу
SERIAL_PATH = "<COMxx>"  # Порт UART, має співпадати з idf.port
```

## Notes (Debriefing)

1. <Рішення, підводні камені>

## What I would add

## TODO
- [ ] Add hardware requirements
- [ ] Add circuit diagram
- [ ] Add code function comments
- [ ] Add debugging log
