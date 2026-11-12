# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 5

**DATE:** 10/08/2026

**NAME OF EXP:** Hardware Timer and Interrupt-Based Timing for LED Fading, Pulse Generation and Stepper Motor Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + LED + DSO + ULN2003 / 28BYJ-48 stepper — Keil µVision 5 (C51), progisp + USBasp ISP programmer, Digital Storage Oscilloscope

---

## TASK 1 : Timer 0 Interrupt, 1 s Software Time Base

### 1. Circuit Diagram

> **Note:** The schematic below combines the wiring for all CAT 5 tasks. Pin P1.1 serves as the LED / waveform output in Tasks 1 and 2, and as the ULN2003 `IN2` phase line in Task 3, so only one of those loads should be connected at a time.

11.0592 MHz crystal → 1.085 µs machine cycle → Timer 0 counter → overflow flag → **ISR** → software tick counter → LED toggle

| Time Base | Machine Cycles | Timer 0 Configuration |
|---|---|---|
| 1 ms | 922 | Mode 1 (16-bit), ideal `TH0:TL0 = 0xFC66` (`0xFC72` with +12-cycle ISR latency compensation), reloaded in ISR |
| 100 µs | 92 | Mode 2 (8-bit auto-reload), `TH0 = 0xA4` |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Timer 0 runs in 16-bit Mode 1 and triggers Interrupt Vector 1 (switching to register bank 1) every millisecond. Inside the ISR, `TH0:TL0` is reloaded with `0xFC72` (`0xFC66` plus 12 cycles to account for interrupt vectoring and reload overhead) and a millisecond counter is incremented. Once 1000 ticks have elapsed, the ISR raises `second_flag`, which the main loop detects and clears before toggling the LED—eliminating busy-wait loops altogether.

**Pseudocode:**

1. Configure `TMOD` for Timer 0 Mode 1, preload `0xFC72`, and enable `ET0`, `EA`, and `TR0`.
2. In the Timer 0 ISR: reload `0xFC72`, increment `ms_ticks`, and set `second_flag` every 1000 ticks.
3. In `main()`: monitor `second_flag`; when set, clear the flag and invert `P1.1`.

**Code Base:**

- Project files: `TASK5A.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK5A.hex`.
- `main.c` — defines `timer0_isr() interrupt 1 using 1` and `timer0_init_1ms()`

### 3. Inference

Delegating timekeeping to Timer 0 hardware interrupts produces an exact 1-second toggle interval while leaving the CPU main loop completely unblocked.
