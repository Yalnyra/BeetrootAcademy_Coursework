# <NN_ProjectName>

> EMB_25 Завдання <N.N>: <тема>
> Handout: [`assignments/EMB_25_HW<NN>.pdf`](../../assignments/EMB_25_HW<NN>.pdf)

### Схемотехніка

[Посилання на Діаграму](<https://app.cirkitdesigner.com/project/...>)

![Схема](output/<NN>_schema.png)

### Відео демонстрація
<video controls width="600">
  <source src="./output/Assignment_<N.N>.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

### Вимоги до програми (Код)

<Що вимагає завдання — один короткий абзац.>

- <вимога 1>
- <вимога 2>

## Набір деталей

| Деталь | Опис |
|------|--------|
| Плата | STM32F411CEU6 Black Pill (WeAct), роз'єм USB Type-C |
| Програматор | ST-Link V2 (STM8 & STM32), підключення по SWD |
| <Деталь> | <кількість, номінал> |
| Кабель | USB Type-C ↔ ПК (живлення плати) |
| Інше | Макетна плата, перемички, Dupont-дроти F-F для ST-Link |

Живлення плати — від USB Type-C. ST-Link підключений тільки для прошивки та налагодження:

| ST-Link V2 | Black Pill (SWD-роз'єм) |
|------|--------|
| SWDIO | SWDIO (PA13) |
| SWCLK | SWCLK (PA14) |
| GND | GND |
| 3.3V / 5.0V | **не підключати** — плата живиться від USB-C |

### Підключення

| Сигнал | Пін MCU | Пін периферії |
|--------|---------|----------------|
| LED (на платі) | PC13 (active-LOW) | — |
| <сигнал> | <Pxx> | <пін> |

### Вихід програми

```
<вивід у консоль / опис поведінки>
```

## Передумови (версії, з якими зібрано проєкт)

| Компонент | Версія / значення |
|------|--------|
| IDE | STM32CubeIDE <версія> |
| Компілятор | GNU Tools for STM32 **<14.3.rel1>** (`arm-none-eabi-gcc`) |
| STM32CubeMX | **<6.18.1>** — для відкриття та перегенерації `.ioc` |
| Firmware package | STM32Cube **FW_F4 <V1.28.3>** |
| MCU | STM32F411CEU6 (UFQFPN48), `STM32F411xE` |
| Бібліотеки | STM32 HAL (`USE_HAL_DRIVER`)<, Middleware> |
| Тактування | HSE 25 MHz → PLL → SYSCLK **96 MHz** |
| Піни | `LED` — PC13 (GPIO Output)<, інші піни з `.ioc`> |
| Налагоджувач | ST-Link V2, інтерфейс SWD, ST-LINK GDB server |
| Linker script | `STM32F411CEUX_FLASH.ld` |
| ОС | Windows 11 |

Версії взято з `<ProjectName>.ioc` (`MxCube.Version`, `ProjectManager.FirmwarePackage`), `.cproject` та `Debug/makefile`.

## Швидкий старт

0. Перевірити, який `main` збирається
   - У `Core/Src` компілюються всі `.c` файли — `main()` має бути рівно в одному з них.
   - Альтернативні варіанти `main` тримати з розширенням `.txt`, щоб вони не потрапляли в збірку.

1. Встановити інструменти
   - STM32CubeIDE (разом із GNU Tools for STM32)
   - STM32CubeMX і пакет STM32Cube FW_F4 — потрібні тільки для зміни `.ioc` та перегенерації коду
   - Драйвер ST-Link (встановлюється разом із STM32CubeIDE)

2. Імпортувати проєкт
   - `File → Import… → General → Existing Projects into Workspace → Next`
   - `Select root directory` → папка `<ProjectName>`
   - Галочку `Copy projects into workspace` **не ставити** — працюємо з файлами в репозиторії
   - `Finish`

3. Зібрати проєкт
   - Вибрати проєкт у `Project Explorer`, конфігурація `Debug`
   - `Project → Build Project` / `Ctrl+B` (іконка молотка)
   - Результат: `Debug/<ProjectName>.elf` (а також `.map` і `.list`)
   - Після зміни набору файлів: `Project → Clean…`, потім знову `Ctrl+B`

4. Підключити плату
   - USB Type-C → ПК (живлення плати)
   - ST-Link V2 → SWD-роз'єм плати: SWDIO, SWCLK, GND (див. таблицю в [Набір деталей](#набір-деталей)). Живлення 3.3V зі ST-Link не підключати
   - ST-Link V2 → USB ПК

5. Прошити та запустити налагодження
   - `Run → Debug Configurations… → STM32 C/C++ Application → <ProjectName>` (конфігурація зберігається у файлі `<ProjectName>.launch`)
   - Вкладка `Debugger`: `ST-LINK (ST-LINK GDB server)`, інтерфейс `SWD`
   - `Debug` (`F11`) — прошити й зупинитися на `main()`, `Resume` (`F8`) — запустити
   - `Run` (`Ctrl+F11`) — прошити без налагодження
   - Якщо IDE пропонує оновити прошивку ST-Link — погодитися

6. Перевірити роботу
   - <очікувана поведінка на платі>

### Зміна конфігурації (`.ioc`)

- Відкрити `<ProjectName>.ioc` у STM32CubeMX, внести зміни, `Generate Code`
- CubeMX перезаписує все поза блоками `/* USER CODE BEGIN … */ … /* USER CODE END … */` — власний код писати тільки всередині них
- Власну логіку тримати в окремих файлах (`app.c` / `app.h` з `setup()` і `loop()`), а з `main.c` лише викликати їх у блоках `USER CODE 2` та `USER CODE 3`

## Notes (Debriefing)

1. <Рішення, підводні камені, деталі на рівні регістрів>

## What I would add

## TODO
- [ ] Add hardware requirements
- [ ] Add circuit diagram
- [ ] Add code function comments
- [ ] Add debugging log
