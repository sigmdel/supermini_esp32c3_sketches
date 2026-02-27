/*
 *  See pulse_leds.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board
#define HAS_OLED
///
/// Time in milliseconds during which duty cycle value is used when pulsing LED.
#define LEVEL_PERIOD 50 
///
/// The increment or decrement in the duty cycle between levels when pulsing LED.
#define DELTA 10
///
/// The initial duty cycle above 0 when pulsing LED.
#define INIT_DELTA 5
///
///  Time in milliseconds during which the current io pin is active
#define ACTIVE_PERIOD 8000
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


/// -- pulsing LED with PWM ---

unsigned long delaytime = 0;
int delta = 5;
int fade = 0;
uint8_t ledPin = 255;

void pulse(void) {
  if (millis() - delaytime > LEVEL_PERIOD) {
    fade += delta;
    if (fade <= 0) {
      fade = 0;
      delta = DELTA;
    } else if (fade >= 255) {
      fade = 255;
      delta = - DELTA;
    }
    analogWrite(ledPin, fade);
    delaytime = millis();
  }
}

void initPulse(void) {
  pinMode(ledPin, OUTPUT);
  delta = 5;
  fade = 0;
  delaytime = millis();
}

int currentindex = -1; // initially no gpio pin is active
unsigned long activetimer = 0;

void nextPad(void) {
  if (currentindex >= 0) {
    // turn current gpio pin off 
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    pinMode(ledPin, INPUT);
    delay(INACTIVE_PERIOD);
  }
  if (currentindex >= padcount)
    currentindex = 0;
  else  
    currentindex++;
  if (currentindex == padcount) {
    ledPin = LED_BUILTIN;
    Serial.printf("\nThe intensity of the blue user LED (LED_BUILTIN, gpio pin %d) is being pulsed up and down.\n", ledPin);
  } else { 
    ledPin = iopins[currentindex];
    Serial.printf("\nThe intensity of a LED connected to board pad %s (gpio pin %d) is being pulsed up and down.\n", padlabels[currentindex], ledPin);
  }  
  initPulse();
  activetimer = millis();
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

  Serial.println("\n\nProject: pulse_leds");
  Serial.println("Purpose: Test pulse width modulation of all io pins");
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
  pulse();               
}
