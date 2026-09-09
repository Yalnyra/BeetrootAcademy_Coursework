# STM32F411CEU6 "Black Pill"

Projects for Cortex-M work: registers, HAL, GPIO, interrupts, timers, DMA.

- Core: ARM Cortex-M4F @ 96 MHz (25 MHz HSE, PLL /25 ×192 /2)
- Memory: 512 KB flash, 128 KB SRAM
- On-board LED: **PC13, active LOW**
- User button: **PA0** (to GND)
- Flashing: ST-Link V2 over SWD, or the on-board DFU bootloader
  (hold BOOT0, tap NRST) — see `upload_protocol` in each `platformio.ini`

Start a project with `cp -r ../_template/stm32f411ce ./NN_Name`.

| # | Project | Assignment | Status |
|---|---------|------------|--------|
| — | | | |
