# <NN_ProjectName>

> EMB_25 · Assignment <NN> — <topic>
> Handout: [`assignments/EMB_25_HW<NN>.pdf`](../../assignments/EMB_25_HW<NN>.pdf)

## Task

<What the assignment asks for, in one short paragraph.>

## Hardware

| Item | Detail |
|------|--------|
| Board | ESP32-S3-DevKitC-1 N16R8 (16 MB flash, 8 MB PSRAM) |
| Programmer | Native USB (CDC) |
| Extra parts | <sensors, resistors, wiring> |

### Wiring

| Signal | MCU pin | Peripheral pin |
|--------|---------|----------------|
|        |         |                |

## Build & flash

```bash
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial console @ 115200
```

## Notes

- <Design decisions, gotchas, register-level details worth remembering.>

## Status

- [ ] Builds clean
- [ ] Runs on hardware
- [ ] Submitted
