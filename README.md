# Super Mini ESP32C3 Arduino Sketches / PlatformIO Projects

**Source code that accompanies [First Look at the Super Mini ESP32-C3](https://sigmdel.ca/michel/ha/esp8266/super_mini_esp32c3_en.html).**

---

**Code source qui accompagne [Coup d'oeil sur la carte de développement Super Mini ESP32-C3](https://sigmdel.ca/michel/ha/esp8266/super_mini_esp32c3_en.html).**

---

***February 26, 2026***

Sketches in this repository have been updated to use version 3.3.7 or better of the ESP32 Arduino core. 

A release and tag were created to easily recover the original 2024-10-14 release using version 3.0.1 of the core *as if that could be of interest*. 

---

**Table of Content**

<!-- TOC -->

- [1. Introduction](#1-introduction)
- [2. Boards](#2-boards)
- [3. Compiling the Example Sketches](#3-compiling-the-example-sketches)
  - [3.1. Board definition](#31-board-definition)
  - [3.2. Arduino IDE](#32-arduino-ide)
  - [3.3. pioarduinoIDE](#33-pioarduinoide)
- [4. Caution](#4-caution)
- [5. List of Projects](#5-list-of-projects)
  - [5.1. Hello world! sketches:](#51-hello-world-sketches)
  - [5.2. Digital input and output sketches:](#52-digital-input-and-output-sketches)
  - [5.3. Wi-Fi connectivity sketches:](#53-wi-fi-connectivity-sketches)
  - [5.4. Bluetooth connectivity sketches:**](#54-bluetooth-connectivity-sketches)
  - [5.5. Working example:](#55-working-example)
- [6. Change Log](#6-change-log)
- [7. Licence](#7-licence)

<!-- /TOC -->

## 1. Introduction

The Super Mini ESP32C3 boards are small simplified versions of the original Espressif development boards for the ESP32-C3 microcontroller. Unlike older Espressif microcontrollers, the C3 has a RISC-V core. 

## 2. Boards 

- [Super Mini ESP32C3 Pinout](images/pinout_top_big_logo.png)

These boards measure 22x18 mm, 24x18 mm including the protruding USB-C connector. A standard 8 pin 0.1" header can be soldered on each side. Of these 16 pins, 3 are dedicated to power connections (GND, 3.3 volts and 5 volts). The remaining 13 pins are input/ouput connections to the microprocessor. The boards have two push buttons (reset and boot), two LEDs (one is a power indicator, the other is a user LED) and an onboard ceramic antenna. The bigger components on the top side are the SoC, the crystal oscillator and a voltage regulator. There are no components on the back side.

There is a plus version which has an RGB user LED and a U.FL connector for an optional external antenna.

- [Super Mini ESP32C3 with 0.42" OLED Pinout](images/pinout_oled_top_big_logo.png)

These boards have an onboard 0.42" OLED display and a different form factor: 25x21 mm, 27x21 mm taking into account the USB-C connector. The same SoC pads are brought out to the header pins, but their ordering is completely different. Most of the components, SoC, oscillator and so on, are on the back side of the board which makes it almost impossible to solder the board on top of a larger printed circuit board. Perhaps that's the reason there are no crenellated edge connectors.


## 3. Compiling the Example Sketches

Each sketch or project in this repository is presented in a way that allows it to be compiled in the Arduino IDE or in the pioarduino IDE (and perhaps the PlatformIO IDE) extensions installed in a code editor such as Visual Studio Code or VSCodium. 

To compile a sketch in the Arduino IDE click on the **Open** option in the **File** menu and navigate to and select the desired file with the `.ino` extension with the system file manager. Alternately, click on the **Sketchbook** option and navigate to the desired sketch in the submenu tree ([screen capture](images/arduino_sketchbook.jpg)). The IDE must be configured as explained in the [Arduino IDE](#21-arduino-ide) section below.

To compile a project in the piaorduino IDE extension (or maybe the PlatformIO IDE), click on the home icon in the status bar at the bottom, then on the **[Open Project]** button and then navigate to the project's directory which contains its `platformio.ini` file. Click on the **[Open "&lt;project&gt;"]** button. The IDE must be configured as explained in the [pioarduinoIDE](#22-pioarduinoide) section below.

### 3.1. Board definition

One important change with the older version of this repository is the choice of the Nologo esp32c3 Super Mini board definition. The definition is present in both the Arduino and pioarduino development environments which simplifies things. The pin definition file is simple and lean. Using the *no logo* definition seems appropriate because the first boards purchased did not have any identifying markings. Nologo or WmNo Logo or wmnologo or NologoTech is (or was) selling ESP32C3 Super Mini boards among many other development boards and other products. There's an extensive English language [ESP32C3SuperMini Getting Started](https://wiki.nologo.tech/en/product/esp32/esp32c3SuperMini/esp32C3SuperMini.html) Wiki and a small [repository on Github](https://github.com/NologoTech/ESP32C3-Supermini/). However, it is not clear if this enterprise is still in operation, the link to its Aliexpress store is broken and there has not been much activity in the GitHub site. 


### 3.2. Arduino IDE 

The latest version of the Arduino IDE can be obtained for Windows, Linux and macOS in the [Downloads](https://www.arduino.cc/en/software) page from Arduino. Install the latest Espressif ESP32 Arduino core. 

 1. Add https://espressif.github.io/arduino-esp32/package_esp32_index.json in the Additional Boards Manager URLS in the Preferences window in the IDE.
  
 1. Using the Boards Manager, install platform `esp32` by Espressif, version 3.3.6 or newer. 

This is a one-time task.

When compiling a sketch from this repository 

 1. Select the `Nologo ESP32 C3 Super Mini` board in the `Tools` menu of the IDE when compiling a project. The default values for the other options in the menu are acceptable.

 1. Set the directory containing the downloaded repository as the sketchbook location in the IDE **Preferences** settings. 
      - This must be done or else some libraries will not be found and consequently some sketches will not compile.
      - Keep track of the old sketchbook location before changing it.  
      - Reset the sketchbook location to its old value when done.
       
[![](images/arduino_preferences.jpg)](images/arduino_preferences_big.jpg)

When the repository is downloaded as an archive, its top-level directory is named `xiao_esp32c3_sketches-main`. The last bit, which identifies the git branch, was removed because there's only one branch in this repository. 


### 3.3. pioarduinoIDE

All of the sketches in the repository have been successfully compiled with the [pioarduino-espressif32](https://github.com/pioarduino/platform-espressif32) platform. The latest stable release ([pioarduino/platform-espressif Release 55.03.37 Arduino Release v3.3.7 based on ESP-IDF v5.5.2.260206](https://github.com/pioarduino/platform-espressif32/releases/tag/55.03.37) February 11, 2026) was used.

The `platformio.ini` configuration file of each example project in the repository takes care of selecting the correct board, framework and platform. The path to the third-party libraries and to the source code is also specified in the configuration file. Each project should compile *as is* in the pioarduinoIDE so long as the directory structure is not changed.

This was tested using the [pioarduinoIDE extension](https://marketplace.visualstudio.com/items?itemName=pioarduino.pioarduino-ide) (v1.2.5) which is a fork of the PlatformioIDE extension in [VSCodium](https://vscodium.com/) (Version: 1.108.10359) which is itself a fork of Visual Studio Code. Note that the PlatformioIDE extension is not available in the VSCodium marketplace and its manual installation is not easily done because the VSIX package is not readily found anymore.


## 4. Caution

There are more than one manufacturer of these boards that differ in more or less subtle ways. Some boards are very thin: 0.6 mm which makes them somewhat fragile. Others are more robust being almost twice as thick 1 mm. 

A major problem with some Super Mini boards is their lack lustre to very bad Wi-Fi performance.

<!--
The pin diagram above shows the markings on four boards purchased from a Chinese vendor in late April 2024. There are no labels identifying the red power LED to the left of the USB connector and the blue LED under the reset (RST) button. There are no other components near the single component between the ESP32-C3 chip and the red ceramic antenna labelled C3. Other boards and the [schematic](https://wiki.icbbuy.com/doku.php?id=developmentboard:esp32-c3mini#schematic) have other components which may explain the problems encountered with Wi-Fi connectivity. 
-->


## 5. List of Projects      

The projects can be grouped in categories. 

### 5.1. Hello world! sketches:

**00_sys_info**  

  Displays information about the ESP32-C3 system on a chip.

**01_pin_names** 

  Lists the I/O pin names and numbers of the Super Mini ESP32-C3 board along with some macro definitions.


### 5.2. Digital input and output sketches:

**02_blink_pulse_led**

  Alternately blinks (heartbeat) and pulses the on-board LED of the Super Mini board. The switch between these modes 
  is done by pressing the boot button. Tests digitalWrite(), analogWrite(), and digitalRead().


### 5.3. Wi-Fi connectivity sketches:


**03_scan_wifi**

  Prints a list of available Wi-Fi networks every five seconds. Some Super Mini boards do not manage to find as many networks as others because of bad Wi-Fi performance. Edit `secrets.h.template` and save as `secrets.h` before compiling.
  
**04_wifi_connect**

  Wi-Fi connection example. The Super Mini may very well fail to connect. Define the TRY_TX_POWER macro to see if that solves the problem. It may be necessary to change the value of the TX_POWER macro. Edit `secrets.h.template` and save as `secrets.h` before compiling.

**05_wifi_tx_power**

  Tests each predefined value for the Wi-Fi TX (transmit) power and records the time required to connect to the Wi-Fi network. Edit `secrets.h.template` and save as `secrets.h` before compiling.     

  The table shows times needed to connect to a Wi-Fi network in milliseconds as a function of the radio TX power setting. The tests were run only once on a XIAO ESP32C3 and once on each of four different Super Mini boards. The `-` signifies that a connection was not made within two minutes. 

|                           |  XIAO	|	SM 1 |	SM 2 | SM 3	|	SM 4 |
| ---                       | :---: |	:---:| :---: | :---:|	:---:|
|	      WIFI_POWER_default 	|	1143	|	-	|	-	|	- 	|	- 	|
|	      WIFI_POWER_19_5dBm 	|	443	|	-	|	1230	|	- 	|	- 	|
|	        WIFI_POWER_19dBm 	|	430	|	477	|	-	|	- 	|	-	|
|	      WIFI_POWER_18_5dBm 	|	440	|	546	|	961	|	- 	|	- 	|
|	        WIFI_POWER_17dBm 	|	391	|	443	|	389	|	532	|	- 	|
|	        WIFI_POWER_15dBm 	|	404	|	410	|	425	|	422	|	- 	|
|	        WIFI_POWER_13dBm 	|	376	|	729	|	684	|	469	|	1440	|
|	        WIFI_POWER_11dBm 	|	429	|	423	|	399	|	409	|	484	|
|	       WIFI_POWER_8_5dBm 	|	729	|	1929	|	414	|	425	|	443	|
|	         WIFI_POWER_7dBm 	|	1923	|	718	|	413	|	430	|	399	|
|	         WIFI_POWER_5dBm 	|	904	|	388	|	427	|	546	|	442	|
|	         WIFI_POWER_2dBm 	|	678	|	507	|	390	|	937	|	408	|

**These results were obtained with the older version of the sketch**. The table contains the time required to connect to the Wi-Fi network and the time needed to acquire a valid IP address from the Wi-Fi router. The newer version of the sketch tabulates connection times only. The test will have to be redone.

<!--
Three conclusions can be drawn. 

  1. The XIAO connected with the Wi-Fi router no matter the TX power setting. That may not be all that significant since the router was 1 metre away.
  
  2. Not one of the four Super Mini boards was able to connect to the Wi-Fi network with the default TX power setting.

  3. There is significant variation between the Super Mini boards. 
  
When deploying a board, it may be necessary to test it multiple times in the position it will be used to find the proper TX power setting.
-->

### 5.4. Bluetooth connectivity sketches:

**07_ble_led**

A rudimentary example of the Bluetooth® Low Energy (BLE) capabilities of the ESP32-C3. The microcontroller is set up as a BLE peripheral with a LED service and as switch characteristic. Any central (client) device can turn the LED on or off by sending a proper message. [Bluetooth Controlled LED](https://sigmdel.ca/michel/ha/xiao/xiao_esp32c3_intro_en.html#ble) explains how to use **LightBlue**, an Android and presumably IOS application, to do that.

The project uses the [ArduinoBLE](https://github.com/arduino-libraries/ArduinoBLE) library from *arduino-libraries* which has very recently been updated. 

**Currently (2026-02-26) this project is not working. When using an ESP32-C3 based board it reboots after a core panic which occurs when the `BLE.begin()` is invoked.** More details are found at the top of the `main.cpp` file.

<!--
The Super Mini C3 boards that were tested did not do well. Sometimes the connection between the microcontroller and the Android tablet would require a considerable amount of time. At other times, the tablet had to be almost touching the microcontroller for the connection to be made. It happened that a connection could not be established at all even if **LightBlue** had found the Super Mini when it scanned for devices. By contrast, the XIAO-ESP32C3 was dependable.-->


**08_ble_led2**

This project is similar to the previous one except for the use of the BLE library instead of the ArduinoBLE library. The [BLE library](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE) included in the ESP32 core for Arduino was the creation of [Neil Kolban](https://github.com/nkolban/ESP32_BLE_Arduino). While in principle the BLE Tx power level could be modified in the hope of improving the performance of some Super Mini boards, problems have been encountered when setting the power level. Consequently, the project does not modify the default BT Tx power level.


<!--
This makes it possible to set the BLE power level. Unlike setting Wi-Fi TX (transmit) power, setting the BLE power level did not result in obvious improvements in Bluetooth connectivity. Only BLE power type `ESP_BLE_PWR_TYPE_DEFAULT` with power levels  -21dbm, -15dbm, -9dbm, -3dbm, +3dbm + 9dbm, +15dbm and +21dbm, were tested. BLE power settings are more complex than Wi-Fi transmit power settings, so there may be something to gain in investigating this further. 

Perhaps this test was done incorrectly. Any help on this subject would be appreciated.
-->



### 5.5. Working example:

**06_async_web_led**

Toggles the built-in LED on and off with a Web interface. It may be necessary to specify a valid Wi-Fi tx power as determined with the previous sketch. Edit `secrets.h.template` and save as `secrets.h` before compiling. 
  
Aside from setting the radio TX power and handling the fact that the built-in LED is active LOW, this project is the same as **05_async_web_led** in [xiao_esp32c3_sketches](https://github.com/sigmdel/xiao_esp32c3_sketches).

 
## 6. Change Log

| Date | Change |
| :---  |  :--- |
| 2026-07-26 | Update to version 3.3.7 of the esp32 arduino core |
| 2026-02-24 | Created a tag and release labelled v3.0.1_2024-10-14 |
| 2024-10-14 | Merged pull requests from Beherith and billzajac | 
| 2024-07-24 | Added 07_ble_led2 |
| 2024-06-21 | Added 07_ble_led |
| 2024-06-21 | Updated to v3.0.1 esp32 Arduino core|
| 2024-05-12 | Initial version | 

## 7. Licence

Copyright 2024-2026, Michel Deslierres. No rights reserved. 

While the copyright pertaining to included libraries must be respected, all the code by Michel Deslierres in this repository is in the public domain. In those jurisdictions where this may be a problem, the [BSD Zero Clause License](https://spdx.org/licenses/0BSD.html) applies.
