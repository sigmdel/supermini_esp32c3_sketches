# Super Mini ESP32C3 Arduino Sketches / PlatformIO Projects

**Source code that accompanies the May 2024 version of [First Look at the Super Mini ESP32-C3](https://sigmdel.ca/michel/ha/esp8266/super_mini_esp32c3_en.html)**.

![Super Mini ESP32C3 Pinout](images/pinout_top_big_logo.png)

<!-- TOC -->

- [1. Introduction](#1-introduction)
- [2. Arduino IDE Notes](#2-arduino-ide-notes)
- [3. PlatformIO Notes](#3-platformio-notes)
- [4. List of Projects](#4-list-of-projects)
  - [01_pin_names](#01_pin_names)
  - [02_blink_pulse_led](#02_blink_pulse_led)
  - [03_scan_wifi](#03_scan_wifi)
  - [04_wifi_connect](#04_wifi_connect)
  - [05_wifi_tx_power](#05_wifi_tx_power)
  - [06_async_web_led](#06-_async_web_led)
- [5. License](#5-license)

<!-- /TOC -->

## 1. Introduction

The Super Mini ESP32C3 boards are a small simplified versions of the original Espressif development boards for the ESP32C microcontroller. Unlike older Espressif microcontrollers these boards have a RISC-V core. It should be possible to compile each project  in the Arduino IDE and in PlatformIO. 

## 2. Arduino IDE Notes

Arduino sketches must have an `.ino` file name extension and must be contained in a directory that has the same name as the Arduino sketch (excluding the extension). Consequenty the `01_pin_names` project containts a directory named `pin_names` that in turn contains the Arduino sketch `pin_names.ino`. That sketch is basically empty as it is a long comment only. This is not a problem because the Arduino IDE will import all source files found in the sketch directory. The actual code is in `main.cpp` which is the default name of a PlatformIO project.

![Directory tree](images/dir_tree.jpg) 

To compile and then upload the sketch in the Arduino IDE, click on the **File** top menu, click on **Open...**, then navigate to the `pin_names.ino` file and open it with the system file manager.

Following the instructions in [Installing using Arduino IDE](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide) the latest release of the Arduino-ESP32 framework was installed by entering the following URL 
```
https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json
```
into the `Additional Board Manager URLs` field of the Arduino Preferences. Using the board manager install the Espressif Arduino core `esp32 version 3.0.0-rc1` or newer. It should then be possible to select `MakerGO ESP32 C3 SuperMini` as the board.

## 3. PlatformIO Notes

Because of the Arduino sketch naming constraints, the `main.cpp` file of a project is not stored in the `src` directory. To work around this change, a `src_dir` entry is added in the `platformio.ini` file to provide the name of the directory in which `main.cpp` is found. That will be the name of the Arduino sketch as shown here for the `01_pin_names` project. 

```ini
[platformio]
; Make the Arduino IDE happy (.INO file must be in a directory of the same name)
src_dir = pin_names
```

PlatformIO will "convert" the Arduino sketch, but that is of no consequence since it contains only comments.

## 4. List of Projects      

### 01_pin_names
  Lists the I/O pin names and numbers of the Super Mini ESP32-C3 board.

### 02_blink_pulse_led
  Alternately blinks (heartbeat) and pulses the on board LED of the Super Mini board.

### 03_scan_wifi
  Prints a list of available Wi-Fi networks every five seconds. The Super Mini do not manage to find as many networks as the XIAO ESP32C3. Edit `secrets.h.template` and save as `secrets.h` before compiling.
  
### 04_wifi_connect
  Wi-Fi station connect example. The Super Mini may very well fail to connect. Define the TRY_TX_POWER macro to see if that solves the problem. It may be necessary to change the value of the TX_POWER define. Edit `secrets.h.template` and save as `secrets.h` before compiling.

### 05_wifi_tx_power
  Tests each possible value for the Wi-Fi tx power and records the time required to connect to the Wi-Fi network. Edit `secrets.h.template` and save as `secrets.h` before compiling.     

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

Two conclusions can be drawn. The XIAO connected with the Wi-Fi router which was only a meter away no matter the TX power setting. None of the four Simple Mini was able to connect to the Wi-Fi network with the default TX power setting.

### 06_async_web_led
  Toggles the built-in LED on and off with a Web interface. May be necessary to specify a valid Wi-Fi tx power as determined with the previous sketch. Edit `secrets.h.template` and save as `secrets.h` before compiling. Not yet tested in the Arduino IDE.
  
  Asside from setting the radio TX power and handling of an active LOW LED, this project is the same as **05_async_web_led** in [xiao_esp32c3_sketches](https://github.com/sigmdel/xiao_esp32c3_sketches).
## 5. License

Copyright 2024, Michel Deslierres. No rights reserved. 

While the copyright pertaining to included libraries must be respected, all the code by Michel Deslierres in this repository is in the public domain. In those jurisdictions where this may be a problem, the [BSD Zero Clause License](https://spdx.org/licenses/0BSD.html) applies.
