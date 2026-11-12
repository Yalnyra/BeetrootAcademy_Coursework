


! This Visualization comes from [Falstad](https://falstad.com/circuit/)

![Actual schema of the switch](rc_and_switch.png)

1. Writing a single pulse at a time 

First half is scrolling encoder wheel fast, first in cw, then in ccw direction, second half is rotating one step at a time in ccw directio

2. Вибір проміжку пропуску частот для Low-pass filter  

За простотою виконання (без катушки індуктивності) було обрано пару Resistor + Capacitor 

### Таблиця виконання двигуна 
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
| New encoder, |                              | 0.1   | 0      | 0.64                      | 12000(232)                                    | 2                            |

The bounce frequency itself is random. The time itself that bounces is the one that correlates with resistor value. 

Hello, another small project to show - an mechanical encoder controlling a PWM servo 
The highlight is I tried learning filter theory by connecting rc filter and comparing the length of bounce period in logic to the resistor which I put in.
I made a table on github and added the recordings from Saleae Logic 2 for each setup. Maybe you'd like to fact check if the conclusions i made based  table make sense


One hypothesis is that the duration is the time for the signal to transition from low state to high state, which is what RC circuit does - it delays the transition to a lower state 

```
I (766285) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-100.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-110.00 deg (-1100.00%)
I (766285) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (766335) mini_project_3: COUNT DECREASED BY -2
E (766385) mini_project_3: COUNT DECREASED BY -3
E (766435) mini_project_3: COUNT DECREASED BY -3
E (766485) mini_project_3: COUNT DECREASED BY -4
E (766535) mini_project_3: COUNT DECREASED BY -4
E (766585) mini_project_3: COUNT DECREASED BY -2
E (766635) mini_project_3: COUNT DECREASED BY -3
E (766685) mini_project_3: COUNT DECREASED BY -4
E (766735) mini_project_3: COUNT DECREASED BY -5
E (766785) mini_project_3: COUNT DECREASED BY -4
E (766835) mini_project_3: COUNT DECREASED BY -7
E (766885) mini_project_3: COUNT DECREASED BY -4
E (766935) mini_project_3: COUNT DECREASED BY -3
W (767335) mini_project_3: COUNT INCREASED BY 4
W (767385) mini_project_3: COUNT INCREASED BY 2
W (767435) mini_project_3: COUNT INCREASED BY 2
W (767485) mini_project_3: COUNT INCREASED BY 2
W (767535) mini_project_3: COUNT INCREASED BY 2
W (767635) mini_project_3: COUNT INCREASED BY 2
W (767685) mini_project_3: COUNT INCREASED BY 2
W (767785) mini_project_3: COUNT INCREASED BY 2
W (767835) mini_project_3: COUNT INCREASED BY 2
W (767935) mini_project_3: COUNT INCREASED BY 4
W (767985) mini_project_3: COUNT INCREASED BY 2
W (768035) mini_project_3: COUNT INCREASED BY 2
W (768085) mini_project_3: COUNT INCREASED BY 4
W (768135) mini_project_3: COUNT INCREASED BY 6
W (768185) mini_project_3: COUNT INCREASED BY 4
W (768235) mini_project_3: COUNT INCREASED BY 2
I (768285) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-110.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-120.00 deg (-1200.00%)
I (768285) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (768485) mini_project_3: COUNT DECREASED BY -2
E (768535) mini_project_3: COUNT DECREASED BY -3
E (768585) mini_project_3: COUNT DECREASED BY -3
E (768635) mini_project_3: COUNT DECREASED BY -2
E (768685) mini_project_3: COUNT DECREASED BY -3
E (768735) mini_project_3: COUNT DECREASED BY -3
E (768785) mini_project_3: COUNT DECREASED BY -4
E (768835) mini_project_3: COUNT DECREASED BY -1
E (768885) mini_project_3: COUNT DECREASED BY -3
E (768935) mini_project_3: COUNT DECREASED BY -2
E (768985) mini_project_3: COUNT DECREASED BY -2
E (769085) mini_project_3: COUNT DECREASED BY -2
E (769135) mini_project_3: COUNT DECREASED BY -2
E (769185) mini_project_3: COUNT DECREASED BY -1
E (769235) mini_project_3: COUNT DECREASED BY -3
W (769885) mini_project_3: COUNT INCREASED BY 2
W (769935) mini_project_3: COUNT INCREASED BY 2
W (769985) mini_project_3: COUNT INCREASED BY 2
W (770035) mini_project_3: COUNT INCREASED BY 2
W (770185) mini_project_3: COUNT INCREASED BY 1
W (770235) mini_project_3: COUNT INCREASED BY 1
W (770285) mini_project_3: COUNT INCREASED BY 2
I (770285) mini_project_3: ENCODER_PULSES=1, ENCODER_ANGLE=-170.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-180.00 deg (-1800.00%)
I (770285) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (770745) mini_project_3: COUNT DECREASED BY -2
E (770795) mini_project_3: COUNT DECREASED BY -2
E (770895) mini_project_3: COUNT DECREASED BY -1
E (770945) mini_project_3: COUNT DECREASED BY -1
E (770995) mini_project_3: COUNT DECREASED BY -2
W (771345) mini_project_3: COUNT INCREASED BY 2
W (771395) mini_project_3: COUNT INCREASED BY 2
W (771545) mini_project_3: COUNT INCREASED BY 2
W (771595) mini_project_3: COUNT INCREASED BY 2
W (771745) mini_project_3: COUNT INCREASED BY 2
W (771795) mini_project_3: COUNT INCREASED BY 2
E (772195) mini_project_3: COUNT DECREASED BY -1
E (772245) mini_project_3: COUNT DECREASED BY -3
I (772295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-170.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-180.00 deg (-1800.00%)
I (772295) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (772345) mini_project_3: COUNT DECREASED BY -1
E (772395) mini_project_3: COUNT DECREASED BY -1
E (772445) mini_project_3: COUNT DECREASED BY -2
W (773045) mini_project_3: COUNT INCREASED BY 1
W (773095) mini_project_3: COUNT INCREASED BY 3
W (773245) mini_project_3: COUNT INCREASED BY 2
W (773295) mini_project_3: COUNT INCREASED BY 2
E (773695) mini_project_3: COUNT DECREASED BY -1
E (773745) mini_project_3: COUNT DECREASED BY -3
E (773795) mini_project_3: COUNT DECREASED BY -1
E (773845) mini_project_3: COUNT DECREASED BY -3
E (774295) mini_project_3: COUNT DECREASED BY -1
I (774295) mini_project_3: ENCODER_PULSES=0, ENCODER_ANGLE=-180.00 deg, SERVO_ANGLE=10/180 deg, ANGLE_ERROR=-190.00 deg (-1900.00%)
I (774295) mini_project_3: SERVO_DUTY=410 ticks, SERVO_PULSE=1.001 ms, SPEED_K=2
E (774355) mini_project_3: COUNT DECREASED BY -1
E (774405) mini_project_3: COUNT DECREASED BY -2
```

### Worth mentioning 

#### Quadrature 

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