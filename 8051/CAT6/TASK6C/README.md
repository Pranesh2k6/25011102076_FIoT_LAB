# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 6

**DATE:** 17/08/2026

**NAME OF EXP:** UART Driver and Serial Debugging

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + USB-to-TTL serial adapter + PuTTY — Keil µVision 5 (C51), progisp + USBasp ISP programmer, PuTTY

---

## TASK C : Interrupt-Driven RX Ring Buffer

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

The UART serial ISR (Interrupt Vector 4) services incoming characters asynchronously, pushing each byte from `SBUF` into a 16-element circular ring buffer (15 usable slots, leaving one slot empty to distinguish full from empty, with `head` updated by the ISR and `tail` consumed by `main`) while also clearing `TI` when transmissions complete. This frees the foreground loop to maintain a background LED heartbeat while assembling incoming characters into a line buffer until Enter is pressed.

**Pseudocode:**

1. Initialise the UART and enable serial and global interrupts (`ES = 1`, `EA = 1`).
2. Inside the serial ISR: if `RI` is set, push `SBUF` into the ring buffer; if `TI` is set, clear `tx_busy`.
3. In `main()`: drain available bytes from the ring buffer, echo each character, and append it to `line`.
4. When Enter (`\r` or `\n`) is received, output `Got N chars: <line>`.
5. Continuously toggle the heartbeat LED in the foreground loop.

**Code Base:**

- Project files: `TASK6C.uvproj` (Keil µVision project), `STARTUP.A51`, compiled binary `Objects/TASK6C.hex`.
- `uart.c` / `uart.h` — serial ISR, 16-byte ring buffer, `uart_available()`, and `uart_getc()`
- `main.c` — line assembly, echo, and background LED heartbeat

**Expected Output:**

```
> hello
Got 5 chars: hello
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

Buffering incoming bytes inside the serial ISR allows the main program to stay occupied for up to 15 character periods (~15 ms at 9600 baud) without dropping characters; if the 16-byte ring fills up, any excess bytes are safely discarded and counted via `uart_overflows()`.
