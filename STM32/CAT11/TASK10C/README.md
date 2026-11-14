# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10

**DATE:** 05/10/2026 (Tasks 10C/10d: 12/10/2026)

**NAME OF EXP:** STM32–Wemos Wi-Fi Modem Interface Using AT Commands + HTTP Server

**HARDWARE SETUP / PLATFORM:** Blue Pill STM32F103C8T6 (ARM Cortex-M3, 72 MHz) + Wemos D1 mini (ESP8266 AT firmware) + USB-to-TTL serial adapter — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, PuTTY

---

## TASK 9C : Connect Wemos to Wi-Fi

> Note: The CAT 10 lab handout labels this exercise **Task 9C**; in this repository it is located in `TASK10C`.

### 1. Circuit Diagram

| STM32 Pin | Target Connection | Role |
|---|---|---|
| `PA9` (`USART1_TX`) | Wemos `RX` | Transmit AT commands to ESP8266 |
| `PA10` (`USART1_RX`) | Wemos `TX` | Receive AT responses from ESP8266 |
| `PA2` (`USART2_TX`) / `PA3` (`USART2_RX`) | USB-to-TTL `RX` / `TX` | Debug console to PC (PuTTY) |
| `GND` | Shared ground rail | Common reference |

Both `USART1` and `USART2` operate at **115200-8-N-1**. Wi-Fi network credentials are read from `Core/Inc/wifi_secrets.h` (git-ignored; see `wifi_config.h` for template macros).

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

![Data flow](dataflow.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The STM32 configures the ESP8266 for Station mode (`AT+CWMODE=1`), joins the configured access point (`AT+CWJAP`), and queries `AT+CIFSR` to extract the assigned IPv4 and MAC addresses from the `STAIP,"…"` and `STAMAC,"…"` response tokens. To verify outbound connectivity, it also opens a TCP socket to `example.com` via `AT+CIPSTART`, sends an HTTP `GET` request with `AT+CIPSEND`, and logs the returned HTTP status line.

**Pseudocode:**

1. Initialise the ESP8266 modem and disable command echo.
2. Set Station mode with `AT+CWMODE=1`.
3. Join the Wi-Fi network using `AT+CWJAP` (allowing up to a 20 s association timeout).
4. Issue `AT+CIFSR`, parse out the station IP and MAC strings, and print them on `USART2`.
5. Run the TCP client check: `AT+CIPSTART` → `AT+CIPSEND` → wait for `>` prompt → send HTTP request → print `HTTP/1.1 200 OK`.

**Code Base:**

- Project files: `CAT10C_Connect_Wemos_to_WiFi.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT10C_Connect_Wemos_to_WiFi.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `extract_after()` — string token extractor for `STAIP` and `STAMAC`
- `esp_wait_prompt()` and `tcp_demo()`

**Expected Output:**

```
Connected to Wifi Successfully
IP Address : 192.168.x.x
MAC Address: xx:xx:xx:xx:xx:xx
Server replied: HTTP/1.1 200 OK
```

### 3. Inference

The STM32 autonomously associates the ESP8266 with a Wi-Fi access point, parses its network address, and completes an outbound TCP/HTTP transaction using `CIPSTART` and `CIPSEND`.
