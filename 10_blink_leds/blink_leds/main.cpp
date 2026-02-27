/*
 *  See blink_leds.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board
#define HAS_OLED
///
///  Time in milliseconds the LED is on during the heartbeat
#define ON_TIME  80
///
///  Time in millisecods the LED is off during the heartbeat
#define OFF_TIME 840
///
///  Time in milliseconds during which the current io pin is active
#define ACTIVE_PERIOD 6000
///
///  Time of inactivity in milliseconds between pins
#define INACTIVE_PERIOD 2000
///
//////////////////////////////////

#if !defined(ESP32)
  #error An ESP32 based board is required
#endif  

#if !ARDUINO_USB_CDC_ON_BOOT || ARDUINO_USB_CDC_ON_BOOT != 1
  #error Expected an ESP32 board with on board USB peripheral
#endif

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 


/// ---- board pad labels and io pin assignement ---- ////

#if defined(HAS_OLED)

// Checking ESP32C3 Super Mini with 0.42" OLED dev board with the USB 
// connector at bottom, ceramic antena at top. Probe each pad in a 
// clockwise fashion starting with the bottom left pad labelled 10.
const int padcount = 13;  
const char *padlabels[padcount] = {"10", "9", "8", "7", "6", "5", "4", "3",  // left edge
                                    "0", "1", "2", "TX", "RX"};              // right edge
const int iopins[padcount] = {10, 9, 8, 7, 6, 5, 4, 3,                       // left edge
                               0, 1, 2, 21, 20};                             // right edge

#else

// Checking ESP32-C3 Super Mini dev board with the USB connector at 
// bottom, ceramic antena at top. Probe each pad in a anti-clockwise 
// fashion starting with the bottom right pad labelled 5.
const int padcount = 13;  
const char *padlabels[padcount] = {"5", "6", "7", "8", "9", "10", "20", "21", // right edge
                                   "0", "1", "2", "3", "4"};                  // left edge
const int iopins[padcount] = {5, 6, 7, 8, 9, 10, 20, 21,                      // right edge
                              0, 1, 2, 3, 4};                                 // left edge

#endif

int ioPin = 255;
int ledOn = 1;
int currentindex = -1; // initially no gpio pin is active
unsigned long activetimer = 0;

void nextPad(void) {
  if (currentindex >= 0) {
    // turn current gpio pin off 
    digitalWrite(iopins[currentindex], LOW);
    pinMode(iopins[currentindex], INPUT);
    delay(INACTIVE_PERIOD);
  }
  if (currentindex >= padcount)
    currentindex = 0;
  else  
    currentindex++;
  if (currentindex == padcount) {
    ioPin = LED_BUILTIN;
    ledOn = 0;
    Serial.printf("\nThe blue user LED (LED_BUILTIN, gpio pin %d) is turned on and off.\n", ioPin);
  } else { 
    ioPin = iopins[currentindex];
    Serial.printf("\nThe board pad %s (gpio pin %d) is turned on and off.\n", padlabels[currentindex], ioPin);
    ledOn = 1;
  }  
  pinMode(ioPin, OUTPUT);
  digitalWrite(ioPin, ledOn);
  activetimer = millis();
}

unsigned long beattime = 0;
int beatcount = 0;
int beatdelay = 0;

/* beatcount
  0 = on short
  1 = off short
  2 = on short
  3 = off long
*/

void heartbeat(void) {
  if (millis() - beattime > beatdelay) {
    if ((beatcount & 1) == 1)
       digitalWrite(ioPin, 1-ledOn);
    else
       digitalWrite(ioPin, ledOn);
    if (beatcount == 3) {
      beatdelay = OFF_TIME;
      beatcount = 0;
    } else {
      beatdelay = ON_TIME;
      beatcount++;
    }
    beattime = millis();
  }
}

void setup() {
  #if !defined(SERIAL_BEGIN_DELAY)
    #if defined(PLATFORMIO)
      #define SERIAL_BEGIN_DELAY 5000    // 5 seconds
    #else
      #define SERIAL_BEGIN_DELAY 2000    // 2 seconds
    #endif
  #endif 
 
  Serial.begin(); 
  delay(SERIAL_BEGIN_DELAY);

  Serial.println("\n\nProject: blink_leds");
  Serial.println("Purpose: Test digitalWrite() and verify io pin assignments");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.print(ARDUINO_BOARD);
  #else 
  Serial.print("Unknown ESP32 board");
  #endif
  #ifdef HAS_OLED
    Serial.print(" with 0.42\" OLED");
  #endif  
  Serial.println();
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif
  Serial.println();
  nextPad();  
}

void loop() {
  if (millis() - activetimer > ACTIVE_PERIOD) {
    nextPad();
  }    
  heartbeat();               
}
