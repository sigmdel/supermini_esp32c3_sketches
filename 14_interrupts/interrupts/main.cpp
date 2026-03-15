/*
 *  See interrupts.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board
///#define HAS_OLED
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

struct Button {
  uint8_t PIN;  
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button = {BOOT_PIN, 0, false};
bool boot_button_pressed = false;

void ARDUINO_ISR_ATTR button_isr() {
  button.numberKeyPresses += 1;
  button.pressed = true;
}

void ARDUINO_ISR_ATTR boot_button_isr() {
  boot_button_pressed = true;
}


// Select the current gpio pin based on the currentindex into the
// above array.

int currentindex = -1;  // initially, no gpio pin is active

void nextIndex() {
  if (currentindex >= 0) {
    // detach interrupt from the current iopins[currentindex] 
    detachInterrupt(iopins[currentindex]);
    digitalWrite(iopins[currentindex], LOW);
    pinMode(iopins[currentindex], INPUT);
    Serial.printf("ISR detached from %s (gpio %d)", padlabels[currentindex], iopins[currentindex]);
    delay(100);
  }
  currentindex++;
  if (iopins[currentindex] == BOOT_PIN) {
    Serial.printf("\nAn ISR will not be attached to pin %s (gpio %d) because it is the pin connnected to the BOOT button",
      padlabels[currentindex], BOOT_PIN);
    currentindex++;
  }
  if (currentindex >= padcount) 
    currentindex = 0;

  button.PIN = iopins[currentindex];
  Serial.printf("\n\nAn ISR has been attached to pin %s (gpio %d) for testing.\n", padlabels[currentindex], button.PIN);
  pinMode(button.PIN, INPUT_PULLUP);
  button.numberKeyPresses = 0;
  attachInterrupt(button.PIN, button_isr, FALLING); 
}  

void printlabel(int pin) {
  for (int i=0; i < padcount; i++) {
    if (iopins[i] == pin) {
      Serial.print(padlabels[i]);
      return;
    }  
  }
  Serial.print("unknown");
}

//----------------------------------------------

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

  Serial.println("\n\nProject: interrupt");
  Serial.println("Purpose: Check the interrupt capabilities of the GPIO pins of the Super Mini");
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

  Serial.println("An interrupt service routine (ISR) will be attached to each gpio pin in turn.");
  Serial.println("Probe the pin with a grounded wire to test the interrupt."); 
  Serial.println("Press the boot button to move on to the next pin.");
  Serial.printf("Do not probe pad %d, (gpio %d), that is the i/o pin of the BOOT button.\n", BOOT_PIN, BOOT_PIN);
  
  delay(2000); // time to read above message

  // start process
  nextIndex();
  attachInterrupt(BOOT_PIN, boot_button_isr, FALLING);
}

unsigned long timer;

void loop() {
  if (button.pressed) {
    Serial.print("Interrupt on pin ");
    printlabel(button.PIN);
    Serial.printf(" (gpio %d) has been raised %lu times\n", button.PIN, button.numberKeyPresses);
    button.pressed = false;
    button.numberKeyPresses = 0;
  }
  if (boot_button_pressed) {
    nextIndex();
    boot_button_pressed = false;
    delay(1000);
  }
}
