# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 5

**DATE:** 10/08/2026

**NAME OF EXP:** Hardware Timer and Interrupt-Based Timing for LED Fading, Pulse Generation and Stepper Motor Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + LED + DSO + ULN2003 / 28BYJ-48 stepper — Keil µVision 5 (C51), progisp + USBasp ISP programmer, Digital Storage Oscilloscope

---

## TASK 3 : Stepper Motor Speed and Direction (optional)

### 1. Circuit Diagram

> **Note:** The schematic below combines the wiring for all CAT 5 tasks. Pin P1.1 serves as the LED / waveform output in Tasks 1 and 2, and as the ULN2003 `IN2` phase line in Task 3, so only one of those loads should be connected at a time.

11.0592 MHz crystal → 1.085 µs machine cycle → Timer 0 counter → overflow flag → **ISR** → coil phase step

| Time Base | Machine Cycles | Timer 0 Configuration |
|---|---|---|
| 1 ms | 922 | Mode 1 (16-bit), ideal `TH0:TL0 = 0xFC66` (`0xFC72` with +12-cycle ISR latency compensation), reloaded in ISR |
| 100 µs | 92 | Mode 2 (8-bit auto-reload), `TH0 = 0xA4` |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

A 1 ms Timer 0 ISR accumulates ticks up to the target step delay `step_ms` and then outputs the next 4-phase wave-drive excitation (`1000` → `0100` → `0010` → `0001`) on Port 1 pins P1.0–P1.3. Because the ISR controls coil commutation, motor speed is determined strictly by `step_ms`. Keypad assignments: key `1` selects slow speed (100 ms/step), `2` selects medium (50 ms/step), `3` selects fast (10 ms/step), `4` reverses rotation direction, and `5` toggles start/stop.

**Pseudocode:**

1. Initialise Timer 0 to generate a 1 ms periodic interrupt.
2. Inside the ISR: increment `ms_count`; once it reaches `step_ms`, step `phase` forward or backward and drive `STEP_SEQ[phase]` onto P1.0–P1.3.
3. Inside `main()`: scan the keypad and update `step_ms`, `clockwise`, or `running` accordingly.

**Code Base:**

- Project files: `TASK5C.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK5C.hex`.
- `main.c` — contains the `STEP_SEQ[]` excitation table and `keypad_get()` scanner

### 3. Inference

Driving stepper coil phases from a timer ISR decouples motor commutation from foreground keypad scanning, ensuring smooth rotation across all speed settings and directions.
