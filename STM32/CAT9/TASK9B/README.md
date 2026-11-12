# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 9

**DATE:** 28/09/2026

**NAME OF EXP:** STM32 UART Command Controlled LED

**HARDWARE SETUP / PLATFORM:** Blue Pill STM32F103C6T6 (ARM Cortex-M3, 72 MHz) + USB-to-TTL serial adapter + PuTTY — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, PuTTY

---

## TASK 9B : String-Based Application Logic

### 1. Circuit Diagram

| STM32 Pin | USB-to-TTL Pin |
|---|---|
| `PA2` (`USART2_TX`) | `RX` |
| `PA3` (`USART2_RX`) | `TX` |
| `GND` | `GND` |

`USART2` configured in asynchronous mode at **115200-8-N-1**. On-board LED on `PC13`.

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Incoming characters are echoed as the user types and accumulated in a line buffer until a carriage return (`\r`) or line feed (`\n`) is received. The completed string is stripped of leading/trailing whitespace, converted to upper-case, and matched against supported commands using `strcmp()`.

| Serial Command | Action Performed |
|---|---|
| `LED ON` / `LED OFF` / `LED TOGGLE` | Set or invert the `PC13` LED |
| `STATUS` | Print the current LED state |
| `CRLF TEST` | Run the line-ending comparison sequence |
| `HELP` | Print the list of valid commands |

| Transmitted String | Behaviour Observed in PuTTY |
|---|---|
| `"HELLO\r\n"` | Advances to column 0 of the next line |
| `"HELLO\r"` | Returns cursor to column 0 of the same line (subsequent text overwrites) |
| `"HELLO\n"` | Drops down one row without returning to column 0 |
| `"HELLO\r\nWORLD\r\n"` | Prints cleanly across two separate lines |

**Pseudocode:**

1. Turn the LED OFF and print the command prompt `> `.
2. Poll `USART2` for the next incoming byte.
3. On `\r` or `\n` (when the buffer is non-empty): NUL-terminate the string, invoke `handle_command()`, and reset the buffer index.
4. On Backspace (`\b` or `0x7F`): remove the last character from the buffer and erase it on the terminal.
5. For any printable character: append to the buffer and echo back.

**Code Base:**

- Project files: `CAT09B_String_Command_LED.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT09B_String_Command_LED.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `normalise()` — trims whitespace and converts characters to upper-case
- `handle_command()` and `crlf_demo()`

**Expected Output:**

```
> led on
OK: LED is ON
> blink
ERR: unknown command 'BLINK' (type HELP)
```

### 3. Inference

Buffering characters until a line terminator and normalising case/whitespace turns a raw UART byte stream into an interactive command-line shell for hardware control.
