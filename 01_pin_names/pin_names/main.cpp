/*
 *  See pin_names.ino for license and attribution.
 */

#include <Arduino.h>

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

#if !defined(ESP32)
  #error An ESP32 based board is required
#endif  

#if !ARDUINO_USB_CDC_ON_BOOT || ARDUINO_USB_CDC_ON_BOOT != 1
  #error Expected an ESP32 board with on board USB peripheral
#endif

void iopins(void) {
  Serial.println("\nArduino I/O Pin Names and Numbers");

  Serial.printf("\nThe symbolic name and corresponding I/O number of the 6 analogue pins\n");
  Serial.printf(" A0 = %d\n", A0);
  Serial.printf(" A1 = %d [JTAG MTMS]\n", A1);
  Serial.printf(" A2 = %d [JTAG MTDI]\n", A2);
  Serial.printf(" A3 = %d [JTAG MTCK]\n", A3);
  Serial.printf(" A4 = %d [JTAG MTDO]\n", A4);
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

  Serial.println("\nOther connected devices");

  Serial.printf("  LED_BUILTIN = %2d\n", LED_BUILTIN);  // static const uint8_t LED_BUILTIN = 8 in pins_arduino.h
  Serial.printf("  BOOT_PIN    = %2d\n", BOOT_PIN);     // static const uint8_t BOOT_PIN = 9 in esp32-hal.h
 
  Serial.println("\nDevice macros");

  #ifdef LED_BUILTIN
  Serial.printf("  LED_BUILTIN = %2d\n", LED_BUILTIN);
  #else
    #error LED_BUILTIN macro should be defined
  #endif
  
  #ifdef BUILTIN_LED
  Serial.printf("  BUILTIN_LED = %2d\n", BUILTIN_LED);
  #else
    #error BUILTIN_LED macro should be defined
  #endif

  #ifdef BOOT_PIN
  Serial.printf("  BOOT_PIN    = %2d\n", BOOT_PIN);
  #else
    #error BOOT_PIN macro should be defined
  #endif


  Serial.println("\nBuild macros");
 
  #ifdef PLATFORMIO
  Serial.printf("  PLATFORMIO = %d\n", PLATFORMIO);
  #else
  Serial.println("  PLATFORMIO not defined"); // must be in Arduino IDE
  #endif    
 
  #ifdef ARDUINO 
  Serial.printf("  ARDUINO = %d\n", ARDUINO);
  #else
    #error ARDUINO should be defined
  #endif
 
  #ifdef ESP_PLATFORM
  Serial.println("  ESP_PLATFORM");
  #else
    #error ESP32_PLATFORM should be defined
  #endif
 
  #ifdef ESP32  // already verified
  Serial.println("  ESP32");
  #else
    #error ESP32 should be defined
  #endif
 
  #ifdef IDF_VER
  Serial.printf("  IDF_VER = \"%s\"\n", IDF_VER);
  #else
    #error IDF_VER should be defined
  #endif  
 
  #ifdef ARDUINO_ARCH_ESP32
  Serial.println("  ARDUINO_ARCH_ESP32");
  #else
    #error ARDUINO_ARCH_ESP32 should be defined
  #endif
  
  #ifdef ARDUINO_BOARD
  Serial.printf("  ARDUINO_BOARD = \"%s\"\n", ARDUINO_BOARD);
  #else
    #error  ARDUINO_BOARD should be defined
  #endif

  #ifdef ARDUINO_VARIANT
  Serial.printf("  ARDUINO_VARIANT = \"%s\"\n", ARDUINO_VARIANT);
  #else
    #error  ARDUINO_VARIANT should be defined
  #endif

  #ifdef ARDUINO_ESP32C3_DEV
  Serial.println("  ARDUINO_ESP32C3_DEV");
  #else
  Serial.println("  ARDUINO_ESP32C3_DEV is not defined");
  #endif


  #ifdef ARDUINO_USB_CDC_ON_BOOT 
  Serial.printf("  ARDUINO_USB_CDC_ON_BOOT = %d\n", ARDUINO_USB_CDC_ON_BOOT);
  #else
    #error ARDUINO_USB_CDC_ON_BOOT=1 should be defined
  #endif                                                       

  #ifdef ARDUINO_USB_MODE 
  Serial.printf("  ARDUINO_USB_MODE = %d\n", ARDUINO_USB_MODE);
  #else
    #error ARDUINO_USB_MODE=1 should be defined
  #endif     

  Serial.print("\n\nPress the boot button to test ");
}

int bootbtn = 0;
unsigned long pinstime = 0;

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

  Serial.println("\n\nProject: pin_names");
  Serial.println("Purpose: Display content of pins_arduino.h and various macro definitions");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else
  Serial.println("Unknown ESP32 board");
  #endif
  #if defined(ARDUINO_ESP32C3_DEV)
  if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
    Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif
  pinMode(BOOT_PIN, INPUT_PULLUP);
  delay(5);
  bootbtn = digitalRead(BOOT_PIN);  
  iopins();
}


void loop() {
  if (bootbtn != digitalRead(BOOT_PIN)) {
    while (bootbtn != digitalRead(BOOT_PIN)) 
      delay(100);
    delay(200);
    Serial.println("\nBoot button released");    
    iopins();
  } else {
    delay(50);
  }
}
