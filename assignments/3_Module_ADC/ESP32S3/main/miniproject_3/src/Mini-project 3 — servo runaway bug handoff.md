# Mini-project 3 — servo runaway bug handoff

Nov 6, 2026 · @Gosha

## Symptom

The servo runs away because `servo_update_duty()` adds the encoder's whole PCNT count to the angle on every 50 ms cycle, not only the change since the last cycle. The first encoder click starts it, and it never stops while the count is non-zero.

What the log shows (`miniproject_3/src/main.c`, build of 2026-11-06):

- Before any rotation: `servo idle, at 0 deg` every 50 ms, `SERVO_DUTY=0`. The status struct says 0°, but `servo_init()` already drove the shaft to 90°.
- First click (t = 13.1 s): 500 ms later `ENCODER_ANGLE=370 deg`, and `ENCODER_PULSES` still reads 0.
- Turning back: the angle falls by \~900° per 500 ms (−90° per cycle = count of −9 × 10°/tick) to −4030°, then rises again (+40°/cycle) once the count moves back toward zero.
- `SERVO_ANGLE` reports 170 / 10, but `SERVO_PULSE` is 2.000 ms / 1.001 ms, which is 180° / 0°: the shaft goes past the soft limits.
- `ANGLE_ERROR` reaches −40 400 %.

## Root causes

Bug 1 alone explains the runaway. Bugs 2–4 explain the out-of-limit pulses and the wrong numbers. Line numbers are from `main.c` as of 11:46 on 2026-11-06.

| # | Where | Bug | Effect in the log |
| --- | --- | --- | --- |
| 1 | `servo_update_duty()` L377–381; `encoder_update_ticks()` L322–326 | `encoder_update_ticks()` returns the absolute PCNT count, and `status->ticks` is never written, so it stays 0. `new_ticks - current_ticks` is the whole count every cycle, and the angle integrates it 20 times a second. | The runaway. `ENCODER_PULSES=0` while the shaft moves. The idle check `ticks == status->ticks` is true only when the count is back at exactly 0. |
| 2 | `servo_update_status()` L399 | `status->angle_deg` stores the unclamped command, so the position winds up past the limits (370°, −4030°). | Turning back does nothing until the whole overshoot is undone. Even after bug 1 is fixed, this makes the knob feel dead at the limits. |
| 3 | `servo_update_status()` L393, L400 | Duty is computed from `cmd_angle_deg`, not from `achieved_angle_deg`. `map_value()` then clamps to 0..180, not to 10..170. Duty is also computed twice. | `SERVO_ANGLE=170` with a 2.000 ms pulse (180°), and `SERVO_ANGLE=10` with 1.001 ms (0°). The shaft hits the end stops. |
| 4 | `app_main()` L156 | `servo_status_t servo_status = {0}`, but `servo_init()` drives the shaft to `SERVO_INIT_DEG` (90°). | The status says 0° at start. The first click commands 0 + 10 = 10°, so the shaft jumps from 90° to 10°. |
| 5 | `servo_update_status()` L397 | `buzzer_error_flag = abs(deviation_deg) == 0` is inverted. | When `buzzer_emit_note()` is implemented, it would beep on every normal move and stay silent at the limits. |
| 6 | `servo_update_status()` L403 | `calc_deviation_percentf(cmd, achieved)` divides by the achieved angle (10° at the low limit). | `ANGLE_ERROR` of −40 400 %, which says nothing useful. |
| 7 | `servo_update_duty()` L377 | The `servo idle` log is `ESP_LOGI` and runs every 50 ms cycle. | Floods the console and buries the 500 ms status lines. |

## Fix plan for the next session

Fix in this order and flash after step 1. Step 1 alone should stop the runaway.

1. **Make the encoder return a delta (bug 1).** Keep the previous count in a `static int` inside `encoder_update_ticks()` and return `count - prev`. Do not call `pcnt_unit_clear_count()` every cycle, because ticks that land between the read and the clear are lost. In `servo_update_duty()`, use `ticks` directly as the step count, and keep a running total in `status->ticks` only for the log (`status->ticks += ticks`).
2. **Clamp the stored position (bugs 2, 3).** Compute `target = position + ticks * SERVO_STEP_FINE_DEG * speed_k`. Then `clamped = val_clamp(target, SERVO_MIN_DEG, SERVO_MAX_DEG)`, store `clamped` as the position, and compute duty once from `clamped`.
3. **Seed the status from the init angle (bug 4).** Set `angle_deg` and `achieved_angle_deg` to `SERVO_INIT_DEG`, and set the duty, before the loop starts.
4. **Set the beep flag from the clamp (bug 5).** `out_of_bounds = (ticks != 0) && (target != clamped)`. A servo resting at a limit stays silent.
5. **Fix or drop the % error (bug 6).** Either divide by the travel range, `deviation / (SERVO_MAX_DEG - SERVO_MIN_DEG) * 100`, or remove it. After step 2 the deviation is just the requested overshoot.
6. **Quiet the idle log (bug 7).** Change it to `ESP_LOGD`, or delete it.

Sketch of `servo_update_duty()` after steps 1–4 (the delta comes from step 1):

```c
bool servo_update_duty(int32_t ticks, int32_t speed_k, servo_status_t *status)
{
    if (status == NULL) return false;
    status->buzzer_error_flag = 0;
    if (ticks == 0) return false;                       // no movement this cycle

    status->ticks += ticks;                             // running total, log only
    int32_t target  = status->achieved_angle_deg + ticks * SERVO_STEP_FINE_DEG * speed_k;
    int32_t clamped = val_clamp(target, SERVO_MIN_DEG, SERVO_MAX_DEG);

    status->angle_deg          = (float)target;         // what was asked
    status->achieved_angle_deg = clamped;               // where the shaft goes
    status->deviation_deg      = (float)(target - clamped);
    status->buzzer_error_flag  = (target != clamped);   // tried to go past a limit
    status->duty_ticks         = servo_angle_to_duty_ticks(clamped);
    status->pulse_width_ms     = duty_to_pulse_width_ms(status->duty_ticks);

    return ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, status->duty_ticks) == ESP_OK
        && ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL) == ESP_OK;
}
```

Check after flashing: one detent should move `SERVO_ANGLE` by 10° (with `SPEED_K=2`) and then hold. Turning back from a limit should move the shaft on the first click. The pulse should stay between \~1.06 ms (10°) and \~1.94 ms (170°).

## Other open items

Three of the flowchart blocks are still empty stubs, so the button and the beep cannot work yet whatever the servo code does.

- [ ] `button_pressed_event()`: always returns `false`. Needs edge detection on `ENCODER_GPIO_SW` (1 → 0, active LOW).
- [ ] `speed_update()`: returns `k` unchanged. Needs `k = (2 * k) % SPEED_N`. With N = 3, k alternates 2 → 1 → 2.
- [ ] `buzzer_emit_note()` / `buzzer_update()`: empty. Needs a file-scope cycle counter, `ledc_set_freq()` + duty 512 to start the note, and duty 0 when the counter hits 0.
- [ ] `encoder_output_ticks()`: a new stub that is never called, and it produces an unused-function warning.
- [ ] `servo_updated` in `app_main()` is unused, and it produces an unused-variable warning.
- [ ] The `map_value()` rounding is a no-op: the integer division happens before `+ 0.5f`. At most 1 duty tick of error.
- [ ] Dead or duplicate defines: `SERVO_MIN_US`/`SERVO_MAX_US` (500/2500) are unused next to `SERVO_MIN_PULSE_US`/`SERVO_MAX_PULSE_US` (1000/2000). `SERVO_RANGE_DEG` duplicates `SERVO_MAX_RANGE_DEG`. `ENCODER_CPR` uses an undefined `ENCODER_PPR`.
- [ ] The wiring comment at the top of the file still lists the old pins (5/6/7/14/18).
- [ ] Unknown: how many PCNT counts one detent gives in X1 mode. Read `ENCODER_PULSES` after step 1 of the fix plan to find out.

## Context

The build compiles and runs. The only problems are in the runtime logic above.

| Item | Value |
| --- | --- |
| File | `D:\!SOFT\!Documents\embed\training\beetroot\assignments\3_Module_ADC\ESP32S3\main\miniproject_3\src\main.c` |
| Build | ESP-IDF v6.1 VS Code extension, target `esp32s3`, COM16. `main/CMakeLists.txt` `SRCS` must point to `miniproject_3/src/main.c`. |
| Architecture | Superloop in `app_main()`, no tasks or queues. `vTaskDelay(50 ms)` per cycle, status log every 500 ms. |
| Pins (current code) | Encoder A = GPIO4, B = GPIO5, SW = GPIO6; servo = GPIO7; buzzer = GPIO16 |
| PWM | Servo: LEDC timer 0 / ch 0, 50 Hz, 13-bit, 1.0–2.0 ms = 0–180°, soft limits 10–170°. Buzzer: LEDC timer 1 / ch 1, 10-bit, 440 Hz. |
| Step | `SERVO_STEP_FINE_DEG` 5° × k, k starts at 2 (10°/tick), N = 3 |
| Backups | `main.c.bak_2026-11-06` (before the declaration and log fixes), `main_ldr_servo.c.bak` (the old LDR draft) |
