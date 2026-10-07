# <1.7>

> EMB_25 Мініпроєкт <1> підключення сенсора і актуатора, відладчик
> Мініпроєкт: Сутінковий перемикач на ESP32-S3


## Video Demo
TODO
<!-- 
<video controls width="600">
  <source src="./docs/Assignment_1.4.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video> -->

### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/93914e23-96de-43bc-9be2-04c2cfafc54f)

### Вимоги до програми (Код)

**1. Переписати “класичний Arduino blink” у Embedded C++**

- Використати enum class для стану LED (On / Off).
- Використати constexpr для номера піну та інтервалу блимання.
- Уникнути глобальних змінних (крім мінімально необхідних для стану/ISR).
- Не використовувати delay(); реалізувати неблокуючий код у superloop за допомогою millis().
- Створити клас Led з методами init() та set(LedState state).

**2. Використати static const / constexpr для параметрів**

- Створити клас-конфігурацію або структуру з static const / constexpr для додаткових налаштувань.
- Повністю усунути «магічні числа», винісши їх у константи.

Measure average cycle runtime for a LED blink, using superloop architecture 

**3. Виміряти час виконання superloop (Опційно)**
- Виміряти час однієї ітерації loop() без delay().
- Виводити середній час або показники у Serial Monitor кожні 1000 ітерацій.


## Набір деталей

| Деталь | Опис |
|------|--------|
| Board | ESP32-S3 N16R8 (16 MB flash, 8 MB PSRAM) |
| Резистори | x1 220 Om |
| Світлодіоди | x1 LED (3V) | 

### Вихід програми 

Середній час loop(), з керування піном за допомогою constexpr - 4294967 ms

```
 | 
17:25:06.901 >  Timestamp: 15010 | 
17:25:06.901 >  LED pin: 6 | 
17:25:06.901 >  State 1 | 
17:25:06.901 >  Avg Loop time from 1000 iterations: 4294967 | 
17:25:07.903 >  Timestamp: 16011 | 
17:25:07.903 >  LED pin: 6 | 
17:25:07.903 >  State 0 | 
17:25:07.903 >  Avg Loop time from 1000 iterations: 4294967 | 
17:25:08.904 >  Timestamp: 17011 | 
17:25:08.904 >  LED pin: 6 | 
17:25:08.904 >  State 0 | 
17:25:08.904 >  Avg Loop time from 1000 iterations: 0 | 
17:25:09.904 >  Timestamp: 18012 | 
17:25:09.904 >  LED pin: 6 | 
17:25:09.904 >  State 1 | 
17:25:09.904 >  Avg Loop time from 1000 iterations: 0 | 
17:25:10.905 >  Timestamp: 19013 | 
17:25:10.905 >  LED pin: 6 | 
17:25:10.905 >  State 1 | 
17:25:10.905 >  Avg Loop time from 1000 iterations: 4294967 |


## Швидкий старт

```bash
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial console @ 115200
```

<!-- ## Згенерувати плоти 

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

### Візуалізувати вхід з ADC/ Вихід в Реле

src\save_serial_output.py

Задайте шлях до логів
```bash
LOG_PATH = Path(__file__).parent / ".." / "dummy.log" # Шлях до логу
SORTED_LOG_PATH = Path(__file__).parent / ".." / "sorted_dummy.log" # Лог з посортованою часовою шкалою (За Timestamp)
FIGURE_PATH = Path(__file__).parent / ".." / "adc_error_vs_voltage.png" # 3 Графіки - зберігаються у docs\plots\***\adc_error_vs_voltage.png
# Тип графіків (по вертикалі донизу)
#1 - Чистий вхід ADC / #2 - фільтрований EMA вхід ADC, / #3 Вихід на керування реле 
# Горизонтальним пунктиром обозначені ліміти THRESHOLD_DARK (Поріг ввимкнення), THRESHOLD_LIGHT - (Поріг ввімкнення)
``` -->

## Notes (Debriefing)

## What I would add

## TODO
- [x] Add hardware requirements
- [x] Add circuit diagram 
- [x] Add code function comments
- [x] Add debugging log (First no button activation at all, then wrong activation pattern due to pull-down misconfiguration)
