/*
 *  See spi.ino for license and attribution.
 */

#include <Arduino.h>
#include <SPI.h>

//////// User configuration //////
///
///  If defined the SPI clock is divided by 8
///#define CHANGE_CLOCK_DIVIDER
///
///  Define the delay between SPI transmissions in milliseconds
#define SPI_DELAY 1000
///
///  Define if there is a 0.42" OLED on the board - not meaningful here
///#define HAS_OLED
///
/// Rate of USB to Serial chip if used on the development board.
/// This is ignored when the native USB peripheral of the 
/// ESP SoC is used.
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

void setup() {
  #if !defined(SERIAL_BEGIN_DELAY)
    #if defined(PLATFORMIO)
      #define SERIAL_BEGIN_DELAY 5000    // 5 seconds
    #else
      #define SERIAL_BEGIN_DELAY 2000    // 2 seconds
    #endif
  #endif 

  #if (ARDUINO_USB_CDC_ON_BOOT > 0)
  Serial.begin();
  delay(SERIAL_BEGIN_DELAY);
  #else 
  Serial.begin(SERIAL_BAUD);
  delay(SERIAL_BEGIN_DELAY);
  Serial.println();
  #endif  

  Serial.println("\nProject: spi");
  Serial.println("Purpose: SPI Loopback Test");
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
  Serial.println("Initializing SPI port");
  Serial.printf("  MOSI (output pin): %d - connected to MISO (input put):  %d\n", MOSI, MISO);
  Serial.printf("  MISO (input pin):  %d - connected to MOSI (output pin): %d\n", MISO, MOSI);
  Serial.printf("  SCK  (clock):      %d - not connected\n", SCK);
  Serial.printf("  SS   (select):     %d - not connected\n", SS);

  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);                // disable chip select
  SPI.begin();
  //bool begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1);
  Serial.printf("Default SPI clock divider: %lu\n", (unsigned long)SPI.getClockDivider());
  #ifdef CHANGE_CLOCK_DIVIDER
  SPI.setClockDivider(SPI_CLOCK_DIV8);   //divide the clock by 8
  Serial.printf("Set SPI clock divider to %lu (SPI_CLOCK_DIV8)\n", (unsigned long)SPI_CLOCK_DIV8);
  #endif
  Serial.println("\nIt may be necessary to reset the board after the firmware is uploaded.");
  Serial.println("Otherwise, it will seem as if the test fails.");
  Serial.println("\nsetup() completed, starting loop back test.");
 }

#define BUFFER_SIZE  32

char outBuffer[BUFFER_SIZE] = {0};
char inBuffer[BUFFER_SIZE] = {0};
int count = 0;

void loop (void) {
  // digitalWrite(SS, LOW); // enable Slave Select - not used here!
  count++;
  int n = snprintf(outBuffer, BUFFER_SIZE, "Message %d", count);
  Serial.printf("\nTransmitting \"%s\"\n", outBuffer);

  for (int i = 0; i < n; i++) {
    inBuffer[i] = SPI.transfer(outBuffer[i]);
  }
  // digitalWrite(SS, HIGH); // disable Slave Select

  Serial.print("Received: \"");
  for (int i=0; i < n; i++) {
    char c = inBuffer[i];
    if ((31 < c) && (c < 127))
      Serial.print(c);
    else
      Serial.printf(" 0x%.2x ", c);
  }
  Serial.println("\"");

  memset(inBuffer, 0xA5, sizeof(inBuffer));
  #ifdef SPI_DELAY
  delay(SPI_DELAY);
  #endif
}
