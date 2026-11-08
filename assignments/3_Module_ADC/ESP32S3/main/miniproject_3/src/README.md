# <3.5>

> Модуль 3. ЦАП та ШИМ модуляція

> Завдання 3.5 Точне керування положенням (Серво)  


### Схемотехніка

[Посилання на Діаграму](https://app.cirkitdesigner.com/project/1d9b4079-1a9c-4b3b-b021-867db9eda888)
[Вивід в консоль](output/3.5_output.log)

### Вимоги до програми (Код)

Потенціометр керує положенням валу сервоприводу в пропорції 1:1.
Розбіжність кута повороту сервоприводу та потенціометра обрізаються і використовується тільки та частина діапазону, котра збігатися.
Кут відхилення від крайнього лівого положення логується в консолі.
### Що виконано

1. 0-4095 (POT_GPIO4) -> 0-270 (POT_ANGLE) (Мапа на кут Потенціометра) 
2. 0-270 -> (SERVO_ANGLE) 0-180 (Обрізання до положення сервомотора SG90) -> 
3. 0-180 -> 1-2 мс (Маппінг у довжину PWM пульсу) 
4. 1-2мс (SERVO_PULSE) -> Ширина duty cycle (SERVO_DUTY) 1/50 - 1/25 Гц 
5. Помилка між POT_ANGLE та SERVO_PULSE, конвертований в кут, з функцією duty_to_servo_angle_deg()

## Набір деталей

| Деталь | Опис |
|------|--------|
| Плата | ESP32-S3 (devkit) |
| Потенціометр | x1 |
| Резистор | x1 220 Ом |
| Світлодіод | x1 Червоний |
| Сервопривід | x1 SG90 (Tower Pro) |
| Джерело живлення | 5V зовнішнє |'


### Вихід програми 

#### Опис Дебаггінгу 

1. Writing a single pulse at a time 

First half is scrolling encoder wheel fast, first in cw, then in ccw direction, second half is rotating one step at a time in ccw directio


```

I (114275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (114275) mini_project_3: SERVO_DUTY=0 ticks, SERVO_PULSE=0.000 ms, SPEED_K=2
I (116275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=0.00 deg, SERVO_ANGLE=0/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (116275) mini_project_3: SERVO_DUTY=0 ticks, SERVO_PULSE=0.000 ms, SPEED_K=2
W (117425) mini_project_3: COUNT INCREASED BY 2
W (117475) mini_project_3: COUNT INCREASED BY 2
W (117575) mini_project_3: COUNT INCREASED BY 1
I (118275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=50.00 deg, SERVO_ANGLE=50/180 deg, ANGLE_ERROR=0.00 deg (0.00%)
I (118275) mini_project_3: SERVO_DUTY=523 ticks, SERVO_PULSE=1.277 ms, SPEED_K=2
W (118625) mini_project_3: COUNT INCREASED BY 1
W (118825) mini_project_3: COUNT INCREASED BY 1
W (118975) mini_project_3: COUNT INCREASED BY 1
W (119125) mini_project_3: COUNT INCREASED BY 1
W (119275) mini_project_3: COUNT INCREASED BY 1
W (119375) mini_project_3: COUNT INCREASED BY 1
W (119675) mini_project_3: COUNT INCREASED BY 2
W (119725) mini_project_3: COUNT INCREASED BY 2
W (119775) mini_project_3: COUNT INCREASED BY 3
W (119825) mini_project_3: COUNT INCREASED BY 2
W (119875) mini_project_3: COUNT INCREASED BY 3
W (119925) mini_project_3: COUNT INCREASED BY 1
I (120275) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=240.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=70.00 deg (41.18%)
I (120275) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (120325) mini_project_3: COUNT INCREASED BY 4
W (120375) mini_project_3: COUNT INCREASED BY 4
W (120475) mini_project_3: COUNT INCREASED BY 3
W (120525) mini_project_3: COUNT INCREASED BY 1
W (121875) mini_project_3: COUNT INCREASED BY 1
W (121925) mini_project_3: COUNT INCREASED BY 1
W (121975) mini_project_3: COUNT INCREASED BY 4
W (122025) mini_project_3: COUNT INCREASED BY 2
W (122075) mini_project_3: COUNT INCREASED BY 2
W (122175) mini_project_3: COUNT INCREASED BY 1
W (122225) mini_project_3: COUNT INCREASED BY 1
E (122275) mini_project_3: COUNT DECREASED BY -1
I (122275) mini_project_3: ENCODER_PULSES=-1, ENCODER_ANGLE=470.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=300.00 deg (176.47%)
I (122275) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (123635) mini_project_3: COUNT INCREASED BY 1
W (123785) mini_project_3: COUNT INCREASED BY 2
W (123835) mini_project_3: COUNT INCREASED BY 1
W (124035) mini_project_3: COUNT INCREASED BY 3
W (124085) mini_project_3: COUNT INCREASED BY 1
W (124135) mini_project_3: COUNT INCREASED BY 1
W (124235) mini_project_3: COUNT INCREASED BY 2
W (124285) mini_project_3: COUNT INCREASED BY 1
I (124285) mini_project_3: ENCODER_PULSES=1, ENCODER_ANGLE=590.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=420.00 deg (247.06%)
I (124285) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (124345) mini_project_3: COUNT INCREASED BY 1
W (124395) mini_project_3: COUNT INCREASED BY 1
W (124795) mini_project_3: COUNT INCREASED BY 1
W (124895) mini_project_3: COUNT INCREASED BY 1
W (124945) mini_project_3: COUNT INCREASED BY 1
W (124995) mini_project_3: COUNT INCREASED BY 1
W (125045) mini_project_3: COUNT INCREASED BY 1
W (125095) mini_project_3: COUNT INCREASED BY 1
W (125145) mini_project_3: COUNT INCREASED BY 1
W (125195) mini_project_3: COUNT INCREASED BY 1
W (125545) mini_project_3: COUNT INCREASED BY 2
W (125645) mini_project_3: COUNT INCREASED BY 2
W (125795) mini_project_3: COUNT INCREASED BY 1
I (126295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=740.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=570.00 deg (335.29%)
I (126295) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (126695) mini_project_3: COUNT INCREASED BY 1
W (126795) mini_project_3: COUNT INCREASED BY 1
W (126895) mini_project_3: COUNT INCREASED BY 1
W (126945) mini_project_3: COUNT INCREASED BY 1
W (126995) mini_project_3: COUNT INCREASED BY 1
W (127095) mini_project_3: COUNT INCREASED BY 3
W (127145) mini_project_3: COUNT INCREASED BY 1
W (127245) mini_project_3: COUNT INCREASED BY 2
W (127395) mini_project_3: COUNT INCREASED BY 2
W (127495) mini_project_3: COUNT INCREASED BY 2
W (127545) mini_project_3: COUNT INCREASED BY 2
W (127645) mini_project_3: COUNT INCREASED BY 2
W (127745) mini_project_3: COUNT INCREASED BY 1
W (127795) mini_project_3: COUNT INCREASED BY 1
W (127845) mini_project_3: COUNT INCREASED BY 3
W (127895) mini_project_3: COUNT INCREASED BY 1
W (128145) mini_project_3: COUNT INCREASED BY 1
I (128295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=1000.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=830.00 deg (488.24%)
I (128295) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (129095) mini_project_3: COUNT INCREASED BY 4
E (129145) mini_project_3: COUNT DECREASED BY -4
W (129345) mini_project_3: COUNT INCREASED BY 1
I (130295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=1010.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=840.00 deg (494.12%)
I (130295) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (130445) mini_project_3: COUNT INCREASED BY 1
W (130545) mini_project_3: COUNT INCREASED BY 5
W (130595) mini_project_3: COUNT INCREASED BY 2
I (132295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=1090.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=920.00 deg (541.18%)
I (132295) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (133095) mini_project_3: COUNT INCREASED BY 3
W (133145) mini_project_3: COUNT INCREASED BY 2
W (133195) mini_project_3: COUNT INCREASED BY 12
I (134295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=1260.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=1090.00 deg (641.18%)
I (134295) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (134495) mini_project_3: COUNT INCREASED BY 3
W (134545) mini_project_3: COUNT INCREASED BY 4
W (134595) mini_project_3: COUNT INCREASED BY 5
W (134895) mini_project_3: COUNT INCREASED BY 1
W (134945) mini_project_3: COUNT INCREASED BY 5
W (135195) mini_project_3: COUNT INCREASED BY 2
W (135245) mini_project_3: COUNT INCREASED BY 3
W (135295) mini_project_3: COUNT INCREASED BY 6
W (135395) mini_project_3: COUNT INCREASED BY 2
E (135445) mini_project_3: COUNT DECREASED BY -2
W (135545) mini_project_3: COUNT INCREASED BY 7
W (135595) mini_project_3: COUNT INCREASED BY 5
W (135645) mini_project_3: COUNT INCREASED BY 6
W (135695) mini_project_3: COUNT INCREASED BY 8
W (135745) mini_project_3: COUNT INCREASED BY 10
W (135795) mini_project_3: COUNT INCREASED BY 1
W (135845) mini_project_3: COUNT INCREASED BY 7
W (135895) mini_project_3: COUNT INCREASED BY 2
W (135945) mini_project_3: COUNT INCREASED BY 2
W (135995) mini_project_3: COUNT INCREASED BY 6
W (136045) mini_project_3: COUNT INCREASED BY 2
W (136095) mini_project_3: COUNT INCREASED BY 12
W (136145) mini_project_3: COUNT INCREASED BY 8
W (136195) mini_project_3: COUNT INCREASED BY 7
W (136245) mini_project_3: COUNT INCREASED BY 6
W (136295) mini_project_3: COUNT INCREASED BY 5
I (136295) mini_project_3: ENCODER_PULSES=5, ENCODER_ANGLE=2490.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=2320.00 deg (1364.71%)
I (136295) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (136355) mini_project_3: COUNT INCREASED BY 5
W (136405) mini_project_3: COUNT INCREASED BY 1
W (136505) mini_project_3: COUNT INCREASED BY 1
W (136555) mini_project_3: COUNT INCREASED BY 1
E (136605) mini_project_3: COUNT DECREASED BY -1
W (136655) mini_project_3: COUNT INCREASED BY 6
W (136705) mini_project_3: COUNT INCREASED BY 9
W (136755) mini_project_3: COUNT INCREASED BY 27
W (136805) mini_project_3: COUNT INCREASED BY 11
W (136855) mini_project_3: COUNT INCREASED BY 2
E (136955) mini_project_3: COUNT DECREASED BY -3
E (137005) mini_project_3: COUNT DECREASED BY -3
E (137105) mini_project_3: COUNT DECREASED BY -8
E (137155) mini_project_3: COUNT DECREASED BY -1
E (137205) mini_project_3: COUNT DECREASED BY -1
W (138005) mini_project_3: COUNT INCREASED BY 1
E (138105) mini_project_3: COUNT DECREASED BY -1
I (138305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=2950.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=2780.00 deg (1635.29%)
I (138305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (138405) mini_project_3: COUNT INCREASED BY 2
W (138455) mini_project_3: COUNT INCREASED BY 1
W (138505) mini_project_3: COUNT INCREASED BY 2
W (138605) mini_project_3: COUNT INCREASED BY 1
E (138655) mini_project_3: COUNT DECREASED BY -1
W (138855) mini_project_3: COUNT INCREASED BY 1
E (138905) mini_project_3: COUNT DECREASED BY -1
W (140105) mini_project_3: COUNT INCREASED BY 4
I (140305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=3040.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=2870.00 deg (1688.24%)
I (140305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (140405) mini_project_3: COUNT INCREASED BY 3
W (140605) mini_project_3: COUNT INCREASED BY 1
W (140655) mini_project_3: COUNT INCREASED BY 1
W (140755) mini_project_3: COUNT INCREASED BY 2
W (140955) mini_project_3: COUNT INCREASED BY 2
W (141005) mini_project_3: COUNT INCREASED BY 1
W (141255) mini_project_3: COUNT INCREASED BY 1
W (141755) mini_project_3: COUNT INCREASED BY 1
W (141805) mini_project_3: COUNT INCREASED BY 19
W (142005) mini_project_3: COUNT INCREASED BY 1
W (142105) mini_project_3: COUNT INCREASED BY 2
I (142305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=3380.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=3210.00 deg (1888.24%)
I (142305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (142855) mini_project_3: COUNT INCREASED BY 1
W (142905) mini_project_3: COUNT INCREASED BY 3
W (142955) mini_project_3: COUNT INCREASED BY 2
W (143005) mini_project_3: COUNT INCREASED BY 2
W (143055) mini_project_3: COUNT INCREASED BY 1
W (143105) mini_project_3: COUNT INCREASED BY 2
W (143155) mini_project_3: COUNT INCREASED BY 1
W (143205) mini_project_3: COUNT INCREASED BY 1
W (143605) mini_project_3: COUNT INCREASED BY 11
W (143655) mini_project_3: COUNT INCREASED BY 4
W (143705) mini_project_3: COUNT INCREASED BY 4
W (143755) mini_project_3: COUNT INCREASED BY 3
W (143805) mini_project_3: COUNT INCREASED BY 3
W (143905) mini_project_3: COUNT INCREASED BY 1
I (144305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=3770.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=3600.00 deg (2117.65%)
I (144305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (144655) mini_project_3: COUNT INCREASED BY 2
W (145055) mini_project_3: COUNT INCREASED BY 1
W (145105) mini_project_3: COUNT INCREASED BY 1
W (145655) mini_project_3: COUNT INCREASED BY 1
W (146005) mini_project_3: COUNT INCREASED BY 1
I (146305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=3830.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=3660.00 deg (2152.94%)
I (146305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (146455) mini_project_3: COUNT INCREASED BY 2
W (146805) mini_project_3: COUNT INCREASED BY 1
W (147255) mini_project_3: COUNT INCREASED BY 1
E (147305) mini_project_3: COUNT DECREASED BY -1
E (147555) mini_project_3: COUNT DECREASED BY -2
W (147805) mini_project_3: COUNT INCREASED BY 1
W (148255) mini_project_3: COUNT INCREASED BY 1
I (148305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=3860.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=3690.00 deg (2170.59%)
I (148305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (148605) mini_project_3: COUNT INCREASED BY 1
W (148955) mini_project_3: COUNT INCREASED BY 1
W (149155) mini_project_3: COUNT INCREASED BY 2
W (149805) mini_project_3: COUNT INCREASED BY 2
I (150305) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=3920.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=3750.00 deg (2205.88%)
I (150305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (150855) mini_project_3: COUNT INCREASED BY 3
W (150905) mini_project_3: COUNT INCREASED BY 3
W (152205) mini_project_3: COUNT INCREASED BY 3
W (152255) mini_project_3: COUNT INCREASED BY 3
W (152305) mini_project_3: COUNT INCREASED BY 1
I (152305) mini_project_3: ENCODER_PULSES=1, ENCODER_ANGLE=4050.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=3880.00 deg (2282.35%)
I (152305) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (152365) mini_project_3: COUNT INCREASED BY 2
W (152415) mini_project_3: COUNT INCREASED BY 1
W (152465) mini_project_3: COUNT INCREASED BY 2
W (152515) mini_project_3: COUNT INCREASED BY 3
W (152565) mini_project_3: COUNT INCREASED BY 1
W (152615) mini_project_3: COUNT INCREASED BY 3
W (152715) mini_project_3: COUNT INCREASED BY 1
W (152815) mini_project_3: COUNT INCREASED BY 1
W (153515) mini_project_3: COUNT INCREASED BY 2
W (153915) mini_project_3: COUNT INCREASED BY 2
W (153965) mini_project_3: COUNT INCREASED BY 1
W (154265) mini_project_3: COUNT INCREASED BY 2
W (154315) mini_project_3: COUNT INCREASED BY 3
I (154315) mini_project_3: ENCODER_PULSES=3, ENCODER_ANGLE=4290.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4120.00 deg (2423.53%)
I (154315) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (154475) mini_project_3: COUNT INCREASED BY 2
W (154675) mini_project_3: COUNT INCREASED BY 3
W (154725) mini_project_3: COUNT INCREASED BY 1
W (154775) mini_project_3: COUNT INCREASED BY 2
W (154825) mini_project_3: COUNT INCREASED BY 1
W (155625) mini_project_3: COUNT INCREASED BY 1
W (155675) mini_project_3: COUNT INCREASED BY 1
E (155725) mini_project_3: COUNT DECREASED BY -1
E (155975) mini_project_3: COUNT DECREASED BY -1
W (156175) mini_project_3: COUNT INCREASED BY 1
E (156225) mini_project_3: COUNT DECREASED BY -1
I (156325) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4380.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4210.00 deg (2476.47%)
I (156325) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (156575) mini_project_3: COUNT INCREASED BY 4
E (156625) mini_project_3: COUNT DECREASED BY -1
E (156825) mini_project_3: COUNT DECREASED BY -1
W (157075) mini_project_3: COUNT INCREASED BY 1
E (157275) mini_project_3: COUNT DECREASED BY -1
W (157425) mini_project_3: COUNT INCREASED BY 1
W (157475) mini_project_3: COUNT INCREASED BY 1
I (158325) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4420.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4250.00 deg (2500.00%)
I (158325) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (158625) mini_project_3: COUNT INCREASED BY 2
W (158675) mini_project_3: COUNT INCREASED BY 3
W (158725) mini_project_3: COUNT INCREASED BY 1
W (159225) mini_project_3: COUNT INCREASED BY 1
W (159275) mini_project_3: COUNT INCREASED BY 1
W (159525) mini_project_3: COUNT INCREASED BY 2
W (159775) mini_project_3: COUNT INCREASED BY 1
W (159975) mini_project_3: COUNT INCREASED BY 2
W (160175) mini_project_3: COUNT INCREASED BY 4
I (160325) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4590.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4420.00 deg (2600.00%)
I (160325) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (160375) mini_project_3: COUNT INCREASED BY 5
W (161925) mini_project_3: COUNT INCREASED BY 1
W (162125) mini_project_3: COUNT INCREASED BY 2
W (162175) mini_project_3: COUNT INCREASED BY 1
W (162325) mini_project_3: COUNT INCREASED BY 2
I (162325) mini_project_3: ENCODER_PULSES=2, ENCODER_ANGLE=4700.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4530.00 deg (2664.71%)
I (162325) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (162385) mini_project_3: COUNT INCREASED BY 1
W (162535) mini_project_3: COUNT INCREASED BY 1
W (162585) mini_project_3: COUNT INCREASED BY 1
W (162885) mini_project_3: COUNT INCREASED BY 1
W (163135) mini_project_3: COUNT INCREASED BY 3
W (163335) mini_project_3: COUNT INCREASED BY 2
W (163535) mini_project_3: COUNT INCREASED BY 1
E (163585) mini_project_3: COUNT DECREASED BY -1
I (164335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4790.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4620.00 deg (2717.65%)
I (164335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (164485) mini_project_3: COUNT DECREASED BY -1
W (164735) mini_project_3: COUNT INCREASED BY 1
E (164785) mini_project_3: COUNT DECREASED BY -1
E (164985) mini_project_3: COUNT DECREASED BY -1
E (165285) mini_project_3: COUNT DECREASED BY -2
E (166185) mini_project_3: COUNT DECREASED BY -1
E (166235) mini_project_3: COUNT DECREASED BY -1
I (166335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4730.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4560.00 deg (2682.35%)
I (166335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (166485) mini_project_3: COUNT INCREASED BY 1
E (166535) mini_project_3: COUNT DECREASED BY -1
E (166735) mini_project_3: COUNT DECREASED BY -1
W (166785) mini_project_3: COUNT INCREASED BY 1
W (166985) mini_project_3: COUNT INCREASED BY 1
E (167485) mini_project_3: COUNT DECREASED BY -1
E (167685) mini_project_3: COUNT DECREASED BY -1
E (168035) mini_project_3: COUNT DECREASED BY -1
I (168335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4710.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4540.00 deg (2670.59%)
I (168335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (168385) mini_project_3: COUNT INCREASED BY 1
E (169335) mini_project_3: COUNT DECREASED BY -1
E (169885) mini_project_3: COUNT DECREASED BY -1
E (170185) mini_project_3: COUNT DECREASED BY -1
I (170335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4690.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4520.00 deg (2658.82%)
I (170335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (170485) mini_project_3: COUNT DECREASED BY -1
E (170835) mini_project_3: COUNT DECREASED BY -1
E (171035) mini_project_3: COUNT DECREASED BY -1
W (171285) mini_project_3: COUNT INCREASED BY 2
E (171385) mini_project_3: COUNT DECREASED BY -1
E (171785) mini_project_3: COUNT DECREASED BY -1
W (172035) mini_project_3: COUNT INCREASED BY 1
E (172085) mini_project_3: COUNT DECREASED BY -1
E (172235) mini_project_3: COUNT DECREASED BY -1
I (172335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4650.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4480.00 deg (2635.29%)
I (172335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (173335) mini_project_3: COUNT DECREASED BY -1
E (173635) mini_project_3: COUNT DECREASED BY -1
E (173985) mini_project_3: COUNT DECREASED BY -1
E (174185) mini_project_3: COUNT DECREASED BY -1
I (174335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4610.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4440.00 deg (2611.76%)
I (174335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
W (174385) mini_project_3: COUNT INCREASED BY 1
E (174485) mini_project_3: COUNT DECREASED BY -1
E (174685) mini_project_3: COUNT DECREASED BY -1
E (174885) mini_project_3: COUNT DECREASED BY -1
E (174985) mini_project_3: COUNT DECREASED BY -1
E (175135) mini_project_3: COUNT DECREASED BY -1
I (176335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4570.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4400.00 deg (2588.24%)
I (176335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (178035) mini_project_3: COUNT DECREASED BY -1
I (178335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4560.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4390.00 deg (2582.35%)
I (178335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (178535) mini_project_3: COUNT DECREASED BY -1
I (180335) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4550.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4380.00 deg (2576.47%)
I (180335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (182335) mini_project_3: COUNT DECREASED BY -1
I (182335) mini_project_3: ENCODER_PULSES=-1, ENCODER_ANGLE=4540.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4370.00 deg (2570.59%)
I (182335) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (184095) mini_project_3: COUNT DECREASED BY -1
I (184345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4530.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4360.00 deg (2564.71%)
I (184345) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (184695) mini_project_3: COUNT DECREASED BY -1
W (184745) mini_project_3: COUNT INCREASED BY 1
E (185245) mini_project_3: COUNT DECREASED BY -1
I (186345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4520.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4350.00 deg (2558.82%)
I (186345) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (188045) mini_project_3: COUNT DECREASED BY -1
I (188345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4510.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4340.00 deg (2552.94%)
I (188345) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
E (188495) mini_project_3: COUNT DECREASED BY -1
I (190345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4500.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4330.00 deg (2547.06%)
I (190345) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2
I (192345) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=4500.00 deg, SERVO_ANGLE=170/180 deg, ANGLE_ERROR=4330.00 deg (2547.06%)
I (192345) mini_project_3: SERVO_DUTY=819 ticks, SERVO_PULSE=2.000 ms, SPEED_K=2

```
## Передумови (версії, з якими зібрано проєкт)

| Компонент | Версія / шлях |
|------|--------|
| ESP-IDF | **v6.1.0** (`C:\esp\v6.1\esp-idf`) |
| Target | `esp32s3` (Xtensa) |
| Компілятор | `xtensa-esp32s3-elf-gcc` **15.2.0** (`esp-15.2.0_20251204`) |
| CMake | **4.0.3** (`C:\Espressif\tools\cmake\4.0.3`), мінімум у проєкті — 3.22 |
| Генератор | Ninja (`C:\Strawberry\c\bin\ninja.exe`) |
| Python (venv IDF) | **3.11.15** (`C:\Espressif\tools\python\v6.1\venv`) |
| clangd | esp-clangd **21.1.3** (`esp-21.1.3_20260408`) |
| VS Code розширення | `espressif.esp-idf-extension` |
| ОС | Windows 11 (є також `.devcontainer` з образом `espressif/idf`) |

Версії взято з `sdkconfig`, `build/project_description.json` та `build/CMakeCache.txt`.

## Швидкий старт

**Увага: PlatformIO депрекований, з метою побудови використовується плагін ESP-IDF**

Команди викликаються з палітри VS Code (`Ctrl+Shift+P`). В дужках — еквівалент `idf.py`
для терміналу `ESP-IDF: Open ESP-IDF Terminal`.

0. Вибрати файл для збірки в `main/CMakeLists.txt` — в `SRCS` має бути **існуючий** файл:

   ```cmake
   idf_component_register(SRCS "main_6_miniproject.c"
                          PRIV_REQUIRES esp_driver_gpio esp_timer)
   ```

1. Встановити ESP-IDF
   - `ESP-IDF: Open ESP-IDF Installation Manager` → встановити **v6.1** в `C:\esp\v6.1\esp-idf`
   - `ESP-IDF: Select Current ESP-IDF Version` → v6.1 (записується в `idf.currentSetup`)
   - `ESP-IDF: Set Espressif Device Target` → `esp32s3` (`idf.py set-target esp32s3`)
   - `ESP-IDF: Select Port to Use (COM, tty, usbserial)` → `COM16`

2. Run CMake Clean
   - `ESP-IDF: Full Clean Project` (`idf.py fullclean`) — видаляє `build/`, потрібно після зміни
     `SRCS`, версії IDF або target

3. Запустити перший білд
   - `ESP-IDF: Build your Project` / `Ctrl+E B` (`idf.py build`)
   - Після білду з'являється `build/compile_commands.json` — його читає clangd

4. Встановити flashing метод UART
   - `ESP-IDF: Select Flash Method` → `UART` (записується в `idf.flashType`)

5. Запустити Flash & Monitor task
   - `ESP-IDF: Build, Flash and Start a Monitor on your Device` / `Ctrl+E D`
     (`idf.py -p COM16 flash monitor`)
   - Окремо: Flash — `Ctrl+E F`, Monitor — `Ctrl+E M`; вихід з монітора — `Ctrl+]`
   - Консоль: UART0 @ **115200**, другорядна — USB-Serial-JTAG

### Конфігурація `.vscode`

`.vscode/settings.json` — налаштування розширення ESP-IDF та clangd:

```json
{
  "C_Cpp.intelliSenseEngine": "default",
  "idf.extensionActivationMode": "always",
  "cmake.configureOnOpen": false,
  "cmake.configureOnEdit": false,
  "cmake.automaticReconfigure": false,
  "cmake.autoSelectActiveFolder": false,
  "idf.openOcdConfigs": ["board/esp32s3-builtin.cfg"],
  "idf.port": "COM16",
  "idf.currentSetup": "C:\\esp\\v6.1\\esp-idf",
  "idf.customExtraVars": { "IDF_TARGET": "esp32s3" },
  "idf.flashType": "UART",
  "clangd.path": "C:\\Espressif\\tools\\esp-clangd\\esp-21.1.3_20260408\\esp-clangd\\bin\\clangd.exe",
  "clangd.arguments": [
    "--background-index",
    "--query-driver=**",
    "--compile-commands-dir=d:\\!SOFT\\!Documents\\embed\\training\\beetroot\\assignments\\2.6_ESPIDF_Debounce\\build"
  ]
}
```

- `cmake.*: false` — вимикає CMake Tools, щоб він не конфігурував проєкт паралельно з ESP-IDF
- `idf.openOcdConfigs` — вбудований USB-JTAG ESP32-S3 для налагодження
- `--compile-commands-dir` — абсолютний шлях; при перенесенні проєкту його треба змінити

`.vscode/c_cpp_properties.json` — IntelliSense бере прапорці з `${config:idf.buildPath}/compile_commands.json`.

`.vscode/launch.json` — конфігурація `gdbtarget` / `attach` (Eclipse CDT GDB Adapter) для
налагодження через OpenOCD.

`.clangd` — `Remove: [-f*, -m*]` прибирає специфічні для Xtensa GCC прапорці, які clangd не розуміє.

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

## What I would add

- LDR resistor ADC control

## TODO
- [] Add hardware requirements
- [] Add circuit diagram 
- [] Add code function comments
- [] Add debugging log 