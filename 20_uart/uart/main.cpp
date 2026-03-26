/*
 *  See uart.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
/// Time between transmission of messages in milliseconds
#define WRITE_INTERVAL  2000  
///
///  "When using the 80 MHz input clock, the UART controller supports a maximum baud rate of 5 Mbaud."
///  ESP32-C3 Technical Reference Manual (Version 1.3), Section 26.4.3.1 Baud Rate Generation, p.553                 
///  Select one of the following
///#define SERIAL0_BAUD  5242880    // default
///#define SERIAL0_BAUD   921600 
///#define SERIAL0_BAUD   576000 
///#define SERIAL0_BAUD   460800 
///#define SERIAL0_BAUD   230400 
///#define SERIAL0_BAUD   115200 
///#define SERIAL0_BAUD    76800 
///#define SERIAL0_BAUD    57600 
///#define SERIAL0_BAUD    28800 
///#define SERIAL0_BAUD     9600
///
///  Delay in milliseconds after receiving each character before checking if a new character is available.
///  Try using SERIAL0_BAUD 9600 without defining this macro to see what happens
///#define RX_DELAY 1
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

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

#ifndef SERIAL0_BAUD
  #warning "SERIAL0_BAUD not defined, default 5242880 baud used"
  #define SERIAL0_BAUD  5242880
#endif

unsigned long writetimer;

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

  Serial.println("\n\nProject: uart");
  Serial.println("Purpose: Test UART peripheral with a loop back test");
  Serial.print("  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32-C3 based board");
  #endif
  Serial.println();

  Serial.printf("Connect the Serial0 Rx (GPIO %d) and the Serial0 Tx (GPIO %d) pins together.\n", RX, TX);
  //Serial0.begin(SERIAL0_BAUD, SERIAL_8N1, RX, TX);
  //Serial0.begin(SERIAL0_BAUD, SERIAL_8N1, -1, -1);
  //Serial0.begin(SERIAL0_BAUD, SERIAL_8N1);
  Serial0.begin(SERIAL0_BAUD);
  while (!Serial0) delay(10);
  Serial.println();
  Serial.printf("Hardware serial port (Serial0) initialized with its baud set to %lu.\n", Serial0.baudRate());

  Serial.println();

  if (Serial0.available()) {
    Serial.print("Clearing Serial0 RX buffer");
    while (Serial0.available()) {
      Serial0.read();
      Serial.write('.');
    }
    Serial.println();
  }
  Serial.println("Initializing the user LED");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn the user LED off

  Serial.println("setup() completed, starting loop().");
  writetimer = millis();
}

int count = 0;

void loop() {
  if (Serial0.available()) {
    Serial.printf("    Received: ");
    while (Serial0.available()) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.write(Serial0.read());
      #ifdef RX_DELAY
      delay(RX_DELAY);
      #endif
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }

  if (millis() - writetimer > WRITE_INTERVAL) {
    count++;
    Serial.printf("\nTransmitting: message #%d\n", count);
    Serial0.printf("message #%d\n", count);
    writetimer = millis();
  }
}
