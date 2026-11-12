# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 6

**DATE:** 17/08/2026

**NAME OF EXP:** UART Driver and Serial Debugging

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + USB-to-TTL serial adapter + PuTTY — Keil µVision 5 (C51), progisp + USBasp ISP programmer, PuTTY

---

## TASK A : UART Initialisation and Character Transmission

### 1. Circuit Diagram

| 8051 Pin | USB-to-TTL Pin |
|---|---|
| P3.1 (`TXD`) | `RX` |
| P3.0 (`RXD`) | `TX` |
| `GND` | `GND` |

PuTTY terminal configuration: **9600 baud, 8 data bits, no parity, 1 stop bit (9600-8-N-1)**.

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

`uart_init()` configures the serial port in 8-bit variable-baud UART mode (`SCON = 0x50`) and sets Timer 1 to 8-bit auto-reload Mode 2 with `TH1 = 0xFD` before starting the timer. To send a byte, `uart_putc()` loads `SBUF`, polls the transmit-complete flag `TI` until hardware sets it, and clears `TI` in software. Upon boot, the firmware sends a single `'U'` (`0x55`, alternating bit pattern) as a framing check, then continuously transmits the greeting string.

**Pseudocode:**

1. Call `uart_init()` to set up `SCON` and Timer 1 for 9600 baud.
2. Transmit the test character `'U'` followed by `\r\n`.
3. In an infinite loop: transmit `"Hello from 8051\r\n"` and pause for 0.5 s.

**Code Base:**

- Project files: `TASK6A.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK6A.hex`.
- `main.c` — implements `uart_init()`, `uart_putc()`, and `uart_puts()`

**Expected Output:**

```
U
Hello from 8051
Hello from 8051
...
```

**Hardware Register Summary:**

| Aspect | Register / Formula |
|---|---|
| UART I/O pins | `TXD` on P3.1, `RXD` on P3.0 (Port 3 alternate functions) |
| Serial control | `SCON`: Mode 1 (8-bit UART, variable baud) + `REN = 1` → `SCON = 0x50` |
| Data buffer | `SBUF` (physically separate transmit and receive registers sharing one SFR address) |
| Transmit flag | `TI` is asserted by hardware at the stop bit and must be cleared in software |
| Baud clock source | Timer 1 operating in Mode 2 (8-bit auto-reload) |
| 9600 baud reload | `11059200 / (384 × (256 − TH1))` → `TH1 = 0xFD` |
| Receive flag | `RI` is asserted when a complete frame arrives in `SBUF` |

### 3. Inference

Readable text in PuTTY verifies that the 8051 UART frame format, Timer 1 baud-rate reload value (`0xFD`), and transmit polling routine are properly configured.
