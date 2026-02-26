/*
 *  See ble_led2.ino for license and attribution.
 */

 #include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "MACs.h"

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board - not meaningful in this sketch
///#define HAS_OLED
///
///  Define the io pin to which a LED is connected, default is the onboard user LED 
#define LED_PIN LED_BUILTIN
///
///  What signal (HIGH or LOW) turns the LED on
#define LED_ON LOW
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

static uint8_t ledPin = LED_PIN;
static uint8_t ledOn = LED_ON;

void setLed(int value) {
  digitalWrite(ledPin, (value) ? ledOn : 1-ledOn);
  Serial.printf("LED now %s.\n", (digitalRead(ledPin) == ledOn) ? "on" : "off");
}

// This is a simplified example which can use custom UUIDs in which case
// the client will probably show the UUID for the service and characteristic
// or it can use some more or less valid reserved UUID from the Bluetooth(R)
// Assigned Numbers document https://www.bluetooth.com/specifications/assigned-numbers/
//
//#define USE_CUSTOM_UUIDS

#define BLUETOOTH_NAME  "BLE_LED2"

#ifdef USE_CUSTOM_UUIDS
  // Custom UUID for service and characteristic must not conflict with a reserved UUID
  // that is no number in the XXXXXXXX-0000-1000-8000-00805F9B34FB range
  // Generated at https://www.guidgenerator.com/
  // https://novelbits.io/uuid-for-custom-services-and-characteristics/
  #define SERVICE_UUID        "57a81fc3-3c5f-4d29-80e7-8b074e34888c"
  #define CHARACTERISTIC_UUID "2eeae074-8955-47f7-9470-73f85112974f"
#else
  #define SERVICE_UUID        "1815" //"00001815-0000-1000-8000-00805F9B34FB"  // Automation IO Service
                            //"1812" //"00001812-0000-1000-8000-00805F9B34FB"  // Human Interface Device Service
                            //"181c" //"0000181c-0000-1000-8000-00805F9B34FB"  // User Data Service
  #define CHARACTERISTIC_UUID "2BE2" //"00002BE2-0000-1000-8000-00805F9B34FB"  // Light Output
                            //"2BO5" //"00002B05-0000-1000-8000-00805F9B34FB"  // Power
#endif

// leave undefined to use default BLE power level
//#define BLE_PWR_LEVEL  ESP_PWR_LVL_N21

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      Serial.println("Restart advertising");
      BLEDevice::startAdvertising();
    }
};

class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value == "on") {
      setLed(1);
    } else if(value == "off"){
      setLed(0);
    } else {
      Serial.printf("Received non valid \"%s\" value \n", value.c_str());
    }
  }
};


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

  Serial.println("\n\nProject: ble_led2");
  Serial.println("Purpose: Toggle an external LED on and off with Bluetooth LE");
  Serial.println("         setting the BLE power level if desired.");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.print(ARDUINO_BOARD);
  #else 
  Serial.print("Unknown ESP32 board");
  #endif
  #ifdef HAS_OLED
    Serial.print(" with 0.42\" OLED");
  #endif  
  Serial.printf("\n BT MAC: %s\n", BT_MAC_STR);
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif

  // begin initialization

  Serial.println("\nInitializing LED");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 1-ledOn);

  if (!BLEDevice::init(BLUETOOTH_NAME)) {
    Serial.println("Could not start Bluetooth® Low Energy device!");
    while (1);
  }
  Serial.println("Bluetooth® Low Energy (BLE) device started.");

  Serial.println("Creating a BLE server");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  Serial.println("Adding a BLE service");
  BLEService *pService = pServer->createService(SERVICE_UUID);

  Serial.println("Adding a BLE characteristic");
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue(String("off"));
  pCharacteristic->setCallbacks(new WriteCallbacks);

  Serial.println("Starting BLE service");
  pService->start();

  Serial.println("Add advertiser");
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->setScanResponse(true);

  Serial.println("Start BLE advertising");
  BLEDevice::startAdvertising();

  Serial.printf("Device \"%s\" now being advertised", BLUETOOTH_NAME);
  Serial.println("Connect to the device with a smartphone applications such as");
  Serial.println("\n  nRF Connect for Mobile by Nordic Semiconductor ASA");
  Serial.println("    Android: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp");
  Serial.println("    IOS: https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403");
  Serial.println("\n  LightBlue - Bluetooth LE by Punch Through Design");
  Serial.println("    Android: https://play.google.com/store/apps/details?id=com.punchthrough.lightblueexplorer");
  Serial.println("    IOS: https://apps.apple.com/us/app/lightblue/id557428110");
  Serial.println("Turn the LED on or off by setting the characteristic to 'on' or 'off' in the application.");
}

unsigned long timer = 0;

void loop() {
  if (millis() - timer > 10000) {
    Serial.println(" - loop busy work");
    timer = millis();
  }
}
