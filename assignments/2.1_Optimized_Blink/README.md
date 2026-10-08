# <2.2>

> EMB_25 Завдання 2.2: Активні та Пасивні елементи
> Завдання 1: Вимірювання часу спрацювання реле


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/e458f877-0795-4711-978f-bc68a347a704)

![Electrical Schema of mini-project](output/Schema_2.2_Relay_Interrupt_Check.png)

[Вивід в консоль](output/2.2_output.log)
```
Цикл 2. Час OFF: 3677 мкс
Цикл 3. Час ON:  4517 мкс
Цикл 3. Час OFF: 3677 мкс
Цикл 4. Час ON:  4516 мкс
Цикл 4. Час OFF: 3675 мкс
Цикл 5. Час ON:  4521 мкс
Цикл 5. Час OFF: 3674 мкс
Цикл 6. Час ON:  4536 мкс
Цикл 6. Час OFF: 3671 мкс
Цикл 7. Час ON:  4526 мкс
Цикл 7. Час OFF: 3670 мкс
Цикл 8. Час ON:  4521 мкс
Цикл 8. Час OFF: 3669 мкс
Цикл 9. Час ON:  4541 мкс
Цикл 9. Час OFF: 3666 мкс
Цикл 10. Час ON:  4542 мкс
Цикл 10. Час OFF: 3665 мкс

====== ПІДСУМКИ ======

Середній час ON:  4562 мкс
Середній час OFF: 3672 мкс
======================
```

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_2.2.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

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