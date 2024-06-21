/*
 * pin_names.ino
 *
 * List I/O pin names and numbers of supported Super Mini ESP32-C3 boards
 *
 * This is a stub to satisfy the Arduino IDE, the source code is in
 * the file main.cpp in the same directory.
 *
 * This sketch will compile in the Arduino IDE
 *
 *  1- Add https://espressif.github.io/arduino-esp32/package_esp32_index.json
 *     in the Additional Boards Manager URLS in the Preferences window.
 *  2- Install platform esp32 by Espressif version 3.0.1 or newer with the Boards Manager
 *  3- Select the MakerGO ESP32 C3 SuperMini board
 *
 * This sketch will also compile in PlatformIO with platform espressif32 v6.6.0 or newer
 *
 * Use the following platformio.ini configuration file
 *   [env:dfrobot_beetle_esp32c3]
 *   platform = espressif32
 *   board = dfrobot_beetle_esp32c3
 *   framework = arduino
 *
 * or use
 *   [env:super_mini_esp32c3]
 *   platform = espressif32
 *   board = super_mini_esp32c3
 *   framework = arduino
 * if the the board definition file super_mini_esp32c3.json and the variant pin
 * definition file pins_arduino.h have been added as explained in
 * https://github.com/sigmdel/supermini_esp32c3_sketches/tree/main/resources
 * 
 * 'BUILTIN_LED' and 'LED_BUILTIN' will not be correct with these
 * three board definitions.
 *
 *
 * Michel Deslierres
 * May 8, 2024
 *
 * Copyright 2024, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html>
 */

// SPDX-License-Identifier: 0BSD
