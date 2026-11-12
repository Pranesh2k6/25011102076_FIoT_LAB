# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 5

**DATE:** 10/08/2026

**NAME OF EXP:** Hardware Timer and Interrupt-Based Timing for LED Fading, Pulse Generation and Stepper Motor Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + LED + DSO + ULN2003 / 28BYJ-48 stepper — Keil µVision 5 (C51), progisp + USBasp ISP programmer, Digital Storage Oscilloscope

---

## TASK 2B : Variable-Frequency Square Wave

### 1. Circuit Diagram

> **Note:** The schematic below combines the wiring for all CAT 5 tasks. Pin P1.1 serves as the LED / waveform output in Tasks 1 and 2, and as the ULN2003 `IN2` phase line in Task 3, so only one of those loads should be connected at a time.

11.0592 MHz crystal → 1.085 µs machine cycle → Timer 0 counter → overflow flag → **ISR** → pin toggle

| Time Base | Machine Cycles | Timer 0 Configuration |
|---|---|---|
| 1 ms | 922 | Mode 1 (16-bit), ideal `TH0:TL0 = 0xFC66` (`0xFC72` with +12-cycle ISR latency compensation), reloaded in ISR |
| 100 µs | 92 | Mode 2 (8-bit auto-reload), `TH0 = 0xA4` |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Each Timer 0 overflow inverts P1.1, so the generated square-wave frequency is governed solely by the 16-bit half-period machine-cycle count `N = 460800 / f`:

| Mode | Target Frequency `f` | Half-Period Counts `N` | Ideal Reload Value |
|---|---|---|---|
| 1 | 100 Hz | 4608 | `0xEE00` |
| 2 | 500 Hz | 922 | `0xFC66` |
| 3 | 1 kHz | 461 | `0xFE33` |
| 4 | 2 kHz | 230 | `0xFF1A` |
| 5 | 5 kHz | 92 | `0xFFA4` |

While the table lists the theoretical reload values, the firmware adds `ISR_LATENCY` (12 cycles) to compensate for the cycles elapsed before the ISR reloads the timer registers: `reload = 65536 − N + 12`. Whenever the user presses the button to change modes, Timer 0 is halted, loaded with the new 16-bit preload, and restarted immediately. Simulated in uCsim, Mode 1 measures 100.2 Hz.

**Pseudocode:**

1. Calculate `reload = 65536 − N + ISR_LATENCY` for the selected frequency mode.
2. Inside the Timer 0 ISR: reload `TH0:TL0` and toggle output pin `P1.1`.
3. On each button press: advance to the next frequency mode and display the mode number (`1`–`5`) on the 7-segment indicator.

**Code Base:**

- Project files: `TASK5B_2.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK5B_2.hex`.
- `main.c` — defines the `HALF_PERIOD[]` look-up table and `set_mode()` reload logic

### 3. Inference

A single Timer 0 ISR generates all five square-wave frequencies from 100 Hz to 5 kHz simply by updating the 16-bit timer preload value on each button press.
