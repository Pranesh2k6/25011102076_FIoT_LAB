# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 14

**DATE:** 27/10/2026

**NAME OF EXP:** BME280 Sensor Interfacing Using I²C and MQTT using Custom Built ESP8266 API

**HARDWARE SETUP / PLATFORM:** NUCLEO-F401RE (STM32F401RET6, ARM Cortex-M4F, 84 MHz) + Bosch BME280 + Wemos D1 mini — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, ESPLib API (clean-room implementation), Node-RED, `broker.emqx.io`

---

## SUBSCRIBE EXAMPLE : LED Control

### 1. Circuit Diagram

Uses the same hardware connections as CAT 13, with `LD2` (`PA5`) additionally used in CAT 14B. Connects to MQTT broker **`broker.emqx.io:1883`**.

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

![Data flow](dataflow.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Once `CONNACK` is received, the Nucleo subscribes to `pranesh/stm32/led` and verifies the broker's `SUBACK`. Calling `MQTTDataHandler()` in the main loop returns `1` whenever a complete, validated `PUBLISH` packet arrives. Exact payload strings `"ON"`, `"OFF"`, and `"TOGGLE"` switch `LD2` (`PA5`), while any unrecognised payload is ignored. A `PINGREQ` heartbeat is sent every 60 s to keep the broker session open; if `PINGRESP` times out, the client automatically reconnects and resubscribes. `MQTTTimer()` remains hooked into `SysTick_Handler` for compatibility with the lab handout, though packet framing no longer relies on silence timeouts.

> **Security note:** Because `broker.emqx.io` is a public test broker, any client that knows the topic name can publish commands to it. For deployments beyond a classroom demonstration, use an authenticated broker by defining `MQTT_USERNAME` and `MQTT_PASSWORD` in `wifi_secrets.h` so `CONNECT` includes credentials.

**Pseudocode:**

1. Initialise `LD2`, run `mqtt_start()`, subscribe via `MQTT_subscribeTopic()`, and ensure `HAL_UART_Receive_IT()` is active.
2. In the main loop: call `MQTTDataHandler()`; when a message arrives, log the topic/payload and update `LD2`.
3. Every 60 s, transmit a keep-alive ping via `MQTT_pingReq()`.

**Code Base:**

- Project files: `CAT14B_MQTT_Subscribe_LED_Control.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT14B_MQTT_Subscribe_LED_Control.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `subscribe_commands()` — sends `SUBSCRIBE`, validates `SUBACK`, and handles automatic reconnection/resubscription
- `handle_message()`

**ESPLib API Reference:**

| ESPLib Call | Underlying AT / MQTT Traffic |
|---|---|
| `ESP8266_Init(StationMode, huart1)` | Sends `AT` and `AT+CWMODE=1` |
| `ESP8266_wifiConnect()` | Sends `AT+CWQAP` and `AT+CWJAP` |
| `ESP8266_portConnect("TCP", broker, "1883")` | Opens TCP socket via `AT+CIPSTART` |
| `MQTT_connectBroker()` | Sends MQTT `CONNECT` (`0x10`, protocol `"MQTT"` level 4, clean session, keep-alive, client ID) |
| `MQTT_publishTopic()` | Sends MQTT `PUBLISH` (`0x30`, topic length + string, payload) |
| `MQTT_subscribeTopic()` | Sends MQTT `SUBSCRIBE` (`0x82`) |

**Clean-Room Library Improvements:** `ESPLib.h` preserves the exact function names from the lab handout so student application code compiles without changes, while `ESPLib.c` and `mqtt_packet.c` provide a new implementation that resolves the flaws of the original course driver:

| Limitation in Original Driver | Behaviour in Reworked Implementation |
|---|---|
| Aborted the UART RX interrupt on every transmit, dropping any broker packets that arrived mid-send | Arms the UART RX interrupt once at startup into a 1 KB ring buffer and never aborts it |
| Inferred packet boundaries from 100 ms of UART silence; left `colonIndex` uninitialised on `SUBACK` / `PINGRESP` frames | Strips `+IPD,<len>:` framing first and feeds payload bytes into a stateful MQTT parser driven by packet type and Remaining Length |
| Handled only 1-byte Remaining Length without bounds checks or guaranteed NUL termination | Decodes variable-length Remaining Length (up to 4 bytes), validates buffer capacities prior to copying, guarantees NUL-terminated strings, and skips oversized packets |
| Ignored `CONNACK` / `SUBACK` responses and used unbounded `do {} while` loops that could hang indefinitely | Verifies `CONNACK` acceptance and `SUBACK ≠ 0x80`; enforces timeouts on every wait and returns `funcOk` / `funcErr` |

Additional safeguards: `PINGREQ` verifies `PINGRESP` (reconnecting if the broker fails to respond), oversized topics are dropped and counted, and the firmware logs RX-overflow or dropped-packet counters whenever they increment.

The client operates strictly at **QoS 0**: publish or subscribe calls requesting QoS 1/2 return `funcErr`, `SUBACK` must grant QoS 0, and any incoming QoS 1/2 publish is discarded and counted. `UNSUBSCRIBE` verifies the matching `UNSUBACK`.

If a UART ring-buffer overflow or malformed `+IPD` header occurs, all buffered bytes are flushed, the parser resets, and both the TCP socket and MQTT session are marked disconnected to force a clean reconnect. If `HAL_UART_Receive_IT` ever fails to re-arm, the main loop retries automatically. `CONNACK` Session Present flags are validated and wildcard publish topics are rejected.

### 3. Inference

Subscribing to an MQTT command topic and parsing incoming `PUBLISH` frames allows remote clients such as Node-RED to control the Nucleo's on-board LED in real time.
