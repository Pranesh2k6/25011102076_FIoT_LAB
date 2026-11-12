# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 5

**DATE:** 10/08/2026

**NAME OF EXP:** Hardware Timer and Interrupt-Based Timing for LED Fading, Pulse Generation and Stepper Motor Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + LED + DSO + ULN2003 / 28BYJ-48 stepper — Keil µVision 5 (C51), progisp + USBasp ISP programmer, Digital Storage Oscilloscope

---

## TASK 2C : Different Signal Types

### 1. Circuit Diagram

> **Note:** The schematic below combines the wiring for all CAT 5 tasks. Pin P1.1 serves as the LED / waveform output in Tasks 1 and 2, and as the ULN2003 `IN2` phase line in Task 3, so only one of those loads should be connected at a time.

11.0592 MHz crystal → 1.085 µs machine cycle → Timer 0 counter → overflow flag → **ISR** → pattern bit output

| Time Base | Machine Cycles | Timer 0 Configuration |
|---|---|---|
| 1 ms | 922 | Mode 1 (16-bit), ideal `TH0:TL0 = 0xFC66` (`0xFC72` with +12-cycle ISR latency compensation), reloaded in ISR |
| 100 µs | 92 | Mode 2 (8-bit auto-reload), `TH0 = 0xA4` |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Each digital signal waveform is encoded as a character string of `'1'` and `'0'` bits stepped out by the 100 µs Timer 0 interrupt at 0.5 ms per bit (every 5 ticks): a 50% square wave (`10`), a variable duty-cycle wave (`11110000`), a narrow pulse train (`1000000000`), and a gated burst train (`1010101010` followed by ten `'0'`s).

**Pseudocode:**

1. Configure Timer 0 in Mode 2 for a 100 µs auto-reload period.
2. In the ISR: every 5 ticks (0.5 ms), output the next bit of the active pattern string to P1.1 and wrap at the end of the string.
3. In `main()`: poll the button and cycle through the four signal patterns on each press.

**Code Base:**

- Project files: `TASK5B_3.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK5B_3.hex`.
- `main.c` — stores the waveform table `PATTERNS[]` and implements `select_pattern()`

### 3. Inference

Representing arbitrary digital waveforms as bit-pattern tables clocked out by a fixed timer interrupt allows one unified ISR to synthesise square waves, asymmetric duty cycles, pulse trains, and burst signals.
