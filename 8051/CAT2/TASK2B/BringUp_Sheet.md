# CAT 2 - Task 1 : Hardware Board Bring-Up Sheet

No separate firmware is required for Task 1. Record the bench measurements in the table below during the lab session, then flash the Task 2 project to verify the toolchain and target board.

| Parameter | Recorded Specification / Measurement |
|---|---|
| Microcontroller | AT89S52 (Microchip / Atmel) |
| CPU Architecture | 8051 (MCS-51), 8-bit core, 12 oscillator periods per machine cycle |
| Nominal Operating Voltage | 5 V (datasheet rating: 4.0 V – 5.5 V) |
| Measured `VCC` (pin 40 to pin 20) | ______ V  *(measure using a digital multimeter)* |
| Crystal Oscillator | 11.0592 MHz → 1.085 µs machine cycle |
| Programming / Debug Interface | No on-chip debug unit. Flashed via SPI ISP (`P1.5 MOSI`, `P1.6 MISO`, `P1.7 SCK`, `RST`) using a USBasp programmer and progisp |
| User LED GPIO Port | Port 1 |
| User LED GPIO Pin | `P1.0` (on-board indicator LED) |
| User LED Active Logic | Active-LOW (LED illuminates when `P1.0 = 0`) |
| LED Blink Frequency (Hz) | ~1 Hz (0.5 s ON, 0.5 s OFF) — measured: ______ Hz |

## Procedure
1. Locate the AT89S52 microcontroller, 10-pin USBasp ISP header, power supply stage, and on-board user LED using the kit schematic (`04_Tools_and_Drivers/8051_Dev_Board_Schematic_Codes_Programmer_Drivers/Amotech Labs 8051 Board Schematics`).
2. Apply power to the board and measure the supply voltage across pin 40 (`VCC`) and pin 20 (`GND`) with a multimeter.
3. Trace the user LED circuit to `P1.0` on the schematic and verify its active-low connection.
4. Install the USBasp driver (via Zadig / `USBasp-Drivers_Windows_11`), compile the Task 2 Keil project (**F7**), and flash the generated `Objects/*.hex` binary using progisp.
