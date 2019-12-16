# HADES Hardware

This folder contains all hardware design files for the HADES flight control system.

Currently, there are three designs:

* *Hades:* Dual STM32-based board containing microcontrollers (F4 and H7), inertial sensors, GPS receiver, power distribution system for servos (up to 9A at 5V). This is the main platform for testing.
* *Hades Micro:* 5cmx5cm flight control board with a single STM32F4. No GPS receiver and limited power distribution.
* *Hades Micro (JLCPCB):* Nearly identical to Hades Micro, however uses SPI instead of I2C for sensor communication. Additionally, nearly all parts are available through LCSC/JLCPCB for assembly.