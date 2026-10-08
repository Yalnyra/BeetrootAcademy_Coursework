# <2.2>

> EMB_25 Завдання 2.2: Активні та Пасивні елементи
> Завдання 1: Вимірювання часу спрацювання реле


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/cbfc5bbb-d66e-47b1-b398-5468194a6da2)

[Вивід в консоль](output/2.3_output.log)
```

 Total Loop time in Microseconds over 1000 iterations: 975 | 
 Timestamp: 29179463 | 
 State Slow 1 | 
 State Medium 0 | 
 State Fast 1 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 29379465 | 
 State Slow 0 | 
 State Medium 0 | 
 State Fast 1 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 29579472 | 
 State Slow 1 | 
 State Medium 1 | 
 State Fast 1 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 29779482 | 
 State Slow 0 | 
 State Medium 1 | 
 State Fast 1 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 29979486 | 
 State Slow 1 | 
 State Medium 1 | 
 State Fast 1 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 30179497 | 
 State Slow 0 | 
 State Medium 0 | 
 State Fast 0 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 30379503 | 
 State Slow 1 | 
 State Medium 0 | 
 State Fast 0 | 
 Total Loop time in Microseconds over 1000 iterations: 976 | 
 Timestamp: 30579506 | 
 State Slow 0 | 
 State Medium 1 | 
 State Fast 0 | 
... (Цикл повторюється)
```

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_2.3.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)


**Керувати кількома незалежними процесами в одному loop() без блокування.**

Є 3 світлодіоди:

LED1 блимає кожні 200 мс
LED2 блимає кожні 500 мс
LED3 блимає кожні 1000 мс

**Обмеження**

❌ Заборонено використовувати delay()
❌ Заборонено використовувати while() для очікування часу у loop() 
✅ Дозволено використовувати millis()

## Набір деталей

| Деталь | Опис |
|------|--------|
| Board | ESP32-S3 N16R8 (16 MB flash, 8 MB PSRAM) |
| Резистори | x3 220 Om |
| Світлодіоди | x3 Green |

### Вихід програми 

Вивід програми 


## Швидкий старт

```bash
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial console @ 115200
```

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

## Notes (Debriefing)

## What I would add

## TODO
- [x] Add hardware requirements
- [x] Add circuit diagram 
- [x] Add code function comments
- [x] Add debugging log (First no button activation at all, then wrong activation pattern due to pull-down misconfiguration)