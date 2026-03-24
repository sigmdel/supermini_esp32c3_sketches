/*
 * deep_sleep_io.ino
 * Wake an ESP32-C3 Super Mini from deep sleep mode with a high or low signal to i/o pins
 *
 * This is a stub to satisfy the Arduino IDE, the source code is in
 * the main.cpp file in the same directory.
 *
 * This sketch will compile in the Arduino IDE
 *
 *  1- Add https://espressif.github.io/arduino-esp32/package_esp32_index.json
 *     in the Additional Boards Manager URLS in the Preferences window.
 *  2- Install platform esp32 by Espressif version 3.3.7 or newer with the Boards Manager
 *  3- Select the Nologo ESP32C3 Super Mini board
 *
 * Michel Deslierres
 * March 23, 2026
 *
 * Based on Deep sleep mode
 *  @https://wiki.seeedstudio.com/xiao_esp32c5_getting_started/#deep-sleep-mode
 *
 * The XIAO ESP32-C5 features deep sleep and wake-up functions. This example utilizes a 
 * high-level trigger on pin D0 to wake the device from deep sleep. It is important to 
 * note that this is a configurable option, as the hardware supports both high-level and 
 * low-level triggers to accommodate different circuit designs.
 *
 * Copyright 2026, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html>*/
// SPDX-License-Identifier: 0BSD

