# ESP32-S3-DevKitC-1 N16R8

Projects for peripherals and connectivity: Wi-Fi, GPIO, ADC, PWM (LEDC),
I²C, SPI, UART.

- Core: dual Xtensa LX7 @ 240 MHz
- Memory: 16 MB QIO flash, 8 MB **octal** PSRAM (`memory_type = qio_opi`)
- On-board LED: addressable RGB on **GPIO48**
- Serial: native USB CDC (`ARDUINO_USB_CDC_ON_BOOT=1`), 115200 baud
- Two USB ports — **USB** is the native one, **UART** goes through the
  USB-serial bridge. If uploads fail, hold BOOT and tap RESET.

Start a project with `cp -r ../_template/esp32s3 ./NN_Name`.

Wi-Fi credentials belong in a `secrets.h` you keep out of git, not in `main.cpp`.

| # | Project | Assignment | Status |
|---|---------|------------|--------|
| — | | | |
