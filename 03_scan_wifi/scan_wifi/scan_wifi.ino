/*
 * scan_wifi.ino
 *
 * Prints a list of available wifi networks every five seconds with 
 * an ESP32-C3 based Super Mini 
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
 * Michel Deslierres
 * May 8, 2024
 *
 * Source: Example WiFiScan.ino in the arduino-esp32 core
 *  @https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiScan/WiFiScan.ino
 *
 * License: unknown for the example file.
 * The GNU LESSER GENERAL PUBLIC LICENSE Version 2.1, February 1999 applies for
 * the complete arduino-esp32 package. See
 *   https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md
 */
