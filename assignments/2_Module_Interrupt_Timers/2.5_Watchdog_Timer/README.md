# <2.5>

> EMB_25 Мініпроєкт 2.7: Активні та Пасивні елементи
>   Урок 26 Таймер для витяжки 


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/e458f877-0795-4711-978f-bc68a347a704)

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
  <source src="./output/Assignment_2.5.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)


**Реалізувати керування витяжним вентилятором, який кожну годину вмикається на 15 хвилин, після чого автоматично вимикається**

Вентилятор = двигун || реле
Таймери мають працювати незалежно від коду в циклі
Не використовувати delay() для реалізації завдання
Цикл повторюється без участі loop() / task
Час роботи і період задаються змінними або макросами
Для налагодження можна:

замінити 1 годину на кілька секунд
15 хв замінити на пропорційно менший відрізок часу
## Набір деталей



| Деталь | Опис |
|------|--------|
| Board | ESP32-S3 N16R8 (16 MB flash, 8 MB PSRAM) |
| Резистори | x3 10k Om |
| 5V Реле x1 | jqc3f-05vdc-c |
| NPN Транзистор | 2Q 2N2222  |
| 5В Плата живлення на бредборді | x1 | 


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