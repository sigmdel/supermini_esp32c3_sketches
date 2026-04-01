/*
 *  See poll.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board
#define HAS_OLED
///
///  Rate of USB to Serial chip if used on the development board.
///  This is ignored when the native USB peripheral of the 
///  ESP SoC is used.
#define SERIAL_BAUD 115200
///
///  Time in milliseconds to wait after Serial.begin() in 
///  the setup() function. If not defined, it will be set
///  to 5000 if running in the PlaformIO IDE to manually switch
///  to the serial monitor otherwise to 2000 if an native USB 
///  peripheral is used or 1000 if a USB-serial adpater is used.
///#define SERIAL_BEGIN_DELAY 8000
///
//////////////////////////////////


#if !defined(CONFIG_IDF_TARGET_ESP32C3)
  #error An ESP32-C3 SoC is required
#endif  

#if !ARDUINO_USB_CDC_ON_BOOT || ARDUINO_USB_CDC_ON_BOOT != 1
  #error Expected an ESP32 board with on board USB peripheral
#endif

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 


/// ---- board pad labels and io pin assignement ---- ////

const int padcount = 13;  

#if defined(HAS_OLED)

const bool hasOLED = true;

// Checking ESP32C3 Super Mini with 0.42" OLED dev board with the USB 
// connector at bottom, ceramic antena at top. Probe each pad in a 
// clockwise fashion starting with the bottom left pad labelled 10.
const char *padlabels[padcount] = {"10", "9", "8", "7", "6", "5", "4", "3",  // left edge
                                    "0", "1", "2", "TX", "RX"};              // right edge
const int iopins[padcount] = {10, 9, 8, 7, 6, 5, 4, 3,                       // left edge
                               0, 1, 2, 21, 20};                             // right edge

#else

const bool hasOLED = false;

// Checking ESP32-C3 Super Mini dev board with the USB connector at the
// bottom, ceramic antena at top. Probe each pad in a anti-clockwise 
// fashion starting with the bottom right pad labelled 5.
const char *padlabels[padcount] = {"5", "6", "7", "8", "9", "10", "20", "21", // right edge
                                   "0", "1", "2", "3", "4"};                  // left edge
const int iopins[padcount] = {5, 6, 7, 8, 9, 10, 20, 21,                      // right edge
                              0, 1, 2, 3, 4};                                 // left edge
#endif

// the i/o level of each pin afte its mode has been set.
// Pins 2, 8 and 9 are pulled high by an external resistor which overrides INPUT_PULLDOWN 
int levels[padcount];

bool polled[padcount] = {false};  // set true if successfully polled (if digitalRead(i) != levels[i] at least once)
int polledcount = 0;              // number of pins successfully polled
int expectedcount = 0;             // padcount when INPUT_PULLUP, padcount-3 when INPUT_PULLDOWN

int testMode = -1;     // test 0 = INPUT_PULLUP, 1 = INPUT_PULLDOWN, 2 = OUTPUT (all pins HIGH), 3 = OUTPUT (all pins LOW) 
int currentindex = 0;


void nextTestMode(void) {
  testMode++;
  if (testMode > 3) testMode = 0;

  uint8_t mode;
  if (testMode >= 2) 
    mode = OUTPUT;
  else if (testMode == 0)
    mode = INPUT_PULLUP;
  else
    mode = INPUT_PULLDOWN;  
    
  // set the mode of all the io pins
  for (int i=0; i < padcount; i++) {
    pinMode(iopins[i], mode);  
    polled[i] = false;
  }  
  polledcount = 0;
  currentindex = 0;

  Serial.print("\n\nThe mode of all I/O pins set to ");
  if (mode == INPUT_PULLUP) {
    Serial.print("INPUT_PULLUP");
    expectedcount = padcount;
  }  else if (mode == INPUT_PULLDOWN) {
    Serial.print("INPUT_PULLDOWN");   
    expectedcount = padcount - 3 - (uint8_t)hasOLED;
  } else if (mode == OUTPUT)  {
    Serial.print("OUTPUT");
    expectedcount = padcount;
  }  
  Serial.println(".");  
 
  if (testMode == 2) 
    Serial.println("All pins set to HIGH");
  else if (testMode == 3)
    Serial.println("All pins set to LOW");

  delay(50);
  for (int i=0; i<padcount; i++) {
    if (mode == OUTPUT) {
      digitalWrite(iopins[i], (testMode == 2));
      delay(3);
    }  
    levels[i] = digitalRead(iopins[i]);
    Serial.printf("Pad %2s (gpio %2d) is currently %s\n", padlabels[i], iopins[i], (levels[i] ? "HIGH" : "LOW"));
  }

  Serial.print("Probe pins with a connection to ");
  if ((testMode == 0) || (testMode == 2))
    Serial.println("GND");
  else 
    Serial.println("Vcc (3.3V MAXIMUM)");   
  if (expectedcount == padcount) 
    Serial.printf("All %d pins can be polled", expectedcount);
  else {
    Serial.printf("%d pins can be polled: ", expectedcount);
    int should_be = ((testMode == 0) || (testMode == 2));
    bool not_first = false;
    for (int i=0; i < padcount; i++) {
      if (levels[i] == should_be) {
        if (not_first) Serial.print(", ");
        Serial.printf("%d", iopins[i]);
        not_first = true;
      }  
    }
  }   
}

void setup() {
  #if (ARDUINO_USB_CDC_ON_BOOT > 0)
  Serial.begin();
  #else 
  Serial.begin(SERIAL_BAUD);
  #endif  

  #if !defined(SERIAL_BEGIN_DELAY)
    #if defined(PLATFORMIO)
      #define SERIAL_BEGIN_DELAY 5000    // 5 seconds
    #else
      #define SERIAL_BEGIN_DELAY 2000    // 2 seconds
    #endif
  #endif 
  delay(SERIAL_BEGIN_DELAY);

  Serial.println("\n\nProject: poll");
  Serial.println("Purpose: Poll all digital i/o pins of the Super Mini");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.print(ARDUINO_BOARD);
  #else 
  Serial.print("Unknown ESP32-C3 based board");
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
  Serial.println("\nAll the pins are continuously polled sequentially for a change in their value.");
  Serial.println("The program is in one of four test states:");
  Serial.println("  1 - all pins are in INPUT_PULLUP mode");
  Serial.println("  2 - all pins are in INPUT_PULLDOWN mode (*)");
  Serial.println("  3 - all pins are in OUTPUT_MODE and set to HIGH");
  Serial.println("  4 - all pins are in OUTPUT_MODE and set to LOW");
  Serial.println("The next test state starts when all the pins have been polled in the current test state.");
  Serial.printf("  (*) An external resitor pulls %s pins high so that they cannot be polled.\n", (hasOLED) ? "four" : "three" );
  delay(4000); // time to read above message

testMode = 1;
  nextTestMode();
  Serial.println(".\n");
}

void loop() {
  if (polledcount >= expectedcount) {
    delay(2000);
    nextTestMode();
    delay(2000);
    Serial.println(".\n");
  }

  int level = digitalRead(iopins[currentindex]);
  // check if the level has changed
  if (level != levels[currentindex])  {
    levels[currentindex] = level;
    if (!polled[currentindex]) {
      polled[currentindex] = true;
      polledcount++;
      Serial.printf("Pin %s (gpio %d) is %s, ", padlabels[currentindex], iopins[currentindex], (level) ? "HIGH" : "LOW");
      Serial.printf("%d/%d pins polled\n", polledcount, expectedcount);
    }  
    delay(100);
  }
  currentindex = (++currentindex)%padcount;
}
