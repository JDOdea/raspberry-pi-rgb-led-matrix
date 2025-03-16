Controlling RGB LED display with Raspberry Pi GPIO
==================================================


A recreated and modified library for controlling 64x64, 32x32, or 16x32 RGB LED panels using a Raspberry Pi. 
Original repo can be found [here](https://github.com/hzeller/rpi-rgb-led-matrix).

Supports 3 chains with many panels each on a regular Pi, as per the original repo's README.
Raspberry Pi 2 or 3 can chain 12 panels in that chain (36 panels total), with 96-ish panels (32 chain length) being potentially possible.

Overview
--------
The `RGBMatrix` class in `include/led-matrix.h` is what controls these.