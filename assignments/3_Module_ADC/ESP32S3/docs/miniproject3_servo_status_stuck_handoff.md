# Mini-project 3 — servo stuck at the limit ("status update error") handoff (2026-11-18)

## Fix applied (2026-11-18)

### Bug 1 — stale `angle_deg` blocked re-entry from the clamped boundary

The early-exit check compared the new **unclamped** command against `achieved_angle_deg` (the **clamped** last position):

```c
if (cmd_angle_deg == status->achieved_angle_deg) return false;
```

When the encoder rolled one step back from a clamped boundary, the new unclamped command equalled the old clamped value exactly, so the function returned `false` without writing anything. Because `angle_deg` (the base for the next command) was never updated, every subsequent click re-computed the same value and was rejected again — the servo stayed stuck indefinitely, even as the encoder kept turning.

Fix: compare the **clamped** result against the **clamped** last position. Store `angle_deg = achieved` (clamped) on every call so the next command is always based on a legal angle.

### Bug 2 — `buzzer_error_flag` was inverted

```c
// old — fires when deviation IS zero (no error)
bool buzzer_error_flag = abs(deviation_deg) == 0;

// fixed — fires when deviation is non-zero (tried to exceed a limit)
status->buzzer_error_flag = (status->deviation_deg != 0);
```

### Removal — `achieved_angle_deg` field dropped

`achieved_angle_deg` was redundant: the servo has no position feedback, so the driver sets the position itself. After the fix `angle_deg` always holds the clamped (= achieved) angle, making the second field unnecessary. It was removed from `servo_status_t`.

Previous handoff (runaway bug, fixed): `claude/miniproject3_handoff.md`.

## Symptom

After turning the encoder past the lower limit and back, single clicks in the opposite direction no longer move the servo. Every click logs:

```
W (113155) mini_project_3: COUNT INCREASED BY 2
I (113155) mini_project_3: status update error, at 10 deg
```

Original capture: `SERVO_MIN_DEG = 10` → stuck message "at 10 deg"; `ENCODER_ANGLE` climbs from -20 to 0 and freezes at 0.
Current code: `SERVO_MIN_DEG = 0` → same bug, now logged as `servo: status update error, at 0 deg`. `angle_deg` freezes at -10.
(The log tag changed from `mini_project_3` to `servo` when the driver moved to `components/servo`.)

It is **not** a rounding error. It is a comparison bug.

## Root cause

`components/servo/servo.c`, `servo_update_status()`, first line:

```c
if (cmd_angle_deg == status->achieved_angle_deg) return false;
```

- `cmd_angle_deg` = previous **unclamped** command `status->angle_deg` + `ticks * SERVO_STEP_FINE_DEG * speed_k` (computed in `main.c`, step 3).
- `achieved_angle_deg` = the **clamped** angle.
- On rejection nothing is written back, so `angle_deg` never changes. The next click computes the same command and is rejected again, permanently.

Trace with the current settings (`SERVO_MIN_DEG 0`, 10 deg per click, status starts zeroed):

| `angle_deg` (unclamped) | click | new command | clamped | result |
|---|---|---|---|---|
| 0 | CCW | -10 | 0 | accepted, `angle_deg` = -10 (below the limit) |
| -10 | CW | **0** | 0 | **0 == 0 → rejected** |
| -10 | CW | 0 | 0 | rejected again: stuck |

Why it depends on the click pattern:
- **X4:** `encoder_update_ticks()` returns one click at a time (two `+2` reads add up to 4 counts = 1 click), so each step lands exactly on the clamped value.
- **Two clicks in one read** give -10 + 20 = 10, which escapes.
- **Turning further CCW** keeps working (-20 ≠ 0).
- **Same at the top limit:** `angle_deg` drifts above 180, and a single click back lands on 180.
- **A long press also logs it:** `servo_reset_angle()` → `servo_set_angle(0)` while already at 0. That one is harmless.

## Fix (not applied yet — the user edits manually)

Store the **clamped** angle as the position. Skip the duty write only when the clamped angle didn't change.

```c
static bool servo_update_status(servo_t *s, int32_t cmd_angle_deg)
{
    servo_status_t *status = &s->status;
    int32_t achieved = val_clamp(cmd_angle_deg, s->cfg.min_deg, s->cfg.max_deg);

    status->deviation_deg     = cmd_angle_deg - achieved;      // != 0: tried to turn past a limit
    status->buzzer_error_flag = (status->deviation_deg != 0);  // was inverted (abs(...) == 0)
    status->angle_deg         = achieved;                      // position = clamped -> reversing works at once

    if (achieved == status->achieved_angle_deg) {
        return false;                                          // at a limit: duty already correct
    }
    status->achieved_angle_deg = achieved;
    status->duty_ticks         = servo_angle_to_duty_ticks(s, achieved);   // from the CLAMPED angle
    status->pulse_width_ms     = duty_to_pulse_width_ms(s, status->duty_ticks);
    status->angle_dev_pcnt     = 0.0f;                         // % of a limit angle was meaningless
    return true;
}
```

Follow-ups in the same area:
1. `servo_set_angle()`: a `false` return now means "at a limit". Change `ESP_LOGI("status update error…")` to `ESP_LOGD(TAG, "at limit, %" PRId32 " deg", …)`.
2. `servo_init()`: seed the status with the init position: `s->status.angle_deg = s->status.achieved_angle_deg = cfg->init_deg;` plus `duty_ticks`. It only works today because `SERVO_INIT_DEG` = `SERVO_MIN_DEG` = 0 and the struct is zeroed.
3. `main.c` step 4: `if (servo_updated)` beeps on **every** move. The spec wants a beep only when turning past a limit, so use `if (ticks != 0 && servo.status.buzzer_error_flag)`.
4. `main.c` periodic log: `ESP_LOGI(TAG, "SPEED_K=%" PRId32, "button pressed %d, …", speed_k, pressed, button_event)` passes the second string literal as the `%PRId32` argument. Merge it into one format string.

Expected log after the fix:
- From 0°, one CW click → 10°.
- At 180°, extra CW clicks stay at 180 (beep, no "error").
- One CCW click → 170° immediately.

## Source files involved

Repo: `beetroot/assignments/3_Module_ADC/ESP32S3/` (ESP-IDF v6.1, target esp32s3; `main/CMakeLists.txt` builds mini-project 3)

| File | Role in the bug |
|---|---|
| `components/servo/servo.c` | `servo_set_angle()` → `servo_update_status()`: the faulty comparison, unclamped `angle_deg`, inverted buzzer flag |
| `components/servo/include/servo.h` | `servo_config_t` (limits, pulse range), `servo_status_t` (`angle_deg` float, `achieved_angle_deg`) |
| `main/miniproject_3/src/main.c` | superloop; step 3 computes `cmd = status.angle_deg + ticks * 5 * k`; step 4 buzzer condition; log line |
| `components/encoder/encoder.c` | `encoder_update_ticks()`: change since last read; X4 divides by `counts_per_detent` and keeps the remainder (`residual`), returning ±1 per click; logs `COUNT INCREASED/DECREASED BY n` (raw counts) |
| `main/miniproject_3/include/app_config.h` | all pins and settings (below) |
| `main/miniproject_3/src/speed.c` | `k = 2k (mod N)`, N = 3 → k alternates 2, 1 |
| `main/CMakeLists.txt` | `option(ENCODER_X4 …)` → `-DENCODER_QUADRATURE_X4=1` (`idf.py -DENCODER_X4=ON build`) |
| `main/miniproject_3/docs/debouncing.md` | debugging log, RC-filter experiment table, video/.sal links |

## Configuration (`app_config.h`, as of 2026-11-18)

```c
/* Pins */
#define ENCODER_GPIO_A          4       // CLK: counted edges
#define ENCODER_GPIO_B          5       // DT:  direction level
#define ENCODER_GPIO_SW         6       // SW:  push button, active LOW
#define SERVO_GPIO              7
#define BUZZER_GPIO             16
#define ENCODER_DEBUG_GPIO      15      // HIGH while the filtered PCNT count is moving

/* Superloop */
#define SUPERLOOP_DELAY_MS      50
#define LOG_PERIOD_MS           2000

/* Encoder */
#define ENCODER_QUADRATURE_X4   0       // 0 = X1 (1 count/click), 1 = X4 (4 counts/click); CMake option ENCODER_X4
#define ENCODER_COUNTS_PER_DETENT  (X4 ? 4 : 1)
#define GLITCH_FILTER_DELAY_NS  1000    // was 10000 earlier in the project
#define ENCODER_PCNT_HIGH_LIMIT 32767
#define ENCODER_PCNT_LOW_LIMIT  -32768

/* Button */
#define DEBOUNCE_MS             30u
#define SHORT_TIME_LIMIT        800u    // longer hold = long press -> servo_reset_angle()

/* Servo: LEDC timer 0 / channel 0, 50 Hz, 13-bit */
#define SERVO_MIN_PULSE_US      500U    // 0 deg   (was 1000 earlier)
#define SERVO_MAX_PULSE_US      2500U   // 180 deg (was 2000 earlier)
#define SERVO_RANGE_DEG         180
#define SERVO_MIN_DEG           0       // travel limits (was 10/170 earlier)
#define SERVO_MAX_DEG           180
#define SERVO_INIT_DEG          0
#define SERVO_STEP_FINE_DEG     5       // deg per click at k = 1
#define SPEED_N                 3
#define SPEED_K_INIT            2       // 10 deg per click at start

/* Buzzer: LEDC timer 1 / channel 1, 10-bit, 50 % duty, 440 Hz, 2 cycles per beep */
```

## Hardware and wiring

| ESP32-S3 DevKitC-1 | Connected to | Notes |
|---|---|---|
| GPIO4 | KY-040 CLK (A) | logic analyser D0 |
| GPIO5 | KY-040 DT (B) | logic analyser D1 |
| GPIO6 | KY-040 SW | active LOW; SW pull-up footprint on the board is **unpopulated**, so the internal pull-up is used |
| GPIO7 | SG90 servo signal | servo powered from an **external 5 V** supply, common GND |
| GPIO15 | debug probe | logic analyser D3 (`encoder_output_ticks()` mirror) |
| GPIO16 | passive buzzer | LEDC PWM |
| 3V3 / GND | KY-040 + / GND | + must be on 3V3 (ESP32-S3 pins are not 5 V-tolerant) |

- **KY-040 board:** two 10 kΩ pull-ups (CLK, DT → +) are fitted. There are no capacitor pads.
- **RC debounce (Bourns suggested filter):** for each line, 10 kΩ in series from encoder to GPIO, plus 10 nF from GPIO to GND. That gives τ_fall = 0.1 ms and τ_rise = 0.2 ms (with the board's 10 kΩ pull-up). With the external filter in place, the internal pull-ups on GPIO4/5 should be **disabled**, because ~45 kΩ against the 10 kΩ series resistor lifts LOW to ~0.6 V. The X4 init path still calls `gpio_pullup_en()`.
- **RC experiments:** R = 600 Ω–10 kΩ with C = 0.47 µF, plus C = 0.1 µF alone. Results, Saleae `.sal` captures and videos are in `docs/debouncing.md` and `output/`.
- **Earlier encoder findings:**
  - X1 decoding (rising A only) lets bounce on A's falling edge produce counts in the wrong direction.
  - X4 cancels them in +1/-1 pairs.
  - An earlier "counts only up" episode coincided with a pin mapping mismatch (4/5 vs 5/6).

## Status

- **Fixed earlier:** runaway (absolute count re-added every cycle; `encoder_update_ticks` now returns the change since the last read); X1/X4 compile flag.
- **Open:**
  - The clamp/stuck bug above, plus its follow-ups 1–4.
  - Pull-up disable for the RC filter.
  - Long serial logs in the README to move into `output/`.
