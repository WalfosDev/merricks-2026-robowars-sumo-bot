# Sumo Bot — ESP32 / ESP-IDF

An autonomous sumo robot built on the ESP32 using the ESP-IDF framework. The robot uses a ring-edge line detector, a directional IR sensor array, and dual BDC motors to compete in sumo matches — searching for opponents, attacking them, and evading the ring boundary — all driven by a priority-based FreeRTOS state machine.

---

## Features

- **5-state autonomous state machine** with hardware-interrupt-driven priority overrides
- **Directional IR opponent detection** across 9 compass directions
- **Ring-edge line detection** via QRE1113 reflective sensors with ISR-driven response
- **Dual BDC motor control** with PWM arc turns, hard pivots, and speed ramping via the ESP-IDF `bdc_motor` component
- **Start sequence choreography** configurable as a timed action table
- **Match start/stop control** via JSumo-compatible hardware start module

---

## Hardware

| Component | Part | Interface |
|---|---|---|
| Microcontroller | ESP32 | — |
| Motor Driver | BTS7960 (×2) | MCPWM |
| Line Sensor | QRE1113 Reflective (×2) | GPIO (interrupt) |
| IR Opponent Sensor | IR Array | ADC (stubbed) |
| Start Module | JSumo-compatible switch | GPIO (interrupt) |

---

## Schematic

<!-- INSERT SCHEMATIC HERE -->

&nbsp;

---

## Pin Assignments

| Signal | GPIO |
|---|---|
| Right Motor PWM A | 32 |
| Right Motor PWM B | 27 |
| Left Motor PWM A | 26 |
| Left Motor PWM B | 25 |
| Line Sensor 1 | 34 |
| Line Sensor 2 | 35 |
| JSumo Start Module | 4 |

---

## Architecture

### State Machine

The main loop runs as a FreeRTOS task and steps through five states. Hardware events delivered via FreeRTOS queues override the current state with strict priority ordering.

```
Priority (high → low):
  1. JSumo Module    →  ON  forces START_SEQUENCE | OFF forces STOP
  2. Line Detector   →  LINE_DETECTED forces EVADE | NO_LINE resumes SEARCH
  3. IR Array        →  drives SEARCH ↔ ATTACK transitions
```

```
         ┌─────────┐
  ON ──► │  START  │ ──────────────────────────────────────────────────┐
         │SEQUENCE │                                                    │
         └─────────┘                                                    │
                                                                        ▼
         ┌────────┐   opponent seen    ┌────────┐   line detected   ┌──────┐
  ──────►│ SEARCH │ ────────────────► │ ATTACK │ ──────────────────►│EVADE │
         │        │ ◄──────────────── │        │ ◄──────────────────│      │
         └────────┘   target lost     └────────┘   no line          └──────┘
              ▲                                                          │
              └──────────────────────────────────────────────────────────┘
                                   no line

  OFF ──► STOP  (blocks until JSumo signals ON)
```

### Components

```
sumo-bot-idf/
├── main/
│   ├── main.c              # FreeRTOS task, state machine, event dispatch
│   └── main.h              # State types, start sequence definition
└── components/
    ├── motor_controll/     # BDC motor init and drive-state LUT
    ├── line_detector/      # QRE1113 ISR → FreeRTOS queue
    ├── ir_array/           # IR opponent sensor sampling → direction enum
    ├── ir_to_drive/        # Maps ir_array_direction_t → drive_state_t
    └── jsumo_module/       # JSumo start switch ISR → FreeRTOS queue
```

#### `motor_controll`
Wraps the ESP-IDF `bdc_motor` library. Exposes `motor_controll_apply_drive_state(drive_state_t)` which applies a pre-computed LUT entry for both motors. No-ops if the state hasn't changed to avoid redundant MCPWM writes.

Drive states: `M_STOP`, `M_F`, `M_B`, `M_L`, `M_R`, `M_FL`, `M_FR`, `M_BL`, `M_BR`

#### `line_detector`
Configures GPIO 34 and 35 with any-edge interrupts. The ISR reads both pins and overwrites a depth-1 FreeRTOS queue with `LINE_DETECTED` or `NO_LINE`. Returns the queue handle to `main` on init.

#### `ir_array`
Polls IR sensors on a 500 µs `esp_timer` ISR and posts the detected `ir_array_direction_t` to a queue. Direction covers 9 positions: `IR_W` through `IR_E` (compass notation) plus `none`. ADC integration is currently stubbed and always returns `IR_N` pending hardware bring-up.

#### `ir_to_drive`
Pure mapping component. Two LUTs translate `ir_array_direction_t` into `drive_state_t` — one for forward-attack steering (`ir_direction_to_drive_fwd`) and one for flat in-place pivoting during evasion (`ir_direction_to_drive_flat`).

#### `jsumo_module`
Monitors GPIO 4 for the JSumo start signal via any-edge ISR. Posts `ON` or `OFF` to a queue. Receiving `ON` triggers the start sequence; `OFF` stops the robot and blocks until the next `ON`.

---

## Start Sequence

The configurable start sequence is defined in `main.h` as a table of `{duration_ms, action}` pairs:

```c
static const control_sequence_t START_SEQUENCE[] = {
    { .duration_ms = 100, .action = M_FL },   // arc left
    { .duration_ms = 200, .action = M_F  },   // full forward
    { .duration_ms = 100, .action = M_FR },   // arc right
};
```

After the sequence completes the robot enters `SEARCH`.

---

## Build & Flash

Requires [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) v5.x.

```bash
# Configure target
idf.py set-target esp32

# Build
idf.py build

# Flash and monitor
idf.py -p PORT flash monitor
```

Log output is tagged per component (`MAIN`, `MOTOR_CONTROLL`, `Line_Detector`, `IR_ARRAY`, `JSUMO_MODULE`) and uses `ESP_LOGD` for verbose debug output.

---

## Project Status

| Feature | Status |
|---|---|
| Motor control | Complete |
| Line detection | Complete |
| JSumo start module | Complete |
| State machine | Complete |
| IR opponent detection | In progress (ADC bring-up) |
