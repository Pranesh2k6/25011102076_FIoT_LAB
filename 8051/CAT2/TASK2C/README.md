# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 2

**DATE:** 20/09/2025

**NAME OF EXP:** 8051 Tool Chain Setup – Development Board – AT89S52

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal oscillator) — Keil µVision 5 (C51), progisp + USBasp ISP programmer

---

## TASK 3 : Custom Library Function for Delay

### 1. Circuit Diagram

| Signal | Pin | Description |
|---|---|---|
| User LED | P1.0 | On-board indicator, **active-LOW** (`0` = ON) |
| ISP Header | P1.5 MOSI, P1.6 MISO, P1.7 SCK, RST | 10-pin USBasp programming interface |
| Power Supply | VCC (pin 40), GND (pin 20) | Regulated 5 V rail |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The software delay routine is factored out into a reusable module (`delay.h` and `delay.c`). Calling `delay_ms(n)` executes a calibrated 1 ms inner block `n` times: each millisecond consists of two sequential 50-iteration `volatile` loops (9 machine cycles per iteration, preventing dead-code elimination) plus loop setup overhead, totalling `22 + 18 × 50 = 922` machine cycles ≈ 1.000 ms at 11.0592 MHz (verified in the uCsim simulator; Keil C51 code generation may vary by a few percent). A short `delay_50us()` helper is also provided for peripheral strobe timing in later LCD experiments.

**Pseudocode:**

1. Expose the `delay_ms()` and `delay_50us()` function prototypes in `delay.h`.
2. Define the cycle-calibrated `volatile` delay loops inside `delay.c`.
3. Include `delay.h` in `main.c`.
4. Turn the LED ON, call `delay_ms(500)`, turn the LED OFF, and call `delay_ms(500)`.
5. Repeat continuously inside the main loop.

**Code Base:**

- Project files: `TASK2C.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK2C.hex`.
- `delay.h` — public function declarations and cycle-timing documentation
- `delay.c` — calibrated millisecond and microsecond busy-wait implementations
- `main.c` — 1 Hz LED blink application using `delay_ms(BLINK_HALF_MS)`

### 3. Inference

Encapsulating the timing loops into a dedicated `delay` module allows application code to specify delays directly in milliseconds and provides a reusable timing library for CAT 3 through CAT 7.
