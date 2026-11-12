# Repository Maintenance & Contribution Guide

## Building and Running Tests

```bash
tools/build_all.sh            # builds all 17 STM32 + 15 8051 targets (requires arm-none-eabi-gcc and sdcc)
tools/build_all.sh stm32      # builds STM32 targets only
tools/build_all.sh 8051       # builds 8051 targets only
make -C tests                 # runs host unit tests (MQTT packet codec, ESPLib, ESP8266 AT driver)
```

- **macOS:** Install SDCC via `brew install sdcc`; the script automatically locates the `arm-none-eabi-gcc` toolchain bundled inside STM32CubeIDE.
- **Ubuntu:** Install dependencies with `sudo apt-get install gcc-arm-none-eabi libnewlib-arm-none-eabi sdcc sdcc-libraries`.

Each STM32 directory remains a standard STM32CubeIDE project (**File → Import → Existing Projects into Workspace**); `tools/build_all.sh` simply provides a headless command-line build across all tasks.

## Keeping Shared Library Copies Synchronised

Because STM32CubeIDE compiles source files directly from each project's `Core/` directory, every project maintains its own local copy of the shared drivers. The canonical master files reside in `Common_Libraries/`.

| Canonical Master Directory | Project Directories to Synchronise |
|---|---|
| `Common_Libraries/ESPLib_MQTT/` (`ESPLib.*`, `mqtt_packet.*`) | `STM32/CAT13/TASK13A`, `TASK13B`, `TASK13C` (`Core/Src` + `Core/Inc`) |
| `Common_Libraries/ESP8266_AT_Driver/` (`esp_at.*`) | `STM32/CAT10/TASK10D`, `STM32/CAT12/TASK12B` |
| `Common_Libraries/BME280_Bosch_API/` | `STM32/CAT12/TASK12A`, `TASK12B`, `STM32/CAT13/TASK13C` |
| `Common_Libraries/Helpers/` (`wifi_config.h`, `fmt_fixed.h`) | All Wi-Fi and BME280 projects listed above |
| 8051 `delay.c/.h`, `lcd.c/.h` | Respective 8051 task folders |

Recommended workflow: edit the master file in `Common_Libraries/` → run `make -C tests` → run `tools/check_lib_copies.sh --fix` to propagate updates across all task folders → run `tools/build_all.sh`. Running `tools/check_lib_copies.sh` verifies that no project copy has drifted from its master. You can also copy files manually:

```bash
L=Common_Libraries/ESPLib_MQTT
for p in STM32/CAT13/TASK13*/; do cp $L/*.c "$p/Core/Src/"; cp $L/*.h "$p/Core/Inc/"; done
```

## Updating 8051 HEX Binaries

The pre-built `Objects/*.hex` files are compiled from the Keil C51 sources using SDCC. Whenever an 8051 source file changes, run `UPDATE_HEX=1 tools/build_all.sh 8051` to refresh the tracked HEX images so the default build check continues to report `"matches committed HEX"`.

## Wi-Fi Credentials

Never commit real network credentials to version control. Place your SSID and password inside `Core/Inc/wifi_secrets.h` (which is git-ignored); refer to `wifi_config.h` for the expected `#define` macros.

## Build Log

`docs/build-log.txt` records the console output of `tools/build_all.sh` followed by `make -C tests`. Refresh this file after making structural or code changes.
