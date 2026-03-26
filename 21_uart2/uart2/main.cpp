/*
 *  See uart2.ino for license and attribution.
 */

#include <Arduino.h>
#include "esp32-hal-uart.h"

//////// User configuration //////
///
/// Time between transmission of messages via SERIAL0 in milliseconds
#define WRITE_INTERVAL0  2000  
///
/// Time between transmission of messages via SERIAL1 in milliseconds
#define WRITE_INTERVAL1  3000  
///  
///  By default Serial0 tx and rx pins are assigned to RX and TX as 
///  defined in pins_arduino.h
///
///  Define the Serial1 tx and rx pins with the SER1_TX and SER1_RX macros. 
///  Possible values are 0 to 10 inclusively. However using two pins
///  in the [0, 1, 2, 3, 10] will leave the default SPI and I2C pins available
///#define SER1_TX 10     // default
///#define SER1_RX  0     // default
///
///  Define this macro if the loopback connections of the two hardware serial
///  devices are cross connected:
///      Serial0 TX connected to Serial1 RX and
///      Serial1 TX connected to Serial0 RX
///  If not defined then SERIALnTX and SERIALnRX (n=0 and 1) are
///  connected together.
///#define CROSS_UARTS
///
/// Bauds used for (cross) loopback tests. 
/// "When using the 80 MHz input clock, the UART controller supports a maximum baud rate of 5 Mbaud."
/// ESP32-C3 Technical Reference Manual (Version 1.3), Section 26.4.3.1 Baud Rate Generation, p.553
///
/// Select the baud of each UART controller. If CROSS_UARTS is defined, SERIAL0_BAUD will be used 
/// for both the Serial0 and Serial1 devices. The default is 5Mbaud 5242880 for both.
///
///#define SERIAL0_BAUD  5242880  
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
///#define SERIAL1_BAUD  5242880  
#define SERIAL1_BAUD   921600 
///#define SERIAL1_BAUD   576000 
///#define SERIAL1_BAUD   460800 
///#define SERIAL1_BAUD   230400 
///#define SERIAL1_BAUD   115200 
///#define SERIAL1_BAUD    76800 
///#define SERIAL1_BAUD    57600 
///#define SERIAL1_BAUD    28800 
///#define SERIAL1_BAUD     9600
///
///  Define this macro to get information about the default RX and 
///  TX pin assignement of Serial1
///#define PRINT_DEFAULT_SERIAL1_PINS
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

#if !defined(SER1_RX)
  #define SER1_RX 0
#endif

#if !defined(SER1_TX)
  #define SER1_TX 10
#endif  

#if SER1_RX == SER1_TX
  #error Serial1 RX and TX pins must be different
#endif

#if SER1_RX == 20 || SER1_RX == 21 || SER1_TX == 20 || SER1_TX == 21
  #error Serial1 RX and TX pins must be different from Serial0 RX and TX pins
  // can't use TX and RX in above tests 
#endif  

#ifndef SERIAL0_BAUD
  #warning "SERIAL0_BAUD not defined, default 5242880 baud used"
  #define SERIAL0_BAUD  5242880
#endif

#ifndef SERIAL1_BAUD
  #define SERIAL1_BAUD SERIAL0_BAUD
#endif

#if defined(CROSS_UARTS) && (SERIAL1_BAUD != SERIAL0_BAUD)
  #undef SERIAL1_BAUD 
  #define SERIAL1_BAUD SERIAL0_BAUD
  #warning "SERIAL1_BAUD reset equal to SERIAL0_BAUD"
#endif  

// Assigning UART controllers and i/o pins
//
//   Should not use TX1 and RX1 as in the makergo_c3_supermini/pins_arduino.h
//   because these are defined as 19 and 18 respectively in esp32/HardwareSerial.h
//   (~/.platformio/packages/framework-arduinoespressif32/cores/esp32/HardwareSerial.h)
//
// Serial0 connects to the UART0 controller with the Arduino assigned pins 
#define Ser0_TX TX // 21 = U0TXD  can be modified
#define Ser0_RX RX // 20 = U0RXD  can be modified
//
// Serial1 connects to the UART1 controller with the following assigne i/o pins
#define Ser1_TX SER1_TX
#define Ser1_RX SER1_RX

unsigned long writetimer0;
unsigned long writetimer1;

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
  Serial.println("Purpose: Test UART peripherals with loop back tests");
  Serial.print("  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32-C3 based board");
  #endif
  Serial.println();

  #ifdef PRINT_DEFAULT_SERIAL1_PINS
    Serial.printf("The Serial1 default Rx and Tx pins before Serial1.begin(BAUD) are %d and %d respectively.\n", uart_get_RxPin(1), uart_get_TxPin(1));
    // Default initialization will break the Serial (USB CDC) port by grabbing pins 18 and 19 normally connected to USB D- and USB D+
    Serial1.begin(SERIAL1_BAUD);
    // So save the pin used for Rx and Tx
    int rxp = uart_get_RxPin(1);
    int txp = uart_get_TxPin(1);
    // disable the port to release pins 18 and 19
    Serial1.end();
    // Re-initialize the Serial (USB CDC) port
    Serial.begin();
    delay(5000);
    Serial.printf("The Serial1 Rx and Tx pins after Serial1.begin(BAUD) are %d and %d respectively.\n\n", rxp, txp);
  #endif

  #ifdef CROSS_UARTS
  Serial.println("Since CROSS_UARTS is defined, the two serial ports should be cross connected:");
  Serial.printf("  Connect Serial0 Tx (GPIO %d) to Serial1 Rx (GPIO %d)\n", Ser0_TX, Ser1_RX);
  Serial.printf("  Connect Serial1 Tx (GPIO %d) to Serial0 Rx (GPIO %d)\n", Ser1_TX, Ser0_RX); 
  #else
  Serial.println("Since CROSS_UARTS is not defined, the serial port loopback connections should be independant:");
  Serial.printf("  Connect Serial0 Tx (GPIO %d) to Serial0 Rx (GPIO %d)\n", Ser0_TX, Ser0_RX);
  Serial.printf("  Connect Serial1 Tx (GPIO %d) to Serial1 Rx (GPIO %d)\n", Ser1_TX, Ser1_RX);
  #endif

  Serial.println();

  //Serial0.begin(SERIAL0_BAUD, SERIAL_8N1, Ser0_RX, Ser0_TX);
  Serial0.begin(SERIAL0_BAUD);
  while (!Serial0) delay(10);
  Serial.printf("Hardware serial port (Serial0) initialized with its baud set to %lu\n", Serial0.baudRate());

  Serial1.begin(SERIAL1_BAUD, SERIAL_8N1, Ser1_RX, Ser1_TX);
  while (!Serial1) delay(10);
  Serial.printf("Hardware serial port (Serial1) initialized with its baud set to %lu\n", Serial1.baudRate());
 
  Serial.println();

  if (Serial0.available()) {
    Serial.print("Clearing Serial0 RX buffer");
    while (Serial0.available()) {
      Serial0.read();
      Serial.write('.');
    }
    Serial.println();
  }
 
  if (Serial1.available()) {
    Serial.print("Clearing Serial1 RX buffer");
    while (Serial1.available()) {
      Serial1.read();
      Serial.write('.');
    }
    Serial.println();  
  }

  Serial.println("setup() completed, starting loop().");
  writetimer0 = millis();
  #ifdef DUAL_UARTS  
  writetimer1 = millis();
  #endif
}

int count = 0;

void loop() {
  if (Serial0.available()) {
    Serial.write("  Serial0 RX: ");
    while (Serial0.available()) {
      Serial.write(Serial0.read());
      #ifdef RX_DELAY
      delay(RX_DELAY);
      #endif
    }
  }

  if (Serial1.available()) {
    Serial.write("  Serial1 RX: ");
    while (Serial1.available()) {
      Serial.write(Serial1.read());
      #ifdef RX_DELAY
      delay(RX_DELAY);
      #endif
    }
  }

  if (millis() - writetimer0 > WRITE_INTERVAL0) {
    count++;
    Serial.printf("\nTransmitting: From Serial0, message #%d\n", count);
    Serial0.printf("From Serial0, message #%d\n", count);
    writetimer0 = millis();
  }

  if (millis() - writetimer1 > WRITE_INTERVAL1) {
    count++;
    Serial.printf("\nTransmitting: From Serial1, message #%d\n", count);
    Serial1.printf("From Serial1, message #%d\n", count);
    writetimer1 = millis();
  }
}
