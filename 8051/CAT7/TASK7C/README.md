# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 7

**DATE:** 24/08/2026

**NAME OF EXP:** 16×2 LCD Driver Development and Dynamic Display Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + HD44780 16×2 character LCD + 4×4 matrix keypad — Keil µVision 5 (C51), progisp + USBasp ISP programmer, HD44780 datasheet

---

## TASK 7C : 4×4 Keypad Character Movement

### 1. Circuit Diagram

| LCD Signal | 8051 Pin | Function |
|---|---|---|
| `RS` | P3.2 | Register select (`0` = instruction, `1` = data) |
| `E` | P3.3 | Enable strobe (latches nibble on falling edge) |
| `D4`–`D7` | P3.4–P3.7 | 4-bit parallel data bus |
| `RW` | `GND` | Tied low for write-only operation |
| `V0` | 10 kΩ potentiometer | Display contrast adjustment |

Keypad connections: rows on P2.7–P2.4, columns on P2.3–P2.0.

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

An asterisk (`'*'`) is initially placed at coordinates `(0, 0)`. Pressing directional keys `2` (up), `8` (down), `4` (left), or `6` (right) moves the marker within the valid bounds of rows `0`–`1` and columns `0`–`15`, while key `5` resets it back to `(0, 0)`. On each valid press, the previous cell is overwritten with a blank space `' '`, the marker is drawn at the updated coordinates, and the routine waits for key release so each press produces a single one-cell step.

**Pseudocode:**

1. Initialise the LCD and render `'*'` at `(0, 0)`.
2. Scan the 4×4 keypad; if no key is pressed, continue polling.
3. Overwrite the current `(row, col)` cell with `' '`.
4. Compute the new `(row, col)` coordinates clamped to `[0..1]` and `[0..15]`.
5. Draw `'*'` at the new location and wait until the key is released.

**Code Base:**

- Project files: `TASK7C.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK7C.hex`.
- `keypad_scan()` — 4×4 row-column scanner backed by `KEYMAP[4][4]`
- 4-bit HD44780 LCD driver from Task 7A

### 3. Inference

Coupling 4×4 matrix keypad scanning with targeted DDRAM cell updates creates an interactive cursor-navigation interface on the 16×2 LCD.
