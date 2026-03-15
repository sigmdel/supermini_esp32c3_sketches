/*
 *  See interrupts2.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board
///#define HAS_OLED
///
///  Define this macro to see the user LED flash for 25 ms
///  whenever a button ISR is executed
///#define FLASH_IN_ISR
///
///  Define this macro to handle button release bounce
///  will make interrupt type LOW ineffective
///#define EXTRA_TEST
///
///  Define this macro to print a seconds time stamp (with 
///  millisecond precision) at the start of each button event  
///#define TIME_STAMP
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
  uint8_t pinIndex;  
  unsigned long actTime;
  unsigned long debounceTime;
  bool pressed;
  uint32_t counter;
};

Button buttons[padcount];

void ARDUINO_ISR_ATTR button_isr(void *arg) { 
  Button *btn = static_cast<Button *>(arg);
  btn->pressed = true;
  btn->actTime = millis();
  btn->counter++;
  #ifdef FLASH_IN_ISR
    digitalWrite(LED_BUILTIN, LOW);
    while (millis() - btn->actTime < 25) {/* do nothing*/};
    digitalWrite(LED_BUILTIN, HIGH);
  #endif
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

  Serial.println("An interrupt service routine (ISR) will be attached to each gpio pin.");
  #ifdef FLASH_IN_ISR
  Serial.printf("Probe any pin except i/o %d (connected to the user LED) with a grounded wire to test the interrupt.\n", LED_BUILTIN); 
  #else
  Serial.println("Probe any pin with a grounded wire to test the interrupt."); 
  #endif

  int mode;
  for (uint8_t ndx=0; ndx < padcount; ndx++) {
    #ifdef FLASH_IN_ISR
    if (iopins[ndx] != LED_BUILTIN) {
    #endif  
      if (ndx > 8) {
        if (ndx == 9) Serial.print("\nPins with RISING interrupt mode:");
        Serial.printf(" %s", padlabels[ndx]);
        mode = RISING;
      } else if (ndx > 4) {
        if (ndx == 5) Serial.print("\nPins with LOW interrupt mode:");
        Serial.printf(" %s", padlabels[ndx]);
        mode = ONLOW;  
      } else {
        if (ndx == 0) Serial.print("\nPins with FALLING interrupt mode:");
        Serial.printf(" %s", padlabels[ndx]);
        mode = FALLING;       
      }
        pinMode(iopins[ndx], INPUT_PULLUP);
        buttons[ndx].pinIndex = ndx;
        buttons[ndx].debounceTime = 200; // 100 ms debounce time
        buttons[ndx].pressed = false;
        attachInterruptArg(iopins[ndx], button_isr, &buttons[ndx], mode); 
    #ifdef FLASH_IN_ISR 
    }
    #endif
  }
  Serial.println("\n");

  #ifdef FLASH_IN_ISR
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  #endif
}

uint8_t ndx = 0;

void loop() {
  if (buttons[ndx].pressed) {
    #ifdef EXTRA_TEST
    if ((millis() - buttons[ndx].actTime > buttons[ndx].debounceTime) && digitalRead(iopins[buttons[ndx].pinIndex])) {
    #else
    if (millis() - buttons[ndx].actTime > buttons[ndx].debounceTime) {
    #endif
      #ifdef TIME_STAMP      
      unsigned long time = millis();
      Serial.printf("%07lu.%03d  ", time/1000, time%1000);
      #endif
      int pi = buttons[ndx].pinIndex;
      Serial.printf("Pad %s (gpio %d) has been pressed (counter = %d)\n", padlabels[pi], iopins[pi], buttons[ndx].counter);
      buttons[ndx].pressed = false;
      buttons[ndx].counter = 0;
    }
  }
  ndx++;
  if (ndx >= padcount)
    ndx = 0;
}
