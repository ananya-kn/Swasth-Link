# SwasthLink wiring (ESP32 DevKit)

| Block | Signal | ESP32 pin | Notes |
|---|---|---|---|
| I2C bus | SDA / SCL | GPIO21 / GPIO22 | MAX30102 (0x57), MLX90614 (0x5A), SSD1306 (0x3C) |
| SIM800L | TXD -> RX2 / RXD <- TX2 | GPIO16 / GPIO17 | 2G only. Power from the battery rail (3.7-4.2 V), NOT the 3.3 V regulator: it draws bursts up to ~2 A. Add 1000 uF + 100 nF at the module. |
| NEO-6M | TXD -> GPIO18 | GPIO18 | 3.3 V; place antenna near a window/top of the case |
| Buttons | MEASURE / SOS | GPIO5 / GPIO4 | to GND, internal pull-ups; SOS = hold 2 s |
| Buzzer / LED | | GPIO25 / GPIO26 | |
| INMP441 (optional) | SCK / WS / SD | GPIO14 / GPIO15 / GPIO32 | not used by the current sketch; phone does speech recognition |
| Power | Li-ion -> TP4056 (with DW01 protection) -> 3.3 V regulator (>= 800 mA) -> ESP32 3V3 and sensors | | Check the SIM800L current budget before closing the case |

Notes: MAX30102 needs 1.8 V core/3.3 V LEDs: use a ready module with level handling. Cold fingers, motion and bright light ruin SpO2: keep the finger well light-tight (matte black).
