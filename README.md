# BeetrootAcademy_Coursework

**EMB_25 Assignments & Mini-Projects**

Coursework for the Beetroot Academy synchronous online embedded course (EMB_25).
Every assignment lives in its own self-contained PlatformIO project; the
homework handouts that go with them are kept together under `assignments/`.

## Layout

```
.
├── assignments/          Homework handouts (PDF), one per lesson
├── _template/            Copy one of these to start a new assignment
│   ├── stm32f411ce/      STM32Cube HAL, Black Pill
│   └── esp32s3/          Arduino / ESP-IDF, ESP32-S3-N16R8
├── stm32f411ce/          Projects targeting the Black Pill
├── esp32s3/              Projects targeting the ESP32-S3
├── stm32f411re/          Earlier work on the F411RE board
├── .gitignore            Shared by every project — projects carry no own copy
└── .gitattributes
```

Each project folder is a standard PlatformIO project:

```
NN_ProjectName/
├── README.md         What the assignment asks, wiring, notes, status
├── platformio.ini    Board, framework, upload protocol, build flags
├── include/          Public headers
├── lib/              Project-local libraries
├── src/              Sources (main.c / main.cpp)
└── test/             Unit tests
```

## Boards

| Board | Folder | Framework | Used for |
|-------|--------|-----------|----------|
| STM32F411CEU6 "Black Pill" | `stm32f411ce/` | `stm32cube` | Cortex-M programming, registers, HAL, interrupts, timers |
| ESP32-S3-DevKitC-1 N16R8 | `esp32s3/` | `arduino` (or `espidf`) | Wi-Fi, GPIO, ADC, PWM, I²C, SPI, UART |

## Starting a new assignment

```bash
# Black Pill
cp -r _template/stm32f411ce stm32f411ce/03_TimerPWM

# ESP32-S3
cp -r _template/esp32s3 esp32s3/04_I2C_Sensor
```

PowerShell:

```powershell
Copy-Item -Recurse _template\stm32f411ce stm32f411ce\03_TimerPWM
```

Then:

1. Open the new folder in VS Code (**PlatformIO → Open Project**) — PlatformIO
   treats each project folder as its own workspace.
2. Fill in `platformio.ini` → `description`.
3. Rewrite `README.md`: title, assignment number, link to the handout PDF.
4. Write the code, build with `pio run`, flash with `pio run -t upload`.

## Naming

- Projects: `NN_PascalCase` — `01_Blink`, `02_UartEcho`, `07_SpiDisplay`
- Handouts: `assignments/EMB_25_HW<NN>_<topic>.pdf`

## Toolchain

- [PlatformIO Core](https://platformio.org/install/cli) (or the VS Code extension)
- `platform = ststm32` pulls the ARM GCC toolchain and STM32Cube HAL automatically
- `platform = espressif32` pulls the Xtensa toolchain and Arduino/ESP-IDF automatically
- ST-Link V2 drivers for Black Pill flashing over SWD

## Index

| # | Assignment | Board | Project | Handout |
|---|------------|-------|---------|---------|
| — | _first assignment goes here_ | | | |
