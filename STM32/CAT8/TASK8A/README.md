# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 8

**DATE:** 18/09/2026

**NAME OF EXP:** STM32 GPIO Bring-Up and Switch-Controlled LED

**HARDWARE SETUP / PLATFORM:** Blue Pill STM32F103C6T6 (ARM Cortex-M3, 72 MHz) + push-button — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL

---

## TASK 8A : GPIO Input/Output

### 1. Circuit Diagram

| Pin | CubeMX Mode | Hardware Connection |
|---|---|---|
| `PC13` | `GPIO_Output`, push-pull | On-board user LED (**active-LOW**) |
| `PA15` | `GPIO_Input`, internal pull-up | Tactile push-button switching to `GND` |
| `PA13` / `PA14` | `SYS`: Debug = Serial Wire (SWD) | ST-LINK debugger (releases `PA15` from JTAG `JTDI`) |
| `RCC` | 8 MHz HSE crystal → PLL ×9 | 72 MHz system clock |

![Circuit Diagram](circuit.png)

*Created in [Wokwi](https://wokwi.com). To view or modify the schematic: open a new project on wokwi.com, copy [`wokwi/diagram.json`](wokwi/diagram.json) into the `diagram.json` editor, and upload any accompanying `wokwi/*.chip.json` custom definitions via **New file**. Custom chip definitions represent the AT89S52, USB-TTL adapter, ULN2003 driver, USBasp programmer, and ST-LINK dongle. In later diagrams, the ESP-01 and BMP180 symbols represent the Wemos D1 mini and BME280 respectively (sharing identical pinouts and protocols), while the Nucleo-64 board represents the F401RE.*

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

With the internal pull-up enabled on `PA15`, the pin reads logic `1` while the switch is open and logic `0` while pressed to ground. The main loop continuously samples the button state and mirrors it directly onto the active-low `PC13` LED via `led_write(button_raw_pressed())`.

**Pseudocode:**

1. Run `HAL_Init()`, configure the 72 MHz system clock, and initialise GPIO pins.
2. Set the `PC13` LED to its initial OFF state (`GPIO_PIN_SET`).
3. Sample `PA15`: drive `PC13 = 0` (LED ON) while pressed, or `PC13 = 1` (LED OFF) while released.
4. Repeat continuously.

**Code Base:**

- Project files: `CAT08A_GPIO_Input_Output.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT08A_GPIO_Input_Output.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `button_raw_pressed()` — wraps `HAL_GPIO_ReadPin()`
- `led_write()` — wraps `HAL_GPIO_WritePin()` accounting for active-low polarity

### 3. Inference

The on-board LED tracks the push-button state in real time, validating the CubeMX clock tree, internal pull-up input, and active-low GPIO output setup.
