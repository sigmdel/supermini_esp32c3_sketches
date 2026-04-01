/*
 *  See spi.ino for license and attribution.
 */

#include <Arduino.h>
#include <SPI.h>

//////// User configuration //////
///
/// If defined the SPI clock is divided by 8.
///#define CHANGE_CLOCK_DIVIDER
///
///  Define if there is a 0.42" OLED on the board.
///#define HAS_OLED
///
///  The default pin attached to the SPI MOSI ouput signal
///  is 6 on the ESP32-C3 Super Mini. Choose a different 
///  pin. This will not change during the test as every other
///  io pin along the edge of the board will be attached to
///  the SPI MISO input signal for a loop back test.
#define ALT_MOSI 5
///
///  Time in milliseconds between messages send out using the
///  SPI peripheral. If not defined, a message is sent on each
///  execution of the loop() function.
#define MESSAGE_DELAY 1000 

///  Time in milliseconds during which the current io pin is active.
///  The number of messages sent in each test is therefore 
///  ACTIVE_PERIOD / MESSAGE_DELAY if the latter is defined.
#define ACTIVE_PERIOD 6000
///
///  Time of inactivity in milliseconds between pins. The delay
///  is applied twice, first when the test for one gpio pin is
///  terminated and then again when the prompt for which new pin 
///  to connect is displayed.
#define INACTIVE_PERIOD 3000
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
                                   "0", "1", "2", "3", "4"};                  // left edgemisoIndex
const int iopins[padcount] = {5, 6, 7, 8, 9, 10, 20, 21,                      // right edge
                              0, 1, 2, 3, 4};                                 // left edge
#endif

int misoIndex = -1; // initially no gpio pin is active
unsigned long activetimer = 0;
int count = 0;

bool nextPad(void) {
  if (misoIndex >= 0) {
    // stop SPI which detaches all io pins
    SPI.end();
  }
  delay(INACTIVE_PERIOD); // stop messages

  if (misoIndex >= padcount)
    misoIndex = 0;
  else  
    misoIndex++;
  /*
  Initializing SPI port
    SPI MOSI attached to pin 5 as before
    SPI MIS0 attached to pin 6
    SPI SCK  attached to pin 5
   Successfully initialed the SPI port.  WOW!
  */   
  if (iopins[misoIndex] == ALT_MOSI)
    misoIndex = (misoIndex + 1) % padcount;   // miso cannot equal mosi
  
  int sckIndex = (misoIndex + 1) % padcount;  // use next pin as SCK 
  //Serial.printf("1. misoIndex = %d, sckIndex = %d", misoIndex, sckIndex);
  if (iopins[sckIndex] == ALT_MOSI)
    sckIndex = (sckIndex + 1) % padcount;
  //Serial.printf("1. misoIndex = %d, sckIndex = %d", misoIndex, sckIndex);
  int sck = iopins[sckIndex];       // changing clock assignement to test it can be changed
  int miso = iopins[misoIndex];

  count = 0;               // reset message counter

  Serial.println("\n\nInitializing SPI port...");
  //bool begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1);
  Serial.printf("    MOSI attached to pin %d as before\n", ALT_MOSI);
  Serial.printf("    MIS0 attached to pin %d\n", miso);
  Serial.printf("    SCK  attached to pin %d\n", sck);

  if (SPI.begin(sck, miso, ALT_MOSI, -1)) {
    #ifdef CHANGE_CLOCK_DIVIDER
    SPI.setClockDivider(SPI_CLOCK_DIV8);   //divide the clock by 8
    Serial.printf("  Setting SPI clock divider to %lu (SPI_CLOCK_DIV8)\n", (unsigned long)SPI_CLOCK_DIV8);
    #endif
    delay(100);
    Serial.println("Successfully initialed the SPI port.");
    Serial.printf("Connect gpio %d (MISO) to pin %d (MOSI) now.\n", iopins[misoIndex], ALT_MOSI);
    delay(INACTIVE_PERIOD);  // more time to switch pin
    activetimer = millis();  // reset active (spi messaging) timer
    return true;
  } else {
    Serial.println("Unable to initialed the SPI port.");
    activetimer = millis() + ACTIVE_PERIOD + 5;
    return false;
  }
}

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
  Serial.printf("Default SPI clock divider: %lu.\n\n", (unsigned long)SPI.getClockDivider());
  Serial.println("In this test, the master out / slave in (MOSI) signal of the single SPI");
  Serial.printf("controller is attached to gpio pin %d, as the master in / slave out (MISO)\n", MISO);
  Serial.println("signal is attached to every other gpio pin available on the Super Mini");
  Serial.println("headers. The clock (SCK) signal of the SPI controller is attached to a");
  Serial.println("third gpio pin but it is not used during the loop back tests.");

  Serial.println("\nIt may be necessary to reset the board after the firmware is uploaded.");
  Serial.println("Otherwise, it will seem as if the test fails.");

  Serial.println("\nThe tests will begin, follow the instructions on which gpio pin needs");
  Serial.println("to be connected to the gpio pin %d attached to the SPI MOSI signal.");
  nextPad();
 }

#define BUFFER_SIZE  32

char outBuffer[BUFFER_SIZE] = {0};
char inBuffer[BUFFER_SIZE] = {0};

void loop (void) {
  if (millis() - activetimer > ACTIVE_PERIOD) {
    if (!nextPad()) return;
  }               

  //digitalWrite(SS, LOW); // enable Slave Select
  count++;
  int n = snprintf(outBuffer, BUFFER_SIZE, "Message %d", count);
  Serial.printf("\nTransmitting \"%s\"\n", outBuffer);

  for (int i = 0; i < n; i++) {
    inBuffer[i] = SPI.transfer(outBuffer[i]);
    //Serial.print(inBuffer[i]);
  }
  //digitalWrite(SS, HIGH); // disable Slave Select

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
  #ifdef MESSAGE_DELAY
  delay(MESSAGE_DELAY);
  #endif
}
