# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 7

**DATE:** 24/08/2026

**NAME OF EXP:** 16×2 LCD Driver Development and Dynamic Display Control

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + HD44780 16×2 character LCD + 4×4 matrix keypad — Keil µVision 5 (C51), progisp + USBasp ISP programmer, HD44780 datasheet

---

## TASK 7A : LCD Initialisation and Character / String Display

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

`lcd_init()` implements the power-on reset sequence from the HD44780 datasheet for 4-bit mode: wait > 15 ms after VCC rises, strobe nibble `0x3` three times, strobe `0x2` to switch into 4-bit interface mode, and then issue full 8-bit commands as high-nibble / low-nibble pairs: Function Set `0x28` (4-bit bus, 2 display lines, 5×8 dot font), Display Control `0x0C` (display on, cursor off), Entry Mode `0x06` (auto-increment cursor), and Clear Display `0x01`. Clear and Return Home commands wait 2 ms; standard instructions and character writes wait 50 µs.

**Pseudocode:**

1. Execute `lcd_init()` to configure the HD44780 controller in 4-bit mode.
2. Set DDRAM address `0x80` via `lcd_cmd(0x80)` and write `'A'` using `lcd_data('A')`.
3. Position the cursor at row 0, column 2 with `lcd_goto(0, 2)` and print `"CAT7 LCD TEST"`.
4. Position the cursor at row 1, column 0 with `lcd_goto(1, 0)` and print `"8051 4-BIT MODE"`.

**Code Base:**

- Project files: `TASK7A.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK7A.hex`.
- `lcd.h` — HD44780 instruction opcodes and driver prototypes
- `lcd.c` — nibble strobe helper, `lcd_init()`, `lcd_cmd()`, and `lcd_data()`
- `main.c`

**Expected Output:**

```
A CAT7 LCD TEST
8051 4-BIT MODE
```

### 3. Inference

Following the HD44780 4-bit initialisation timing sequence and instruction opcodes drives both rows of the 16×2 LCD using only six microcontroller GPIO pins.
