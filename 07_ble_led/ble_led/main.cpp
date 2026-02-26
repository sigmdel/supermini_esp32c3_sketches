/*
 *  See ble_led.ino for license and attribution.
 */

/*
XXXXX Major errors --- 2026-02-26 XXXXXXXXXXXXXXXXX


Will not even compile with ESP32C6 - linking error

Linking .pio/build/seeed_xiao_esp32c6/firmware.elf
/home/michel/.platformio/packages/toolchain-riscv32-esp/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld: .pio/build/seeed_xiao_esp32c6/lib6ff/ArduinoBLE/utility/HCIVirtualTransport.cpp.o: in function `bleTask(void*)':
/home/michel/Documents/PlatformIO/GitHubs/supermini_esp32c3_sketches/07_ble_led/.pio/libdeps/seeed_xiao_esp32c6/ArduinoBLE/src/utility/HCIVirtualTransport.cpp:46:(.text._Z7bleTaskPv+0x28): undefined reference to `esp_vhci_host_register_callback'
/home/michel/.platformio/packages/toolchain-riscv32-esp/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld: /home/michel/Documents/PlatformIO/GitHubs/supermini_esp32c3_sketches/07_ble_led/.pio/libdeps/seeed_xiao_esp32c6/ArduinoBLE/src/utility/HCIVirtualTransport.cpp:52:(.text._Z7bleTaskPv+0x46): undefined reference to `esp_vhci_host_check_send_available'
/home/michel/.platformio/packages/toolchain-riscv32-esp/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld: /home/michel/Documents/PlatformIO/GitHubs/supermini_esp32c3_sketches/07_ble_led/.pio/libdeps/seeed_xiao_esp32c6/ArduinoBLE/src/utility/HCIVirtualTransport.cpp:53:(.text._Z7bleTaskPv+0x58): undefined reference to `esp_vhci_host_send_packet'
collect2: error: ld returned 1 exit status
*** [.pio/build/seeed_xiao_esp32c6/firmware.elf] Error 1

Will compile with ESP32C3 but getting core panic when BLE.begin() is executed:

Project: ble_led.ino
Purpose: Toggle an external LED on and off with Bluetooth
  Board: Seeed Studio XIAO ESP32C3

Initializing LED
btdm: bss start 0x3fcdf0a8, len 40
btdm: data start 0x3fcdf09c, data start rom 0x400591fc, len 12
MAGIC fadebead VERSION 0001000a
MAGIC fadebead VERSION 00010001
Guru Meditation Error: Core  0 panic'ed (Instruction access fault). Exception was unhandled.

Core  0 register dump:
MEPC    : 0x00000000  RA      : 0x42026c36  SP      : 0x3fc9fa80  GP      : 0x3fc93c00  
TP      : 0x3fc9fc20  T0      : 0x4005890e  T1      : 0x0000000f  T2      : 0x00000001  
S0/FP   : 0x3fce0000  S1      : 0x3fce0000  A0      : 0x00000001  A1      : 0x0000011c  
A2      : 0x3fca0444  A3      : 0x00000003  A4      : 0x00000000  A5      : 0x00000000  
A6      : 0x80000000  A7      : 0x00000010  S2      : 0x3fce0000  S3      : 0x3fce0000  
S4      : 0x3fce0000  S5      : 0x3fcdf96c  S6      : 0x00000000  S7      : 0x00000000  
S8      : 0x00000000  S9      : 0x00000000  S10     : 0x00000000  S11     : 0x00000000  
T3      : 0x00000000  T4      : 0x004b0005  T5      : 0x01010000  T6      : 0x00000064  
MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000001  MTVAL   : 0x00000000  
MHARTID : 0x00000000  

Stack memory:
3fc9fa80: 0x00000000 0x004b0005 0x3fcdf96c 0x42022a7a 0x3fc9b890 0x3fc9fb38 0x00000000 0x0001000a
3fc9faa0: 0x3fca0202 0x0000000a 0x00000002 0x00000042 0x80000000 0x00000010 0x00000001 0x00000006
3fc9fac0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9fae0: 0x00000001 0x3fc9fb38 0x00000001 0x4201f530 0x00000000 0x3c058a0c 0x00000000 0x4201c962
...

Three things have changed since this project was last tested as working:

ESP-IDF            updated from v5.5.2.260116  to V5.5.2.260206
esp32 Arduino core updated from 3.3.6          to 3.3.7
ArduinoBLE         updated from 1.5.0          to 2.0.0 
*/

//#error Not currently working, see above comment

#include <Arduino.h>
#include <ArduinoBLE.h>

// #include "MACs.h"

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board - not meaningful in this sketch
///#define HAS_OLED
///
///  Define the io pin to which a LED is connected, default is the onboard user LED 
///  XXXXXXXXXXXX undef to test with XIAO EPS32C3
///#define LED_PIN LED_BUILTIN
///
///  What signal (HIGH or LOW) turns the LED on
///  XXXXXXXXXXXXXXX undef to test with XIAO EPS32C3
///#define LED_ON LOW
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

//XXXXXXXXXXXX temp fix to test with other boards
#if defined(ARDUINO_XIAO_ESP32C3)
#define LED_PIN D10
#define LED_ON HIGH
#else
#define LED_PIN LED_BUILTIN
#define LED_ON LOW
#endif

static uint8_t ledPin = LED_PIN;
static uint8_t ledOn = LED_ON;

void setLed(int value) {
  digitalWrite(ledPin, (value) ? ledOn : 1-ledOn);
  Serial.printf("LED now %s.\n", (digitalRead(ledPin) == ledOn) ? "on" : "off");
}

// Bluetooth® Low Energy LED Service
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, readable and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

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

  Serial.println("\n\nProject: ble_led.ino");
  Serial.println("Purpose: Toggle an external LED on and off with Bluetooth");
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
//  Serial.printf("BT MAC: %s\n", BT_MAC_STR);
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif

  // begin initialization

  Serial.println("\nInitializing LED");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 1-ledOn);

  if (!BLE.begin()) {
    Serial.println("Could not start Bluetooth® Low Energy module!");
    while (1);
  }
  Serial.println("Bluetooth® Low Energy (BLE) module started.");

  // set advertised local name and service UUID:
  BLE.setLocalName("HOME Automation");            // this will appear in the App search result.
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);
  // set the initial value for the characeristic, i.e. LED off
  switchCharacteristic.writeValue(0);
  Serial.println("\nLED service added.");

  BLE.advertise();
  Serial.println("\"HOME Automation\" device now being advertised");
  Serial.println("Setup completed.");

  Serial.println("\nTurn the attached LED on/off with a smartphone applications such as");
  Serial.println("\n  nRF Connect for Mobile by Nordic Semiconductor ASA");
  Serial.println("    Android: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp");
  Serial.println("    IOS: https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403");
  Serial.println("\n  LightBlue - Bluetooth LE by Punch Through Design");
  Serial.println("    Android: https://play.google.com/store/apps/details?id=com.punchthrough.lightblueexplorer");
  Serial.println("    IOS: https://apps.apple.com/us/app/lightblue/id557428110");
}


void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    String dName = central.deviceName();
    if (!dName.length())
      dName = central.localName();
    if (!dName.length())
      dName = "device";                 // default if no name is found for the newly connected device
    dName += ": " + central.address();  // add the latter's MAC address
    Serial.print("Connected to ");
    Serial.println(dName);

    while (central.connected()) {
      if (switchCharacteristic.written()) {
        int Rvalue=switchCharacteristic.value();
        Serial.printf("Received switchCharacteristic = %02x.\n", Rvalue);

        if ((Rvalue == 0) || (Rvalue == 1)) {
          setLed(Rvalue);
        } else {
          Serial.println("Ignored, expected 00 or 01.\n");
        }

      }
    }

    Serial.print("Disconnected from ");
    Serial.println(dName);
  }
}
