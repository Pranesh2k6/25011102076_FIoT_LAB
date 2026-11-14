# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 14

**DATE:** 27/10/2026

**NAME OF EXP:** BME280 Sensor Interfacing Using I²C and MQTT using Custom Built ESP8266 API

**HARDWARE SETUP / PLATFORM:** NUCLEO-F401RE (STM32F401RET6, ARM Cortex-M4F, 84 MHz) + Bosch BME280 + Wemos D1 mini — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, ESPLib API (clean-room implementation), Node-RED, `broker.emqx.io`

---

## STEP 1–3 : Interface Wrappers (Publish Test)

### 1. Circuit Diagram

Uses the same hardware connections as CAT 13, with `LD2` (`PA5`) additionally used in CAT 14B. Connects to MQTT broker **`broker.emqx.io:1883`**.

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

![Data flow](dataflow.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The ESPLib driver associates with Wi-Fi, opens a TCP socket to the broker, transmits an MQTT 3.1.1 `CONNECT` packet (120 s keep-alive, clean session), and blocks with a timeout until a valid `CONNACK` confirms connection acceptance. If any step fails, the state machine retries every 5 s and automatically reconnects if the link drops at runtime. Once connected, the Nucleo publishes `hello #n from STM32 Nucleo` to `pranesh/stm32/test` once per second.

**Pseudocode:**

1. Call `mqtt_start()`: `ESP8266_Init()` → `ESP8266_wifiConnect()` → `ESP8266_portConnect()` → `MQTT_connectBroker()`.
2. In the main loop: format the counter string, publish via `MQTT_publishTopic(..., QoS0)`, log to `USART6`, and wait 1 s.

**Code Base:**

- Project files: `CAT14A_MQTT_Publish_Test.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT14A_MQTT_Publish_Test.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `mqtt_start()` and `dbg()`

**Expected Output:**

```
MQTT connected (CONNACK accepted) as stm32-pranesh-test
PUB pranesh/stm32/test : hello #1 from STM32 Nucleo
```

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

Framing MQTT `CONNECT` and `PUBLISH` packets in C and sending them over the ESP8266 TCP link enables reliable telemetry publishing to a cloud MQTT broker.
