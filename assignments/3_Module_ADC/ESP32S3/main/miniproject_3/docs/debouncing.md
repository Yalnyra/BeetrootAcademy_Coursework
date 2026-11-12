## Журнал налагодження

### 1. Runaway кута сервоприводу (06.11)

<!-- Симптом → Як відтворити → Доказ → Причина → Виправлення (commit 084d557) -->

### 2. Лічильник рахує лише вгору (08.11)

<!-- Симптом → Як відтворити → Доказ → Причина → Виправлення (commit 9814e05) -->

### 3. Дребезг енкодера та RC-фільтр (09.11)

#### Методика

Схема під'єднання: [TODO]()


! Дякую за візуалізацію порталу [Falstad](https://falstad.com/circuit/)

![Схема підключення RC фільтра](rc_and_switch.png)

0. Програма, яку треба виконати на ESP-32s3

```cmakelists

idf_component_register(SRCS "miniproject_3/src/main.c"
                          REQUIRES esp_adc esp_driver_gpio esp_timer esp_driver_ledc log esp_driver_pcnt freertos)

# Miniproject 3 encoder decoding mode (compile-time flag ENCODER_QUADRATURE_X4):
#   idf.py -DENCODER_X4=ON  build   -> X4 quadrature (2 PCNT channels, 4 counts per click)
#   idf.py -DENCODER_X4=OFF build   -> X1 (lesson 3.6, default)
# The value is stored in the CMake cache, so it stays until you set it again.
option(ENCODER_X4 "Miniproject 3: X4 quadrature encoder decoding instead of X1" OFF)
if(ENCODER_X4)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ENCODER_QUADRATURE_X4=1)
endif()

```

[Посилання на код](..\src\main.c)

Комміт - 2ae0ea4f3fa0c35af9baf969eb23216129efc112


1. Як відбувається запис 

Спочатку за годинниковою стрілкою 2-3 оберти, потім проти годинникової.

Перший 10-20 обертів: Дуже повільно, По черзі.

Решта обертів: Прокручування на 90 градусів дуже швидко


First half is scrolling encoder wheel fast, first in cw, then in ccw direction, second half is rotating one step at a time in ccw directio


2. Вибір проміжку пропуску частот для Low-pass filter  

За простотою виконання (без катушки індуктивності) було обрано пару Resistor + Capacitor 

#### Результати

| Video title  | Logic Analyzer log           | C, uF | R, Ohm | Longest bounce, ms (Ch A) | Max bounce frequency, kHZ (Avg frequency kHZ) | Maximum ticks per pcnt cycle |
|--------------|------------------------------|-------|--------|---------------------------|-----------------------------------------------|------------------------------|
|              | 09.11_0_filter_              | 0     | 0      | 13.08                     | 510 (62)                                      | 7                            |
|              | 09.11 cwccw encodeerbaseline | 0     | 0      | 3.756                     | 6000 (5.44)                                   | 2                            |
|              | 09.11_encoder600om_rc_filter | 0.47  | 600    | 6.69                      | 12000 (73.02)                                 | 2                            |
|              | 104_capacitor_x4             | 0.1   | 0      | 0.25                      | 20 (8.017)                                    | 4                            |
|              | 10kohm_rc                    | 0.47  | 10000  | 5.1                       | 12000 (128.521)                               | 10-15                        |
|              | 720ohm_rc                    | 0.47  | 720    | 13.11                     | 12000 (212)                                   | 7                            |
|              | 1220ohm_rc                   | 0.47  | 1200   | 5.85                      | 12000 (141)                                   | 7                            |
|              | 2000ohm_rc                   | 0.47  | 2000   | 7.5                       | 12000 (145)                                   | 7                            |
|              | 1k_100nF_rc                  | 0.1   | 10000  | 5.1                       | 12000(128)                                    | 10-15                        |
| New encoder, |                              | 0.1   | 0      | 0.64                      | 12000(232)                                    | 2                        

 Оригінальна таблиця - [09.11 cwccw encodeerbaseline](../../../output/docs/Miniproject3_debounce)

##### R = 0 Ω, C = 0 F

[0ohm_0capacitor](../../../output/videos/0ohm_0capacitor.mp4)
- Saleae Logic 2: [09.11 cwccw encodeerbaseline](../../../output/09.11_encoder_baseline.sal)

Примітка: циклічний шум на відео - шум частотою 12Mhz, від живлення 3.3V та прибирається glitch filter 

Поворот в режимі X2: рахуються лише з зростаючого фронту - код не помічає обертів проти годинникової стрілки, бо зі сторони спадаючих фронтів різниця між CLK та DT фронтами занадто мала для коду, або прихована за брязкітом.

Кожен поворот рахується лише один раз, як видно в лозі `"COUNT INCREASED BY 1"`

##### C = 0.1 µF (104), без резистора

- Відео: [410_capacitor_2x_encoding_12_03.mp4](../../../output/videos/410_capacitor_2x_encoding_12_03.mp4)
- Відео: [410_capacitor_4x_encoding.mp4](../../../output/videos/410_capacitor_4x_encoding.mp4)
- Saleae Logic 2: [104_capacitor_x4.sal](../../../output/104_capacitor_x4.sal)

В режимі X2 нема повторів, і довжина брязкоту в перехідному  періоді мінімальна - 0.64 мс
Мінуси - енкодер менш чутливий до швидкого прокручування і пропускає деякі оберти
Навантаженням в цій схемі виступає лише pull-up резистор на стороні енкодера 


##### R = 720 Ω, C = 0.47 µF

- Відео: [720_Ohm_44m.mp4](../../../output/videos/720_Ohm_44m.mp4)
- Saleae Logic 2: [720ohm_rc.sal](../../../output/720ohm_rc.sal)

Часті пересчитування - - `Count increased by 1` повторюється по декілька разів за один тік. 
Тобто, хоча шумів і немає - з'явився небажаний шум, високочастотний (12Mhz) між низьким і високим положенням напруги
Рахується в режимі - X4 
Тобто і передній, і задній фронт рахуються як 2 оберти
<!-- Опис запису: що відбувається на відео, на якій секунді -->

##### R = 1220 Ω, C = 0.47 µF

- Відео: [1220_Ohm_47.mp4](../../../output/videos/1220_Ohm_47.mp4)
- Saleae Logic 2: [1220ohm_rc.sal](../../../output/1220ohm_rc.sal)


<!-- Опис запису: що відбувається на відео, на якій секунді -->

##### R = 2000 Ω, C = 0.47 µF

- Відео: [2000_Ohm_55.mp4](../../../output/videos/2000_Ohm_55.mp4)
- Saleae Logic 2: [2000ohm_rc.sal](../../../output/2000ohm_rc.sal)

<!-- Опис запису: що відбувається на відео, на якій секунді -->

##### R = 10 kΩ, C = 0.47 µF

- Відео: [10kohmrc_miniproject3.mp4](../../../output/videos/10kohmrc_miniproject3.mp4)
- Відео: [10kohmrc_miniproject3_2.mp4](../../../output/videos/10kohmrc_miniproject3_2.mp4)
- Відео: [10kohmrc_lesson3.6_4x.mp4](../../../output/videos/10kohmrc_lesson3.6_4x.mp4)
- Saleae Logic 2: [10kohm_rc.sal](../../../output/10kohm_rc.sal)

<!-- Опис запису: що відбувається на відео, на якій секунді -->


output\videos\Miniproject3_output.mp4
##### Кінцевий результат - Енкодер з коробки +  C = 0.1 µF (104), без резистора

- Відео: [410_capacitor_2x_encoding_12_03.mp4](../../../output/videos/410_capacitor_2x_encoding_12_03.mp4)
- Saleae Logic 2: [new_encoder_2x_104_capacitor](../../../output/new_encoder_2x_104_capacitor.sal)

**Лог програми з лише з capacitor фільтром,  ШВИДКЕ ОБЕРТАННЯ**

```

I (30125) mini_project_3: status update error, at 0 deg
I (30275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-10.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=-10.00 deg (0.00%)
I (30275) mini_project_3: SERVO_DUTY=205 ticks, SERVO_PULSE=0.500 ms, SPEED_K=2
W (30675) mini_project_3: COUNT INCREASED BY 1
I (30675) mini_project_3: status update error, at 0 deg
W (30725) mini_project_3: COUNT INCREASED BY 2
W (30825) mini_project_3: COUNT INCREASED BY 2
W (30875) mini_project_3: COUNT INCREASED BY 2
W (30975) mini_project_3: COUNT INCREASED BY 1
W (31325) mini_project_3: COUNT INCREASED BY 1
W (31375) mini_project_3: COUNT INCREASED BY 3
W (31475) mini_project_3: COUNT INCREASED BY 2
I (32275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=120.00 deg, SERVO_ANGLE=120/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (32275) mini_project_3: SERVO_DUTY=751 ticks, SERVO_PULSE=1.833 ms, SPEED_K=2
I (34275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=120.00 deg, SERVO_ANGLE=120/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (34275) mini_project_3: SERVO_DUTY=751 ticks, SERVO_PULSE=1.833 ms, SPEED_K=2
W (35025) mini_project_3: COUNT INCREASED BY 1
W (35525) mini_project_3: COUNT INCREASED BY 1
W (35825) mini_project_3: COUNT INCREASED BY 1
I (36275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=150.00 deg, SERVO_ANGLE=150/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (36275) mini_project_3: SERVO_DUTY=887 ticks, SERVO_PULSE=2.166 ms, SPEED_K=2
E (37725) mini_project_3: COUNT DECREASED BY -1
E (38125) mini_project_3: COUNT DECREASED BY -1
I (38275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=130.00 deg, SERVO_ANGLE=130/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (38275) mini_project_3: SERVO_DUTY=796 ticks, SERVO_PULSE=1.943 ms, SPEED_K=2
E (38475) mini_project_3: COUNT DECREASED BY -1
I (40275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=120.00 deg, SERVO_ANGLE=120/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (40275) mini_project_3: SERVO_DUTY=751 ticks, SERVO_PULSE=1.833 ms, SPEED_K=2
E (41225) mini_project_3: COUNT DECREASED BY -1
I (42275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=110.00 deg, SERVO_ANGLE=110/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (42275) mini_project_3: SERVO_DUTY=705 ticks, SERVO_PULSE=1.721 ms, SPEED_K=2
E (43775) mini_project_3: COUNT DECREASED BY -1
E (43825) mini_project_3: COUNT DECREASED BY -1
E (43925) mini_project_3: COUNT DECREASED BY -1
E (44025) mini_project_3: COUNT DECREASED BY -1
E (44075) mini_project_3: COUNT DECREASED BY -1
E (44125) mini_project_3: COUNT DECREASED BY -1
I (44275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=50.00 deg, SERVO_ANGLE=50/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (44275) mini_project_3: SERVO_DUTY=432 ticks, SERVO_PULSE=1.055 ms, SPEED_K=2
I (46275) mini_project_3
```

**RC-фільтр 720 Ом, (X2), №1 - ПОВІЛЬНЕ ОБЕРТАННЯ**

Немає false positives, кожен поворот в правильну сторону

```
I (24275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (24275) mini_project_3: SERVO_DUTY=0 ticks, SERVO_PULSE=0.000 ms, SPEED_K=2
E (25675) mini_project_3: COUNT DECREASED BY -1
E (25775) mini_project_3: COUNT DECREASED BY -1
E (25825) mini_project_3: COUNT DECREASED BY -1
E (25875) mini_project_3: COUNT DECREASED BY -1
E (25925) mini_project_3: COUNT DECREASED BY -1
E (25975) mini_project_3: COUNT DECREASED BY -1
I (26275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-60.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=-60.00 deg (0.00%)
I (26275) mini_project_3: SERVO_DUTY=205 ticks, SERVO_PULSE=0.500 ms, SPEED_K=2
W (26575) mini_project_3: COUNT INCREASED BY 1
W (26775) mini_project_3: COUNT INCREASED BY 1
W (26825) mini_project_3: COUNT INCREASED BY 1
W (27075) mini_project_3: COUNT INCREASED BY 1
W (27375) mini_project_3: COUNT INCREASED BY 1
W (27675) mini_project_3: COUNT INCREASED BY 1
I (27675) mini_project_3: status update error, at 0 deg
I (28275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-10.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=-10.00 deg (0.00%)
I (28275) mini_project_3: SERVO_DUTY=205 ticks, SERVO_PULSE=0.500 ms, SPEED_K=2
E (28325) mini_project_3: COUNT DECREASED BY -1
E (28425) mini_project_3: COUNT DECREASED BY -1
E (28575) mini_project_3: COUNT DECREASED BY -1
E (28775) mini_project_3: COUNT DECREASED BY -1
W (30175) mini_project_3: COUNT INCREASED BY 1
W (30225) mini_project_3: COUNT INCREASED BY 1
I (30275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-30.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=-30.00 deg (0.00%)
```



**RC-фільтр 720 Ом, (X2), №2 - ШВИДКЕ ОБЕРТАННЯ**

Частіше оберт зараховується не в ту сторону, а кількість обертів за один цикл збільшилось в 3-4x рази

```

I (48295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=180.00 deg, SERVO_ANGLE=180/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (48295) mini_project_3: SERVO_DUTY=1024 ticks, SERVO_PULSE=2.500 ms, SPEED_K=2
W (48795) mini_project_3: COUNT INCREASED BY 1
W (48845) mini_project_3: COUNT INCREASED BY 3
E (49145) mini_project_3: COUNT DECREASED BY -1
W (49195) mini_project_3: COUNT INCREASED BY 4
W (49245) mini_project_3: COUNT INCREASED BY 4
W (49495) mini_project_3: COUNT INCREASED BY 1
W (49545) mini_project_3: COUNT INCREASED BY 4
W (49595) mini_project_3: COUNT INCREASED BY 1
E (49895) mini_project_3: COUNT DECREASED BY -1
E (49945) mini_project_3: COUNT DECREASED BY -1
E (49995) mini_project_3: COUNT DECREASED BY -2
I (50295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=310.00 deg, SERVO_ANGLE=180/180 deg, ANGLE_ERROR=130.00 deg (72.22%)
I (50295) mini_project_3: SERVO_DUTY=1024 ticks, SERVO_PULSE=2.500 ms, SPEED_K=2
E (50345) mini_project_3: COUNT DECREASED BY -2
W (50395) mini_project_3: COUNT INCREASED BY 2
W (50445) mini_project_3: COUNT INCREASED BY 1
W (50745) mini_project_3: COUNT INCREASED BY 4
W (50795) mini_project_3: COUNT INCREASED BY 2
W (51095) mini_project_3: COUNT INCREASED BY 4
W (51145) mini_project_3: COUNT INCREASED BY 2
W (51445) mini_project_3: COUNT INCREASED BY 1
W (51495) mini_project_3: COUNT INCREASED BY 4
W (51795) mini_project_3: COUNT INCREASED BY 1
W (51845) mini_project_3: COUNT INCREASED BY 4
W (52145) mini_project_3: COUNT INCREASED BY 4
W (52195) mini_project_3: COUNT INCREASED BY 1
I (52295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=590.00 deg, SERVO_ANGLE=180/180 deg, ANGLE_ERROR=410.00 deg (227.78%)
I (52295) mini_project_3: SERVO_DUTY=1024 ticks, SERVO_PULSE=2.500 ms, SPEED_K=2
E (52495) mini_project_3: COUNT DECREASED BY -1
W (52545) mini_project_3: COUNT INCREASED BY 3
W (52595) mini_project_3: COUNT INCREASED BY 2
W (52845) mini_project_3: COUNT INCREASED BY 5
W (52895) mini_project_3: COUNT INCREASED BY 1
W (53145) mini_project_3: COUNT INCREASED BY 4
E (53195) mini_project_3: COUNT DECREASED BY -1
I (54295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=720.00 deg, SERVO_ANGLE=180/180 deg, ANGLE_ERROR=540.00 deg (300.00%)
I (54295) mini_project_3: SERVO_DUTY=1024 ticks, SERVO_PULSE=2.500 ms, SPEED_K=2
```

**Квадратурне декодування (X4), лог:**

```

I (100345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-20.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-30.00 deg (-300.00%)
I (100345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
I (102345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-20.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-30.00 deg (-300.00%)
I (102345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (102595) mini_project_3: COUNT INCREASED BY 1
W (102645) mini_project_3: COUNT INCREASED BY 1
W (102695) mini_project_3: COUNT INCREASED BY 2
W (103245) mini_project_3: COUNT INCREASED BY 1
W (103295) mini_project_3: COUNT INCREASED BY 3
W (103845) mini_project_3: COUNT INCREASED BY 2
W (103895) mini_project_3: COUNT INCREASED BY 2
I (103895) mini_project_3: status update error, at 10 deg
I (104345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (104345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (104495) mini_project_3: COUNT INCREASED BY 2
W (104545) mini_project_3: COUNT INCREASED BY 2
I (104545) mini_project_3: status update error, at 10 deg
W (104945) mini_project_3: COUNT INCREASED BY 1
W (104995) mini_project_3: COUNT INCREASED BY 3
I (104995) mini_project_3: status update error, at 10 deg
W (105145) mini_project_3: COUNT INCREASED BY 2
W (105195) mini_project_3: COUNT INCREASED BY 2
I (105195) mini_project_3: status update error, at 10 deg
W (105245) mini_project_3: COUNT INCREASED BY 2
W (105295) mini_project_3: COUNT INCREASED BY 2
I (105295) mini_project_3: status update error, at 10 deg
W (105395) mini_project_3: COUNT INCREASED BY 2
W (105945) mini_project_3: COUNT INCREASED BY 2
I (105945) mini_project_3: status update error, at 10 deg
W (106145) mini_project_3: COUNT INCREASED BY 1
W (106195) mini_project_3: COUNT INCREASED BY 1
W (106245) mini_project_3: COUNT INCREASED BY 2
I (106245) mini_project_3: status update error, at 10 deg
I (106345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (106345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (106445) mini_project_3: COUNT INCREASED BY 1
W (106495) mini_project_3: COUNT INCREASED BY 2
W (106545) mini_project_3: COUNT INCREASED BY 1
I (106545) mini_project_3: status update error, at 10 deg
W (106995) mini_project_3: COUNT INCREASED BY 1
W (107045) mini_project_3: COUNT INCREASED BY 2
W (107095) mini_project_3: COUNT INCREASED BY 1
I (107095) mini_project_3: status update error, at 10 deg
W (107195) mini_project_3: COUNT INCREASED BY 2
W (107245) mini_project_3: COUNT INCREASED BY 2
I (107245) mini_project_3: status update error, at 10 deg
W (107395) mini_project_3: COUNT INCREASED BY 2
W (107445) mini_project_3: COUNT INCREASED BY 2
I (107445) mini_project_3: status update error, at 10 deg
W (107545) mini_project_3: COUNT INCREASED BY 2
W (107595) mini_project_3: COUNT INCREASED BY 2
I (107595) mini_project_3: status update error, at 10 deg
W (108195) mini_project_3: COUNT INCREASED BY 1
W (108245) mini_project_3: COUNT INCREASED BY 3
I (108245) mini_project_3: status update error, at 10 deg
W (108345) mini_project_3: COUNT INCREASED BY 2
I (108345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (108345) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (108405) mini_project_3: COUNT INCREASED BY 2
I (108405) mini_project_3: status update error, at 10 deg
W (108655) mini_project_3: COUNT INCREASED BY 1
W (108955) mini_project_3: COUNT INCREASED BY 1
W (109005) mini_project_3: COUNT INCREASED BY 2
I (109005) mini_project_3: status update error, at 10 deg
W (109855) mini_project_3: COUNT INCREASED BY 1
W (109905) mini_project_3: COUNT INCREASED BY 3
I (109905) mini_project_3: status update error, at 10 deg
I (110355) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (110355) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (110655) mini_project_3: COUNT INCREASED BY 2
W (110705) mini_project_3: COUNT INCREASED BY 2
I (110705) mini_project_3: status update error, at 10 deg
W (111805) mini_project_3: COUNT INCREASED BY 2
W (111855) mini_project_3: COUNT INCREASED BY 2
I (111855) mini_project_3: status update error, at 10 deg
W (112155) mini_project_3: COUNT INCREASED BY 2
W (112205) mini_project_3: COUNT INCREASED BY 2
I (112205) mini_project_3: status update error, at 10 deg
I (112355) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-10.00 deg (-100.00%)
I (112355) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
W (112405) mini_project_3: COUNT INCREASED BY 2
W (112455) mini_project_3: COUNT INCREASED BY 2
I (112455) mini_project_3: status update error, at 10 deg
W (113105) mini_project_3: COUNT INCREASED BY 2
W (113155) mini_project_3: COUNT INCREASED BY 2
I (113155) mini_project_3: status update error, at 10 deg
W (113405) mini_project_3: COUNT INCREASED BY 4
I (113405) mini_project_3: status update error, at 10 deg
W (113505) mini_project_3: COUNT INCREASED BY 4
I (113505) mini_project_3: status update error, at 10 deg
W (113555) mini_project_3: COUNT INCREASED BY 5
I (113555) mini_project_3: status update error, at 10 deg
W (113605) mini_project_3: COUNT INCREASED BY 3
I (113605) mini_project_3: status update error, at 10 deg
W (114005) mini_project_3: COUNT INCREASED BY 5
I (114005) mini_project_3: status update error, at 10 deg
W (114055) mini_project_3: COUNT INCREASED BY 5
I (114055) mini_project_3: status update error, at 10 deg
```

#### Спостереження

- З огляду на середнє значення (в дужках) в таблиці **частота брязкоту** повністю випадкова. 

- Довжина брязкоту неперервно зростає зі збільшенням опору (0.64 мс на схемі з конденсатором, 3.7 мс при прямому підключенні, проти 7.5мс на 2000 Ом) - функція переходу RC фільтра більш плавно зростає та спалає

- X4 Модуляція більш чутлива до зміни положення при великих шумах на переході - але при цьому рахує по 7-10 за один перехід в залежності від довжини переходу до HIGH стану

 З огляду на відео 104_capacitor_2x_encoding_12_03.mp4 проти 410_capacitor_2x_encoding_12_03.mp4 проти 104_capacitor_4x_encoding.mp4

#### Інтерпретація

Гіпотеза в тому, що час брязкоту коррелює із часом встановлення із LOW стану напруги до HIGH. Так як він напряму підключений до цифрового GPIO входу, то 

Для перевірки швидкості зростання аналогових в hardware, а не симуляації потрібен цифровий осциллограф, а також TODO - додати до схеми Тріггер Шмітта (є в наявності) і повторити експеримент на 720 Ом та конденсатором
One hypothesis is that the duration is the time for the signal to transition from low state to high state, which is what RC circuit does - it delays the transition to a lower state 

#### Висновок

<!-- Обрані R і C та чому -->
