/*
 * ble_led.ino
 *
 * Toggle an external LED on and off an app using Bluetooth LE
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
 *  4- Install the ArduinoBLE (1.3.6 or newer) library with the Library Manager
 *  5- Temporarily move the directory ~/.arduino15/packages/esp32/hardware/esp32/3.0.1/libraries/BLE
 *     outside of the ~/.arduino15/ tree to avoid name conflicts 
 *
 * This sketch will also compile in PlatformIO with platform espressif32 v6.6.0 or newer
 *
 * Use the following platformio.ini configuration file
 *   [env:dfrobot_beetle_esp32c3]
 *   platform = espressif32
 *   board = dfrobot_beetle_esp32c3
 *   framework = arduino
 *   lib_deps = arduino-libraries/ArduinoBLE@^1.3.2
 *
 * or use
 *   [env:super_mini_esp32c3]
 *   platform = espressif32
 *   board = super_mini_esp32c3
 *   framework = arduino
 *   lib_deps = arduino-libraries/ArduinoBLE@^1.3.2
 * if the the board definition file super_mini_esp32c3.json and the variant pin
 * definition file pins_arduino.h have been added as explained in
 * https://github.com/sigmdel/supermini_esp32c3_sketches/tree/main/resources
 * 
 * Michel Deslierres
 * May 10, 2024
 *
 * Copyright 2024, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html>
 
// SPDX-License-Identifier: 0BSD
