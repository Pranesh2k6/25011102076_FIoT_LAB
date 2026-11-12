# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 5

**DATE:** 10/08/2026

**NAME OF EXP:** Hardware Timer and Interrupt-Based Timing for LED Fading, Pulse Generation and Stepper Motor Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + LED + DSO + ULN2003 / 28BYJ-48 stepper — Keil µVision 5 (C51), progisp + USBasp ISP programmer, Digital Storage Oscilloscope

---

## TASK 2A : Software PWM LED Fading

### 1. Circuit Diagram

> **Note:** The schematic below combines the wiring for all CAT 5 tasks. Pin P1.1 serves as the LED / waveform output in Tasks 1 and 2, and as the ULN2003 `IN2` phase line in Task 3, so only one of those loads should be connected at a time.

11.0592 MHz crystal → 1.085 µs machine cycle → Timer 0 counter → overflow flag → **ISR** → software tick counter → PWM output

| Time Base | Machine Cycles | Timer 0 Configuration |
|---|---|---|
| 1 ms | 922 | Mode 1 (16-bit), ideal `TH0:TL0 = 0xFC66` (`0xFC72` with +12-cycle ISR latency compensation), reloaded in ISR |
| 100 µs | 92 | Mode 2 (8-bit auto-reload), `TH0 = 0xA4` |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Timer 0 operates in 8-bit auto-reload Mode 2 to fire an interrupt every 100 µs. Ten consecutive 100 µs slots form a 1 ms PWM frame (1 kHz carrier frequency, well above visible flicker). The variable `duty` (ranging from 0 to 10) determines how many slots out of 10 keep the LED active. Every 100 ms (1000 interrupts), `duty` steps by 10%, ramping brightness smoothly from 0% to 100% and back down over a 2-second cycle.

**Pseudocode:**

1. Configure Timer 0 in Mode 2 with `TH0 = TL0 = 0xA4` (100 µs auto-reload).
2. Inside the ISR: increment `pwm_slot` modulo 10, drive the LED active while `pwm_slot < duty`, and raise `step_flag` every 1000 interrupts (100 ms).
3. Inside `main()`: whenever `step_flag` is set, adjust `duty` by `direction` and flip `direction` at the 0 and 10 boundaries.

**Code Base:**

- Project files: `TASK5B_1.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK5B_1.hex`.
- `main.c` — combines the 1 kHz software PWM generator and 100 ms fade ramp inside a compact Timer 0 ISR

### 3. Inference

Modulating the on-time duty cycle across a fixed 1 kHz interrupt-driven carrier produces smooth, flicker-free LED breathing on a microcontroller without dedicated PWM hardware.
