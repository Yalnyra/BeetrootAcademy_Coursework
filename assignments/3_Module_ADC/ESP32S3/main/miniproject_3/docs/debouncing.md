## Журнал налагодження

### 1. Runaway кута сервоприводу (06.11)

<!-- Симптом → Як відтворити → Доказ → Причина → Виправлення (commit 084d557) -->

### 2. Лічильник рахує лише вгору (08.11)

<!-- Симптом → Як відтворити → Доказ → Причина → Виправлення (commit 9814e05) -->

### 3. Дребезг енкодера та RC-фільтр (09.11)

#### Методика

! This Visualization comes from [Falstad](https://falstad.com/circuit/)

![Actual schema of the switch](rc_and_switch.png)

2. Вибір проміжку пропуску частот для Low-pass filter  

За простотою виконання (без катушки індуктивності) було обрано пару Resistor + Capacitor 

1. Writing a single pulse at a time 

First half is scrolling encoder wheel fast, first in cw, then in ccw direction, second half is rotating one step at a time in ccw directio

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


##### R = 0 Ω, C = 0 F

[0ohm_0capacitor](../../../output/videos/0ohm_0capacitor.mp4)
- Saleae Logic 2: [09.11 cwccw encodeerbaseline](../../../output/09.11_encoder_baseline)

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
- Saleae Logic 2: [720ohm_rc.sal](../../../output/new_encoder_2x_104_capacitor.sal)

**Лог програми з RC-фільтром:**

```
todo
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

- 

- З огляду на лог відео X4 Модуляція більш чутлива до зміни положення при великих шумах на переході - але при цьому рахує по 7-10 за один перехід в залежності від довжини переходу до HIGH стану

The bounce frequency itself is random. The time itself that bounces is the one that correlates with resistor value. 

#### Інтерпретація

One hypothesis is that the duration is the time for the signal to transition from low state to high state, which is what RC circuit does - it delays the transition to a lower state 

#### Висновок

<!-- Обрані R і C та чому -->
