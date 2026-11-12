# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 9

**DATE:** 28/09/2026

**NAME OF EXP:** STM32 UART Command Controlled LED

**HARDWARE SETUP / PLATFORM:** Blue Pill STM32F103C6T6 (ARM Cortex-M3, 72 MHz) + USB-to-TTL serial adapter + PuTTY — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, PuTTY

---

## TASK 9A : Basic UART Communication

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

`HAL_UART_Receive()` polls `USART2` with a short 10 ms timeout so the main loop stays responsive. Whenever a byte arrives from the terminal, the MCU echoes it back immediately; if the serial line remains idle for 1 s (tracked via `HAL_GetTick()`), the MCU transmits a heartbeat character `'A'`.

**Pseudocode:**

1. Send the startup banner over `USART2`.
2. Poll `HAL_UART_Receive()` for 1 byte with a 10 ms timeout.
3. If a byte is received: echo it back (expanding `\r` to `\r\n`) and update the activity timestamp.
4. Otherwise, if 1000 ms have elapsed since the last activity, transmit `'A'` and update the timestamp.

**Code Base:**

- Project files: `CAT09A_Basic_UART_Communication.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT09A_Basic_UART_Communication.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `uart_send_str()` and `uart_send_char()`

**Expected Output:**

```
CAT9A - STM32 Blue Pill UART ready (115200-8-N-1)
AAAAhelloAAA
```

### 3. Inference

Periodic `'A'` beacon characters and immediate keyboard echoing verify full-duplex `USART2` communication at 115200-8-N-1.
