# <2.8>

> EMB_25 : Додатковий урок: Реагування кнопки у STM32
>   Урок 30 Гнучкий слот. Поглиблене дослідження теми


>  [\assignments\2.8_STM32_Button_Handler\Core\Src\main_2_IQR_Button.c\*.c](.\assignments\2.6_ESPIDF_Debounce\main\lesson_2.6)


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/bd7b1519-784d-40f1-b32b-ad18fb8e5ca7)

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_2.8.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)

Використати STM32 HAL. додати обробку стану кнопки. Світлодіод на платі повинен реагувати на натискання кнопки. Реакція світлодіода довільна.

Код з уроку:
```c
   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

   HAL_Delay(500);
```

- Створити додаткові файли app.c i app.h і в них функції setup() i loop().
- Перенести бізнес логіку (blink) в ці функції за аналогією Ардуіно фреймворку.
- *Опційно додати механізм уникнення циклічного імпорту в app.h

## Набір деталей

| Деталь | Опис |
|------|--------|
| Плата | STM32F411CEU6 Black Pill (WeAct), роз'єм USB Type-C |
| Програматор | ST-Link V2 (STM8 & STM32), підключення по SWD |
| Кнопка | x1 тактова кнопка |
| Конденсатор | x1 100 nF (керамічний) — апаратне подавлення брязкоту кнопки |
| Резистори | x2 220 Ом |
| Світлодіоди | x1 Yellow |
| Кабель | USB Type-C |
| Інше | Макетна плата, перемички, Dupont-дроти Тато-Тато для ST-Link |

Живлення плати — від USB Type-C. ST-Link підключений тільки для прошивки та налагодження:

| ST-Link V2 | Black Pill (SWD-роз'єм) |
|------|--------|
| SWDIO | SWDIO (PA13) |
| SWCLK | SWCLK (PA14) |
| GND | GND |
| 3.3V / 5.0V | **не підключати** — плата живиться від USB-C |

### Вихід програми 


## Передумови (версії, з якими зібрано проєкт)

| Компонент | Версія / значення |
|------|--------|
| IDE | STM32CubeIDE |
| Компілятор | GNU Tools for STM32 **14.3.rel1** (`arm-none-eabi-gcc`) |
| STM32CubeMX | **6.18.1** (DB.6.0.181) — для відкриття та перегенерації `.ioc` |
| Firmware package | STM32Cube **FW_F4 V1.28.3** |
| Бібліотеки | STM32 HAL (`USE_HAL_DRIVER`), USB Device Library — CDC (Virtual COM Port) |
| Тактування | HSE 25 MHz → PLL → SYSCLK **96 MHz** |
| Піни | `LED` — PC13 (GPIO Output), `BTN` — PA1 (EXTI, falling edge, pull-up) |
| Налагоджувач | ST-Link V2, інтерфейс SWD, ST-LINK GDB server |
| Linker script | `STM32F411CEUX_FLASH.ld` |
| ОС | Windows 11 |

Версії взято з `2.8_STM32_Button_Handler.ioc`, `.cproject` та `Debug/makefile`.

## Швидкий старт

0. Перевірити, який `main` збирається
   - У `Core/Src` компілюються всі `.c` файли. Зараз активний `main_2_IQR_Button.c` (кнопка через переривання EXTI).
   - `main_1_Pollingbutton.txt` (Приклад з виводом лічильника в консоль) не компілюється. Щоб зібрати його, перейменуйте на `.c`, а `main_2_IQR_Button.c` - на `.txt`. Два файли з `main()` одночасно дадуть помилку лінкування.

1. Встановити інструменти
   - STM32CubeIDE (разом із GNU Tools for STM32 14.3.rel1)
   - STM32CubeMX 6.18.1 і пакет STM32Cube FW_F4 V1.28.3 — потрібні тільки для зміни `.ioc` та перегенерації коду
   - Драйвер ST-Link (встановлюється разом із STM32CubeIDE)

2. Імпортувати проєкт
   - `File → Import… → General → Existing Projects into Workspace → Next`
   - `Select root directory` → папка `2.8_STM32_Button_Handler`
   - Галочку `Copy projects into workspace` **не ставити** — працюємо з файлами в репозиторії
   - `Finish`

3. Зібрати проєкт
   - Вибрати проєкт у `Project Explorer`, конфігурація `Debug`
   - `Project → Build Project` / `Ctrl+B` (іконка молотка)
   - Результат: `Debug/2.8_STM32_Button_Handler.elf` (а також `.map` і `.list`)
   - Після зміни набору файлів: `Project → Clean…`, потім знову `Ctrl+B`

4. Підключити плату
   - USB Type-C → ПК (живлення плати + USB CDC Virtual COM Port)
   - ST-Link V2 → SWD-роз'єм плати: SWDIO, SWCLK, GND (див. таблицю в [Набір деталей](#набір-деталей)). Живлення 3.3V зі ST-Link не підключати
   - ST-Link V2 → USB ПК

5. Прошити та запустити налагодження
   - `Run → Debug Configurations… → STM32 C/C++ Application → 2.8_STM32_Button_Handler` (конфігурація зберігається у файлі `2.8_STM32_Button_Handler.launch`)
   - Вкладка `Debugger`: `ST-LINK (ST-LINK GDB server)`, інтерфейс `SWD`
   - `Run` (`Ctrl+F11`) — прошити без налагодження
   - `Debug` (`F11`) — прошити й зупинитися на `main()`, `Resume` (`F8`) — запустити

6. Перевірити роботу
   - Натиснути кнопку на PA1 — світлодіод PC13 реагує на натискання
   - У Диспетчері пристроїв Windows з'являється новий COM-порт (USB CDC)

### Зміна конфігурації (`.ioc`)

- Відкрити `2.8_STM32_Button_Handler.ioc` у STM32CubeMX, внести зміни, `Generate Code`
- CubeMX перезаписує все поза блоками `/* USER CODE BEGIN … */ … /* USER CODE END … */` — власний код писати тільки всередині них
- Після генерації CubeMX створить `Core/Src/main.c`. Перенесіть свої зміни з `main_2_IQR_Button.c` у нього (або видаліть згенерований `main.c`), щоб не було двох `main()`

## TODO
- [x] Add hardware requirements
- [x] Add IDE setup instructions
- [x] Add circuit diagram 
- [x] Add code function comments