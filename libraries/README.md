# Additional Libraries

**Table of Content**
<!-- TOC -->

- [1. MACs library](#1-macs-library)
- [2. Async libraries](#2-async-libraries)
  - [2.1. Incomplete Libraries](#21-incomplete-libraries)
- [3. Arduino IDE Configuration](#3-arduino-ide-configuration)
- [4. pioarduino/PlatformIO IDE Configuration](#4-pioarduinoplatformio-ide-configuration)
- [5. Installing the Full Libraries with the Library Manager](#5-installing-the-full-libraries-with-the-library-manager)
  - [5.1. Arduino IDE](#51-arduino-ide)
  - [5.2. pioarduino IDE](#52-pioarduino-ide)

<!-- /TOC -->



## 1. MACs library

This library is used in the 
   - `03_scan_wifi`, 
   - `04_wifi_connected`, 
   - `05_wifi_tx_power`, 
   - `06_async_web_led`, and 
   - `07_ble_led` 

projects.

This library is available in this directory only.

## 2. Async libraries

The following two libraries from the ESP32Async organization

 - [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer)  (version 3.9.6)
 - [AsyncTCP](https://github.com/ESP32Async/AsyncTCP) (version 3.4.10)

are used in the `06_async_web_led` project.

### 2.1. Incomplete Libraries

:warning: Only essential elements of the two additional libraries are included; all examples and documentation have been removed. 
**Please do not redistribute the content of these directories and always return to the original source when using the libraries in other projects.**

Of course, the incomplete libraries can be replaced by the full libraries. The simplest could be to delete the directories containing the incomplete libraries. Then for each library

- Click on the link to its GitHub repository given above
- Click on the ``[Code]`` button and then on the `Download ZIP` button
- Save the downloaded archive
- Extract the library to a subdirectory of this directory which contains this `README.md` file.

## 3. Arduino IDE Configuration

These local libraries will be found and used when compiling the projects only if the sketchbook location is correctly configured in the Arduino IDE.

   1. Go to the Preferences window of the IDE: **File » Preferences**. It would be a good to jot down the previous sketchbook location beforehand in order to restore it when done with the example sketch.
   3. Enter the directory containing the `03_button_led` and `05_async_web_led` directories in  `Sketchbok location:`  
   ![](location.jpg)  
   4. Click on the `OK` button.


## 4. pioarduino/PlatformIO IDE Configuration

There is nothing to do here. The `platformion.ini` configuration file contains an entry to the local libraries directory: `lib_dir = ../libraries`. 

## 5. Installing the Full Libraries with the Library Manager

First remove the directory containing this file.

### 5.1. Arduino IDE

The ESP32Async organization libraries are can be installed with the ArduinoIDE Library Manager (Ctrl+Shift+i or clicking on the library icon which looks like four books on a shelf in the left tool bar). Enter `esp32async` in the search box and click on the **INSTALL** button of the **Async TCP** by ESP32Async and the **ESP Async WebServer** by ESP32Async entries. The ArduinoBLE library can be installed in a similar fashion by searching form `arduinoble`.

### 5.2. pioarduino IDE

The procedure is very similar in pioarduinoIDE. Go to PIO Home, click on the Libraries icon, and search using `esp32async`. Then click on the 
`AsyncTCP` by ESP32Async and the `ESPAsyncWebServer` by ESP32Async links to install the libraries. Similarly search using `arduinoble` and click on the `ArduinoBLE` by Arduino link to install.

It may be preferable to simply specify the dependency on a project-by-project basis in each project's `platformio.ini` configuration file. The required `lib_deps` values are already in the pertinent configuration file as comments. Remove the leading ';' to enable.

In either case, remove the `lib_dir` entry in the `platformio.ini` file.
