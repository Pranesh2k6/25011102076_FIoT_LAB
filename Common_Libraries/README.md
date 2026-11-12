# Common Libraries

Canonical master copies of the reusable drivers and helper headers shared across experiments. Because STM32CubeIDE compiles source files directly from each project's `Core/Src` and `Core/Inc` directories, individual STM32 tasks maintain local copies synchronized from these master files.

| Directory | Source Files | Assessments | Provenance & Version | Licence | Design Constraints & Notes |
|---|---|---|---|---|---|
| `BME280_Bosch_API` | `bme280.c/.h`, `bme280_defs.h` | CAT 12, 13 | Official Bosch Sensortec `BME280_SensorAPI` v3.5.1 (unmodified) | BSD-3-Clause | Requires user-supplied `read`, `write`, and `delay_us` callbacks; compiled with `BME280_DOUBLE_ENABLE` (double-precision compensation on Cortex-M4) |
| `ESPLib_MQTT` | `ESPLib.c/.h`, `mqtt_packet.c/.h` | CAT 13 | Clean-room implementation matching the course ESPLib API signatures | MIT | Operates over a single TCP link (`CIPMUX=0`); supports QoS 0 exclusively (QoS 1/2 requests return an error and incoming QoS 1/2 publishes are discarded); packets exceeding 256 B, payloads > 127 B, and topics > 63 chars are dropped and tallied; buffers one incoming message at a time; unencrypted MQTT over port 1883 |
| `ESP8266_AT_Driver` | `esp_at.c/.h` | CAT 10D, 12B | Custom implementation | MIT | Uses timeout-bounded blocking during `AT+CIPSEND` transfers; lines exceeding 127 characters are skipped and counted; `esp_at_read_line()` reports the full unclipped length to expose small caller buffers; automatically retries failed UART interrupt re-arming in the main loop; flushes buffered bytes on RX overflow and resynchronises at the next line ending |
| `Helpers` | `fmt_fixed.h`, `wifi_config.h` | CAT 10C – 13 | Custom implementation | MIT | `fmt_2dp()` formats fixed-point values within ±21,474,836.47 (`int32 × 100`); real Wi-Fi credentials reside in the git-ignored `wifi_secrets.h` header |

For the 8051 experiments, reusable modules—including `delay.c/.h` (cycle-calibrated volatile busy-wait loops), `uart.c/.h` (CAT 6C interrupt-driven UART with a 16-byte RX circular buffer), and `lcd.c/.h` (CAT 7 4-bit HD44780 driver)—are kept directly inside their respective task folders.

Unit testing: run `make -C tests` on the host machine to execute the MQTT packet codec and ESPLib test suites against a simulated ESP8266 modem.
