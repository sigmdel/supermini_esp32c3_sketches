/*
 * scan_wifi.ino
 *
 * Prints a list of available wifi networks every five seconds with 
 * an ESP32-C3 based Super Mini 
 * 
 * In the Arduino IDE use board 'Nologo ESP32C3 Super Mini' or 
 * 'MakerGO C3 SuperMini'. These board definitions for the 
 * Super Mini boards are not contained in the currently 
 * (2024-05-08) available stable release of the Arduino-esp32 
 * core (@ https://github.com/espressif/arduino-esp32). They
 * should be included in the next stable release.
 *
 * Until then, use release candidate 1 of version 3.0.0 of the 
 * esp32 core. To do that, add the following boards manager 
 * configuration file
 *   https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json
 * in the Arduino IDE preferences.
 *
 * With the stable version 2.0.16 of the esp32 Arduino core, use board
 * 'ESP32C3 Dev Module' and ensure that the `USB CDC on BOOT' option 
 * is 'Enabled'.
 * 
 
 * 'BUILTIN_LED' and 'LED_BUILTIN' will not be correct with these
 * three board definitions.
 
 * This is a stub to satisfy the Arduino IDE, the source code is in
 * the file main.cpp in the same directory.
 *
 * This sketch will compile in the Arduino IDE and in PlatformIO
 *
 * Source: Example WiFiScan.ino in the arduino-esp32 core
 *  @https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiScan/WiFiScan.ino
 *
 * License: unknown for the example file.
 * The GNU LESSER GENERAL PUBLIC LICENSE Version 2.1, February 1999 applies for
 * the complete arduino-esp32 package. See
 *   https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md
 */
