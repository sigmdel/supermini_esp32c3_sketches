# SSD1315 Arduino Library

A lightweight library for 72x40 OLED displays based on the SSD1315 controller.

## Features

- Draw pixels, rectangles, bitmaps and text
- Multiple fonts including a compact 5x8 font
- Screen rotation (0/90/180/270 degrees)
- Adjustable brightness and sleep mode
- Examples demonstrating usage

## Changes (2026-03-28) &lt;sigmdel.ca&gt;

The [72x40oled_lib](https://github.com/AbdulKus/72x40oled_lib) is a
repackaging of [EastRisging Technology](https://www.buydisplay.com/i2c-white-0-42-inch-oled-display-module-72x40-arduino-raspberry-pi)
`er_oled` driver as an Arduino library. From the `Examples_OLEDMO.42-1.ino` two
physical connections between the OLED and the microcontroller were contemplated:

```
/*
  == Hardware connection for 4 PIN module==
    OLED   =>    Arduino
  *1. GND    ->    GND
  *2. VCC    ->    3.3V
  *3. SCL    ->    SCL
  *4. SDA    ->    SDA

 == Hardware connection for 7 PIN module==
 Note:The module needs to be jumpered to an I2C interface.
    OLED   =>    Arduino
  *1. GND    ->    GND
  *2. VCC    ->    3.3V
  *3. SCL    ->    SCL
  *4. SDA    ->    SDA
  *5. RES    ->    8
  *6. DC     ->    GND
  *7. CS     ->    GND
*/
```

The library assumes that a 7 PIN module is used and that by default
the OLED reset pin (RES) is connected to a microcontroller GPIO pin (8 by default):

```cpp
class SSD1315 {
public:
    SSD1315(uint8_t rstPin=8);
```
The onboard display of the ESP32C3 Super Mini with 0.42" OLED has a
four-wire connection. As a consequence `display.begin()` would toggle GPIO 8 which prevented use of the library with the onboard OLED of the Super Mini. To
avoid this problem, create the SSD1315 object with the `NO_RESET_PIN` as the `rstPin` value. 

```cpp
SSD1315 display(NO_RESET_PIN);
```

The `NO_RESET_PIN` macro (equal to 255) is defined in the library header `SSD1315.h`. 

### `invert()` function
A function was added.
```cpp
  void invert(bool enable); 
```  

This function inverts the value of all the display pixels at once when `enable` is set to `true` and restores their value when `enable` is set to `false`. In other words 

```cpp
display.invert(true);
delay(1000);
display.invert(true);
delay(1000);
display.invert(false);
```
will invert the display for 2 seconds. Think of `invert` as an on/off switch, not as a toggle.


## Installation

Download this repository as a ZIP archive and install it through the Arduino
IDE's **Sketch > Include Library > Add .ZIP Library** menu. The original vendor
examples are kept in the `origin` folder for reference.

## Usage

```cpp
#include <Wire.h>
#include <SSD1315.h>

SSD1315 display;                   // 7 pin connection, GPIO 8 connected to OLED RST signal
//SSD1315 display(12);             // 7 pin connection, GPIO 12 connected to OLED RST signal
//SSD1315 display(NO_RESET_PIN);   // 4 pin connection, no GPIO connection to OLED RST signal

void setup() {
    Wire.begin();
    display.begin();
    display.setRotation(1);      // rotate display 90 degrees
    display.setBrightness(0x7F); // medium brightness
    display.drawString(0, 0, "Hello", 5);
    display.display();
}

void loop() {
    // put the display to sleep to save power
    display.sleep(true);
    delay(1000);
    display.sleep(false);
    delay(1000);
}
```

See the `examples` directory for more complete demonstrations.
