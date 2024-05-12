// Main module of pin_mames
// Copyright: see notice in pin_names.ino

#include <Arduino.h>

int TRUE_LED_PIN = 8;    // modify if needed in the following if/else ladder

// Define the title macro and fill in BOOT_BUILTIN as needed for each board

#if defined(ARDUINO_MAKERGO_C3_SUPERMINI)
  #define TITLE "MakerGO C3 SuperMini"
  //static const uint8_t BOOT_BUILTIN = 9;          // defined in pins_arduino.h
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_NOLOGO_ESP32C3_SUPER_MINI)
  #define TITLE "Nologo ESP32C3 Super Mini"
  static const uint8_t BOOT_BUILTIN = 9;
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_SUPER_MINI_C3)
  #define TITLE "DfRobot Beetle ESP32-C3"  //"Modified ESP32 C3 DEVKITC 02"
  static const uint8_t BOOT_BUILTIN = 9;
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_SUPER_MINI_ESP32C3)
  #define TITLE "Custom ESP32C3 Super Mini"
  //static const uint8_t BOOT_BUILTIN = 9;          //defined in pins_arduino.h
  //#define BOOT_BUILTIN BOOT_BUILTIN               //defined in pins_arduino.h
#elif defined(ARDUINO_ESP32C3_DEV)
  #define TITLE "ESP32-C3 Dev Module"  
  static const uint8_t BOOT_BUILTIN = 9;          // defined in pins_arduino.h
  #define BOOT_BUILTIN BOOT_BUILTIN

#else
  #error "Unknown device"
#endif

unsigned long pinstime = 0;

void iopins(void) {
  if ((pinstime) && (millis() - pinstime < 10000)) return;

  Serial.println("\nArduino I/O Pin Names and Numbers");
  Serial.print("Board: ");
  Serial.println(TITLE);

  Serial.printf("\nThe symbolic name and corresponding I/O number of the 6 analogue pins\n");
  Serial.printf(" A0 = %d\n", A0);
  Serial.printf(" A1 = %d\n", A1);
  Serial.printf(" A2 = %d\n", A2);
  Serial.printf(" A3 = %d\n", A3);
  Serial.printf(" A4 = %d\n", A4);
  Serial.printf(" A5 = %d\n", A5);

  Serial.println("\nThe I/O number of the 13 digital pins");
  for (int i=0; i<11; i++) {
    Serial.printf(" %2d\n", i);
  }
  Serial.println(" 20");
  Serial.println(" 21");

  Serial.println("\nThe symbolic name and corresponding I/O number of the 7 pins connected to default serial peripherals");
  Serial.printf("  TX = %2d [UART]\n", TX);
  Serial.printf("  RX = %2d [UART]\n", RX);

  Serial.printf(" SCL = %2d [I2C]\n", SCL);
  Serial.printf(" SDA = %2d [I2C]\n", SDA);

  Serial.printf("  SS = %2d [SPI]\n", SS);
  Serial.printf("MOSI = %2d [SPI]\n", MOSI);
  Serial.printf("MISO = %2d [SPI]\n", MISO);
  Serial.printf(" SCK = %2d [SPI]\n", SCK);

  #if defined(ARDUINO_MAKERGO_C3_SUPERMINI) || defined(ARDUINO_SUPER_MINI_ESP32C3)
    Serial.printf(" TX1 = %2d [UART1]\n", TX1);
    Serial.printf(" RX1 = %2d [UART1]\n", RX1);
  #endif

  Serial.print("\nOther connected devices");
  Serial.printf("\n  BOOT_BUILTIN [BUTTON]= %d", BOOT_BUILTIN);

  #ifdef LED_BUILTIN
    Serial.printf("\n  LED_BUILTIN = %2d", LED_BUILTIN);
  #endif
  #ifdef BUILTIN_LED
    Serial.printf("\n  BUILTIN_LED = %2d", BUILTIN_LED);
  #endif
  #ifdef LED_BUILTIN
    if (LED_BUILTIN != TRUE_LED_PIN) Serial.printf("\n  ** LED_BUILTIN should be = %d **", TRUE_LED_PIN);
  #endif

  Serial.println("\n\nPress the boot button to test");
  pinstime = millis();
}

int bootbtn = 0;

void setup() {
  Serial.begin();
  delay(2000); // 2 second delay should be sufficient for USB-CDC
  Serial.println("\n\nList ESP32C3 based board I/O pin names and numbers");

  #ifdef BOOT_BUILTIN  // in case a board without boot button added
  pinMode(BOOT_BUILTIN, INPUT);
  bootbtn = digitalRead(BOOT_BUILTIN);
  Serial.printf("Boot button default value: %d\n", bootbtn);
  #endif
  Serial.println("Setup completed");
}

void loop() {
  iopins();
  #ifdef BOOT_BUILTIN
  if (bootbtn != digitalRead(BOOT_BUILTIN)) {
    bootbtn = digitalRead(BOOT_BUILTIN);
    Serial.printf("Boot button value: %d\n", bootbtn);
  }
  #endif
}
