# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 2

**DATE:** 20/09/2025

**NAME OF EXP:** 8051 Tool Chain Setup – Development Board – AT89S52

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal oscillator) — Keil µVision 5 (C51), progisp + USBasp ISP programmer

---

## TASK 1 : Hardware Board Bring-Up

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

Using the board schematic as a reference, the key components on the development kit were located, the 5 V supply rail between VCC and GND was verified with a digital multimeter, the user LED connection was traced to port pin P1.0 (wired active-low), and the AT89S52 was interfaced to the host PC via the USBasp ISP programmer.

| Parameter | Specification |
|---|---|
| Microcontroller | AT89S52 |
| CPU Architecture | 8051 (MCS-51), 8-bit core, 12 oscillator periods per machine cycle |
| Supply Voltage | 5 V nominal (operating range 4.0 – 5.5 V) |
| Programming / Debug Interface | No on-chip debug hardware; flashed via SPI ISP (USBasp + progisp) |
| User LED Port / Pin | Port 1 / P1.0 |
| LED Drive Logic | Active-LOW |
| Target Blink Rate | ≈ 1 Hz |

**Pseudocode:**

1. Locate the microcontroller, ISP header, power regulator, and user LED on the board schematic.
2. Measure the supply voltage across pin 40 (VCC) and pin 20 (GND) using a multimeter.
3. Follow the PCB trace from the user LED to P1.0 and confirm its active-low configuration.
4. Set up the USBasp driver on the PC, compile the test project in Keil µVision, and upload the generated HEX file via progisp.

**Code Base:**

- Project files: `TASK2B.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK2B.hex`.
- No separate C source is required for Task 1 alone — the Task 2 firmware is flashed to verify end-to-end toolchain operation.
- `BringUp_Sheet.md` provides the record table for logging bench measurements.

### 3. Inference

The AT89S52 board was inspected, powered, and successfully flashed from the workstation, confirming that the Keil + USBasp + progisp workflow is ready for bare-metal development.

---

## TASK 2 : Bare-Metal LED Blink

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

The declaration `sbit USER_LED = P1^0;` binds `USER_LED` directly to bit 0 of the Port 1 Special Function Register (`0x90`). Because standard 8051 quasi-bidirectional ports do not require a separate data-direction register for output, writing `0` or `1` to this bit immediately drives the pin. A ~0.5 s delay is generated using a nested software loop whose inner counter is declared `volatile` so the compiler cannot optimise the loop away. At 11.0592 MHz, each outer iteration executes in roughly 2086 machine cycles (≈ 2.26 ms as profiled in uCsim), so 221 outer iterations produce a half-period of ~0.5 s.

**Pseudocode:**

1. Include the 8051 register header `reg52.h`.
2. Bind `P1^0` using `sbit` and define active-low constants (`LED_ON = 0`, `LED_OFF = 1`).
3. Implement `soft_delay()` using calibrated nested loops for an approximate 500 ms pause.
4. Drive the LED ON and call `soft_delay()`, then drive the LED OFF and call `soft_delay()`.
5. Loop indefinitely.

**Code Base:**

- `main.c` — implements `soft_delay(HALF_PERIOD_PASSES)` calibrated with `HALF_PERIOD_PASSES = 221`
- `TASK2B.uvproj`, `STARTUP.A51`

### 3. Inference

Direct bit-level SFR manipulation combined with a calibrated software delay loop toggles the on-board LED reliably at approximately 1 Hz.
