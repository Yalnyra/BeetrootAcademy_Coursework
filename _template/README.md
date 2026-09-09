# Project templates

Copy one of these folders into the matching board folder to start a new
assignment — do not edit a project in place here.

| Template | Board | Framework |
|----------|-------|-----------|
| `stm32f411ce/` | STM32F411CEU6 "Black Pill" | `stm32cube` |
| `esp32s3/` | ESP32-S3-DevKitC-1 N16R8 | `arduino` (ESP-IDF env included, commented) |

```bash
cp -r _template/stm32f411ce stm32f411ce/03_TimerPWM
```

Each template builds and runs as-is: the Black Pill one blinks PC13 at 1 Hz,
the ESP32-S3 one blinks GPIO48 and prints a heartbeat plus flash/PSRAM/heap
figures over the native USB serial port. Verify the blink first, then replace
the logic — that way a build failure is your code, not the setup.

Neither template carries a `.gitignore`; the repository root has the shared one.
