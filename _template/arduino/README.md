# <NN_ProjectName>

> EMB_25 Завдання <N.N>: <тема>
> Handout: [`assignments/EMB_25_HW<NN>.pdf`](../../assignments/EMB_25_HW<NN>.pdf)

### Схемотехніка

[Посилання на Діаграму](<https://app.cirkitdesigner.com/project/...>)

[Вивід в консоль](output/<N.N>_output.log)
```
<фрагмент виводу в консоль>
... (Цикл повторюється)
```

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_<N.N>.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)

**<Головна вимога завдання одним реченням.>**

<Деталі: що робить кожен елемент, з якими інтервалами тощо.>

**Обмеження**

❌ <що заборонено, напр. `delay()`>
✅ <що дозволено, напр. `millis()`>

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

<Опис поведінки на платі>

## Передумови

| Компонент | Версія / значення |
|------|--------|
| IDE | VS Code + розширення PlatformIO IDE |
| Platform | `espressif32` <версія> |
| Framework | `arduino` |
| Board | `esp32-s3-devkitc-1` (16 MB flash, OPI PSRAM: `memory_type = qio_opi`) |
| Serial | Native USB CDC (`ARDUINO_USB_CDC_ON_BOOT=1`) @ 115200 |
| Налагоджувач | `esp-builtin` (вбудований USB-JTAG) |
| ОС | Windows 11 |

Налаштування — у `platformio.ini`: заповнити `description`, перевірити `monitor_port`.

## Швидкий старт

```bash
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial console @ 115200
```

## Записати лог

### Вимоги
- Python <3.x>
- pyserial 3.5

Задайте шлях до логу та порт у `src/save_serial_output.py`:

```python
LOG_PATH = Path(__file__).parent / ".." / "output" / "<N.N>_output.log"  # Шлях до логу
SERIAL_PATH = "COM16"  # Має співпадати з platformio.ini: monitor_port
```

Перед запуском закрийте `pio device monitor` — порт може використовувати лише одна програма.

## Notes (Debriefing)

1. <Рішення, підводні камені>

## What I would add

## TODO
- [ ] Add hardware requirements
- [ ] Add circuit diagram
- [ ] Add code function comments
- [ ] Add debugging log
