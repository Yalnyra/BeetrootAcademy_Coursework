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
├── stm32f411ce/          (Currently unused) Projects targeting the Black Pill
├── esp32s3/              (Currently unused) Projects targeting the ESP32-S3
├── stm32f411re/          Earlier work on the STM32-F411RE board
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

## Projects 

# Mini project #1 (Assignment): Twilight light switch 

## Link to the project 

Код знаходиться за посиланням та опис

` .\assignments\1.7_Photoresistor_Mini_Project\src`

Опис реалізації проєкту знаходиться за посиланням: 

` .\assignments\1.7_Photoresistor_Mini_Project\README.md`


## Schema. 

Platform: ESP32-S3

[Electric Diagram link](https://app.cirkitdesigner.com/project/93914e23-96de-43bc-9be2-04c2cfafc54f)

![Electrical Schema of mini-project](docs/Assignment_1.7_schema.png)

Зберіть схему на макетній платі згідно з наданим кресленням:

Вхід (Фоторезистор): Підключіть дільник R1(LDR) + R2 (10 кОм) до шини +3.3V. Точку з'єднання підключіть до GPIO In (ADC).

Ключ (Узгодження 3.3V -> 5V): Підключіть GPIO Out через R3 (10 кОм) до бази транзистора BC547B (VT1). Колектор підтягніть до +5V через R4(10 кОм) та підключіть до входу IN модуля реле.

Вихід (Навантаження): До контактів реле OUT NO підключіть будь-яке безпечне низьковольтне навантаження (світлодіод, LED-стрічку 5V/12V).

### Software requirements
Напишіть просту програму, яка виконує 3 кроки у головному циклі:

Зчитати значення ADC з піна GPIO IN (діапазон 0...4095).

Порівняти значення з порогами (Гістерезис):

Якщо ADC < THRESHOLD_DARK (темно) -> подати HIGH на GPIO Out (увімкнути реле).

Якщо ADC > THRESHOLD_LIGHT (світло) -> подати LOW на GPIO Out (вимкнути реле).

Якщо значення між порогами -> нічого не змінювати (це захищає реле від брязкоту).


# Templates 

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
