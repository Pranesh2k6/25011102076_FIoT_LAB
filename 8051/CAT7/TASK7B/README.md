# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 7

**DATE:** 24/08/2026

**NAME OF EXP:** 16×2 LCD Driver Development and Dynamic Display Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + HD44780 16×2 character LCD + 4×4 matrix keypad — Keil µVision 5 (C51), progisp + USBasp ISP programmer, HD44780 datasheet

---

## TASK 7B : Cursor Positioning and Dynamic Value Update

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

`lcd_goto(row, col)` computes and transmits the DDRAM address command `0x80 + (row ? 0x40 : 0x00) + col`. To display integers without `sprintf`, `lcd_print_number()` extracts decimal digits via `value % 10 + '0'` into a small buffer and writes them from most-significant to least-significant digit. Static text labels are drawn only once at startup; during runtime, only the numeric fields are cleared and overwritten every 500 ms.

**Pseudocode:**

1. Initialise the LCD and print the static labels `"Counter:"` and `"Up time:"` once.
2. Every 500 ms, clear the counter field and write the updated count at its DDRAM offset.
3. Every 1000 ms (2 ticks), clear and update the elapsed seconds field on the second row.

**Code Base:**

- Project files: `TASK7B.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK7B.hex`.
- `lcd_goto()`, `lcd_puts()`, `lcd_print_number()`, and `lcd_clear_field()`

**Expected Output:**

```
Counter:  137
Up time: 68   s
```

### 3. Inference

Direct DDRAM cursor addressing allows live numerical fields to refresh cleanly in place without clearing or flickering the static labels on the screen.
