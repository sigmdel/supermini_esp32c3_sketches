/*
 *  See i2c_ds3231.ino for license and attribution.
 */

#include <Arduino.h>
#include <Wire.h>
#include <DS3231_Simple.h>
#include "compile_time.h"

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board.
///#define HAS_OLED
///
///  By default the Wire library attaches pins SDA and SCL (defined in 
///  the variant pins_arduino.h file) to the single ESP32-C3 I2C 
///  controller. This can be changed if necessary, such as on some
///  ESP32-C3 Super-Mini-like boards with a 0.42" OLED where 
///  gpio pins 5 and 6 are connected to the OLED SDA and SCL.
///#define ALT_SDA 5
///#define ALT_SCL 6
///
///  Scan for I2C devices before testing DS3231 real-time clock 
///  and, optionally, an onboard AT24C32 I2C EEPROM
#define SCAN_12C_BUS
///
///  If this macro is defined, the DS3232 date and time will be
///  set to the the sketch's compile date and time no matter
///  what. If the macro is not defined, the RTC date and time 
///  is set to the compile date and time unless the
///  RTC is already running (i.e. has a year value not equal 
///  to 0) and then the user can choose to manually enter 
///  a date and time.
#define USE_COMPILE_TIME
///
///  Time between printing of the date/time to serial monitor
///  in seconds (allowed values: 1 to 20 seconds).
#define CLOCK_INTERVAL 5
///  
///  Define analogue input pin to read when testing the AT24C32
///  EEPROM  on the RTC module. Do not define if the RTC module 
///  does not have an onboard EEPROM or to skip the EEPROM
///  read and write test.
#define ANALOG_PIN A3
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


#if !defined(CONFIG_IDF_TARGET_ESP32C3)
  #error An ESP32-C3 SoC is required
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

void errorRestart(const char* msg) {
  if (strlen(msg))
    Serial.println(msg);
  Serial.println("Restarting in 5 seconds");
  delay(5000);
  ESP.restart();
}  

// Scan I2C bus for devices
//   Wire.setPins(sda, scl) must be used before if not using SDA and SCL as defined in 
//   Source: .../.platformio/packages/framework-arduinoespressif32/libraries/Wire/examples/WireScan/WireScan.ino
//
void i2cScan(void) {
  Serial.println("\nScanning for I2C devices ...");
  // Wire.setPins(ALT_SDA, ALT_SCL);
  if (Wire.begin()) 
    Serial.println("Wire (I2C) initialized");
  else 
    errorRestart("Wire (I2C) initialization failed");
  delay(1000);

  // do scan
  byte error, address;
  int nDevices = 0;
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
  Wire.end();  // because DS3231::begin() starts Wire !
  Serial.println();
}
 

DS3231_Simple Clock;

#ifdef ANALOG_PIN
void dumpLog() {
  // Taken from DataLogger.ino in DS3231_Simple examples

  uint16_t loggedData;
  DateTime loggedTime;
  
  // Note that reading a log entry also deletes the log entry
  // so you only get one-shot at reading it, if you want to do
  // something with it, do it before you discard it!
  unsigned int x = 0;
  while (Clock.readLog(loggedTime,loggedData)) {
    if(x == 0) Serial.printf(F("\nDate, analogRead(A%d)\n"), ANALOG_PIN);   
    x++;
    Clock.printTo(Serial,loggedTime);
    Serial.print(", ");
    Serial.println(loggedData);
  }
  Serial.print(F("Number of log entries found: "));
  Serial.println(x);
  Serial.println();
}
#endif

void setClockToCompileTime(void) {
  DateTime compTime;
  compTime.Second = BUILD_SEC;  // 0-59 = 6 bits
  compTime.Minute = BUILD_MIN;  // 0-59 = 6 bits
  compTime.Hour = BUILD_HOUR;   // 0-23 = 5 bits
  compTime.Dow = 0;             // 1-7  = 3 bits (Day Of Week)
  compTime.Day = BUILD_DAY;     // 1-31 = 5 bits
  compTime.Month = BUILD_MONTH; // 1-12 = 4 bits
  compTime.Year = BUILD_YEAR;   // 0-199 = 8 bits + 2000 base
  Clock.write(compTime);
  Serial.print("DS3231 time set to the sketch compile time: ");
  Clock.printTo(Serial, compTime);
  Serial.println();
}

void setClockTime(void) {
  #ifdef USE_COMPILE_TIME
  setClockToCompileTime();
  #else
  char buffer[2] = { 0 };          

  DateTime now = Clock.read();
  if (now.Year == 0)
    setClockToCompileTime();
  Serial.print("Current DS3231 date and time: ");
  Clock.printTo(Serial);

  while (true) {   
    delay(250);
    while (Serial.available()) Serial.read(); // clear buffer
    Serial.print("\nDo you want to set the date and time manually (Y/N)? ");
    while (!Serial.available()) ; // wait for 1 byte
    Serial.readBytes(buffer, 1);
    Serial.println();
    delay(250);
    while (Serial.available()) Serial.read(); // clear buffer
    if ((buffer[0] == 'n') || (buffer[0] == 'N')) {
      return;
    }  
    if ((buffer[0] == 'y') || (buffer[0] == 'Y')) {
      Clock.promptForTimeAndDate(Serial);
      return;
    }
  }
  #endif
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

  Serial.println("\n\nProject: i2c_ds3231");
  Serial.println("Purpose: Test the I2C peripheral with an RTC/EEPROM module");
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

  int inter = CLOCK_INTERVAL;
  if (inter < 1) 
    errorRestart("CLOCK INTERVAL must be >= 1 (second)");
  else if (inter > 20)
    errorRestart("CLOCK INTERVAL must be <= 20 (seconds)");
 
  #if defined(ALT_SDA) && defined(ALT_SCL) && ((ALT_SDA != SDA) || (ALT_SCL != SCL))
  Serial.printf("The gpio pins attached to the data and clock signals of the I2C controller are %d and %d respectively\n", ALT_SDA, ALT_SCL);
  Serial.printf("Note that SDA and SCL defined in 'pins_arduino.h' are %d and %d respectively\n", SDA, SCL);
  Wire.setPins(ALT_SDA, ALT_SCL);
  #else
  Serial.printf("The defined SDA (gpio %d) and SCL (gpio %d) pins are attached to the data and clock signals of the I2C controller by default\n", SDA, SCL);
  #endif
  
  #ifdef SCAN_12C_BUS
  i2cScan();
  delay(1000);
  #endif

  Clock.begin();
  setClockTime(); 
  Serial.println();

  #ifdef ANALOG_PIN
  // Test EEPROM
  Clock.disableAlarms(); 
  Clock.formatEEPROM();
  Clock.setAlarm(DS3231_Simple::ALARM_EVERY_SECOND);  
  Serial.printf(F("Logging value of analogRead(A%d), enter any character to dump the log.\n"), ANALOG_PIN);
  #endif
}

void printClockDateTime() {
  Clock.printDateTo_YMD(Serial);
  Serial.print(" ");
  Clock.printTimeTo_HMS(Serial);
  Serial.println();
}

int counter = CLOCK_INTERVAL; 

void loop() {
#ifdef ANALOG_PIN
  if (Clock.checkAlarms()) {
    uint16_t data = analogRead(A1);
    Clock.writeLog(data);
    Serial.printf("A%d in: %d\n", ANALOG_PIN, data);
    counter--;
    if (counter <= 0) {
      printClockDateTime();
      counter = CLOCK_INTERVAL;
    }  
  } 

  if(Serial.available()) {
    while(Serial.available()) Serial.read();
    dumpLog();
  }
#else
  delay(CLOCK_INTERVAL*1000);
  printClockDateTime();
#endif
}
