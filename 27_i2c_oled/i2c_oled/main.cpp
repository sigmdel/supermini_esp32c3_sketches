/*
 *  See i2c_oled.ino for license and attribution.
 */

#include <Arduino.h>
#include <Wire.h>
#include "SSD1315.h"

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board.
#define HAS_OLED
///
///  if ALT_SDA is not defined then SDA = 8 will be used.
///  if ALT_SCL is not defined then SCL = 9 will be used.
///  SDA and SCL are defined in variant pins_arduino.h
///  The onboard OLED display SDA and SCL signals are hardwired 
///  gpio 5 and 6 respectively.
#define ALT_SDA 5
#define ALT_SCL 6
///
///  Scan for I2C devices before testing DS3231 real-time clock 
///  and, optionally, an onboard AT24C32 I2C EEPROM
#define SCAN_12C_BUS
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
#define SERIAL_BEGIN_DELAY 8000
///
//////////////////////////////////

#ifdef ALT_SDA
  #define OLED_SDA  ALT_SDA
#else
  #define OLED_SDA  SDA
#endif

#ifdef ALT_SCL
  #define OLED_SCL  ALT_SCL
#else
  #define OLED_SCL  SCL
#endif

static const uint8_t oledSDA = OLED_SDA;
static const uint8_t oledSCL = OLED_SCL;


// Scan I2C bus for devices
//   Wire.setPins(sda, scl) must be used before if not using SDA and SCL as defined in 
//   Source: .../.platformio/packages/framework-arduinoespressif32/libraries/Wire/examples/WireScan/WireScan.ino
//
void i2cScan(void) {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
}

void errorRestart(const char* msg) {
  if (strlen(msg))
    Serial.println(msg);
  Serial.println("Restarting in 5 seconds");
  delay(5000);
  ESP.restart();
}

// Create display object specifying that only the SDA and SCL signals are
// connected. Presumably the RES (reset) pin of the onboard OLED is always disabled.
SSD1315 display(NO_RESET_PIN);

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

  Serial.println("\n\nProject: i2c_oled");
  Serial.println("Purpose: Test the I2C display of an ESP32-C3 Super Mini with 0.42\" OLED display");
  Serial.print("  Board: ");
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

  #ifdef HAS_OLED 
  if (oledSDA != 5)
    Serial.printf("Warning: Expected gpio 5 as SDA pin not %d\n", oledSDA);
  if (oledSCL != 6)
    Serial.printf("Warning: Expected gpio 6 as SCL pin not %d\n", oledSCL);
  #endif  

  Serial.println("Initializing Wire (I2C)");

  if (Wire.setPins(oledSDA, oledSCL)) 
    Serial.printf("Wire SDA and SCL signals attached to GPIO pins %d and %d respectively\n", oledSDA, oledSCL);  
  else {
    Serial.printf("Cannot attach GPIO pins %d (SDA) and %d (SCL) to Wire object.\n", oledSDA, oledSCL);
    errorRestart("");
  }
    
  if (Wire.begin()) 
    Serial.println("Wire (I2C) initialized");
  else 
    errorRestart("Wire (I2C) initialization failed");

  i2cScan();

  Serial.println("Initializing the OLED display");
  display.begin();
  //display.setRotation(0);       // default, top edge of the display is nearest the ceramic antenna
  //display.setRotation(1);       // rotate display 90 degrees - will only see half of the [ Hello! ] box
  //display.setRotation(2);       // rotate display 180 degrees - will only see half of the [ Hello! ] box
  //display.setRotation(3);       // rotate display 270 degrees - will only see half of the [ Hello! ] box
  //display.setBrightness(0x7F);  // medium brightness - default on reset/powering up
  display.drawString(20, 15, "Hello!", 5);
  display.drawRect(1, 1, 70, 38); //fill=false, invert=false)
  display.display();
}

const uint8_t levels = 4;
const uint8_t contrast[levels] = {31, 63, 127, 255};

void testContrast(void) {
  for (int i=0; i < levels; i++) {
    uint8_t cval = contrast[i];
    display.setBrightness(cval);
    Serial.printf("  Contrast: %3d (0x%02x)\n", cval, cval);
    delay(3000);
  }  
  display.setBrightness(127); // back to default
}

void loop() {
  Serial.println("\nDisplay: Normal");
  testContrast();

  // put the display to sleep to save power
  Serial.println("Display: Sleeping");
  display.sleep(true);
  delay(5000);

  // back to normal for a short while
  Serial.println("Display: Normal");
  display.sleep(false);
  delay(1000);

  // before inverting display
  Serial.println("Display: Inverted");
  display.invert(true);
  testContrast();
  display.invert(false);
}
