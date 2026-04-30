/*
 *  See ble_led3.ino for license and attribution.
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
///  Define the io pin to which the momentary push button is connnected, default is the onboard BOOT button
#define BUTTON_PIN BOOT_PIN
///
///  Define value of button pin (HIGH or LOW) when the button is not pressed, default is HIGH
#define BUTTON_OFF HIGH
///
///  If defined, then an attempt will be made to set all BLE TX power levels. The test
///  should show that, according to esp_tx_power_get(), all power levels from
///  ESP_PWR_LVL_N24 (-24 dbm) to ESP_PWR_LVL_P20 (+20 dbm) can be set with either 
///  BLEDevice::setPower() or esp_ble_tx_powerset(). However, according to 
///  BLEDevice::getPower the only valid TX power values are between
///  ESP_PWR_LVL_N12 (-12 dbm) to ESP_PWR_LVL_P9 (+9 dbm).
///  Nothing is done after the test is run
///#define TEST_SETTING_BLE_PWR_LVL
///
///  Define low and high Bluetooth TX power levels to use when advertising. 
///  For possible values, see esp_power_level_t in 
///  https://github.com/espressif/esp-idf/blob/master/components/bt/include/esp32c3/include/esp_bt.h
///  BLEDevice::getPower returns ESP_PWR_LVL_N12 to ESP_PWR_LVL_P9 as valid and ESP_PWR_LVL_P9 is the default
///
///  If LOW_BLE_PWR_LEVEL is not defined, it will be set to ESP_PWR_LVL_N24
///#define LOW_BLE_PWR_LEVEL  ESP_PWR_LVL_N12
///
///  if HIGH_BLE_PWR_LEVEL is not defined, it will be set to  ESP_PWR_LVL_P20
///#define HIGH_BLE_PWR_LEVEL ESP_PWR_LVL_P9
///
/////////////////////////////////////


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
static uint8_t buttonPin = BUTTON_PIN;
#if BUTTON_OFF == HIGH
  #define BUTTON_MODE INPUT_PULLUP
  static uint8_t buttonActive = LOW;
#elif BUTTON_OFF == LOW
  #define BUTTON_MODE INPUT_PULLDOWN
  static uint8_t buttonActive = HIGH;
#else
  #error BUTTON_OFF value invalid (wanted HIGH or LOW)  
#endif

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

#define BLUETOOTH_NAME  "BLE_LED3"

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

const char* ESP_BLE_PWR_TYPES[] = {
  "ESP_BLE_PWR_TYPE_CONN_HDL0",  //0,  TX power for Connection state handle 0
  "ESP_BLE_PWR_TYPE_CONN_HDL1",  //1,  TX power for Connection state handle 1
  "ESP_BLE_PWR_TYPE_CONN_HDL2",  //2,  TX power for Connection state handle 2
  "ESP_BLE_PWR_TYPE_CONN_HDL3",  //3,  TX power for Connection state handle 3
  "ESP_BLE_PWR_TYPE_CONN_HDL4",  //4,  TX power for Connection state handle 4
  "ESP_BLE_PWR_TYPE_CONN_HDL5",  //5,  TX power for Connection state handle 5
  "ESP_BLE_PWR_TYPE_CONN_HDL6",  //6,  TX power for Connection state handle 6
  "ESP_BLE_PWR_TYPE_CONN_HDL7",  //7,  TX power for Connection state handle 7
  "ESP_BLE_PWR_TYPE_CONN_HDL8",  //8,  TX power for Connection state handle 8
  "ESP_BLE_PWR_TYPE_ADV",        //9,  TX power for Advertising state
  "ESP_BLE_PWR_TYPE_SCAN",       //10, TX power for Scanning state
  "ESP_BLE_PWR_TYPE_DEFAULT",    //11, TX power for states that have not been set before
  "ESP_BLE_PWR_TYPE_NUM"         //12  Reserved  
};

String powerTypeStr(int ptyp) {
  if ((ptyp < ESP_BLE_PWR_TYPE_CONN_HDL0) || (ptyp >= ESP_BLE_PWR_TYPE_NUM)) {
    return String("ESP_BLE_PWR_TYPE INVALID");
  } else {
    return String(ESP_BLE_PWR_TYPES[ptyp]);
  }
}

const char* ESP_PWR_LEVELS[] = {
  "ESP_PWR_LVL_N24",   // 0,     -24dbm
  "ESP_PWR_LVL_N21",   // 1,     -21dbm
  "ESP_PWR_LVL_N18",   // 2,     -18dbm
  "ESP_PWR_LVL_N15",   // 3,     -15dbm
  "ESP_PWR_LVL_N12",   // 4,     -12dbm
  "ESP_PWR_LVL_N9 ",   // 5,      -9dbm
  "ESP_PWR_LVL_N6 ",   // 6,      -6dbm
  "ESP_PWR_LVL_N3 ",   // 7,      -3dbm
  "ESP_PWR_LVL_N0 ",   // 8,       0dbm
  "ESP_PWR_LVL_P3 ",   // 9,      +3dbm
  "ESP_PWR_LVL_P6 ",   // 10,     +6dbm
  "ESP_PWR_LVL_P9 ",   // 11,     +9dbm
  "ESP_PWR_LVL_P12",   // 12,     +12dbm
  "ESP_PWR_LVL_P15",   // 13,     +15dbm
  "ESP_PWR_LVL_P18",   // 14,     +18dbm
  "ESP_PWR_LVL_P20"    // 15,     +20dbm    
     //ESP_PWR_LVL_P21 = 15,             /*!< Deprecated */
     //ESP_PWR_LVL_INVALID = 0xFF,       /*!< Indicates an invalid value */
};

String powerLevelStr(int level) {
  if ((level < ESP_PWR_LVL_N24) || (level > ESP_PWR_LVL_P20)) {
    return String("ESP_PWR_LVL_INVALID");
  } else {
    return String(ESP_PWR_LEVELS[level]);
  }
}

void read_ble_tx_power(void) {  
  esp_power_level_t esp_ble_tx_power;
  int txpower;

  Serial.println("\nReading txpower, ESP_BLE_PWR_TYPE_DEFAULT");
  esp_ble_tx_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
  Serial.printf("  using esp_tx_power_get(): %s (index=%d)\n", powerLevelStr(esp_ble_tx_power).c_str(), esp_ble_tx_power);
  txpower = BLEDevice::getPower(ESP_BLE_PWR_TYPE_DEFAULT); // same as BLEDevice::getPower()
  Serial.printf("  using BLEDevice::getPower(): %d dBm\n", txpower); // BLE power type: ESP_BLE_PWR_TYPE_DEFAULT

  Serial.println("\nReading txpower, ESP_BLE_PWR_TYPE_ADV");
  esp_ble_tx_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_ADV);
  Serial.printf("  using esp_tx_power_get(): %s (index=%d)\n", powerLevelStr(esp_ble_tx_power).c_str(), esp_ble_tx_power);
  txpower = BLEDevice::getPower(ESP_BLE_PWR_TYPE_ADV);
  Serial.printf("  using BLEDevice::getPower(): %d dBm\n", txpower); // BLE power type: ESP_BLE_PWR_TYPE_DEFAULT

  Serial.println("\nReading txpower, ESP_BLE_PWR_TYPE_SCAN");
  esp_ble_tx_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_SCAN);
  Serial.printf("  using esp_tx_power_get(): %s (index=%d)\n", powerLevelStr(esp_ble_tx_power).c_str(), esp_ble_tx_power);
  txpower = BLEDevice::getPower(ESP_BLE_PWR_TYPE_SCAN);
  Serial.printf("  using BLEDevice::getPower(): %d dBm\n", txpower); // BLE power type: ESP_BLE_PWR_TYPE_DEFAULT
}

void set_BLEDevicePower(esp_power_level_t plevel) {
  Serial.printf("\n\nSetting ble tx power with BLEDevice::setPower(%s, ESP_BLE_PWR_TYPE_DEFAULT)\n", powerLevelStr(plevel).c_str());
  BLEDevice::setPower(plevel, ESP_BLE_PWR_TYPE_DEFAULT);
  delay(100);
  read_ble_tx_power();
}  

void set_ble_tx_power(esp_ble_power_type_t ptype, esp_power_level_t plevel) {
  Serial.printf("\n\nSetting ble tx power with esp_ble_tx_powerset(%s, %s)\n", powerTypeStr(ptype).c_str(),  powerLevelStr(plevel).c_str());
  esp_err_t espErr = esp_ble_tx_power_set(ptype, plevel);  // this will fail if TYPE_DEFAULT, but the function returns ESP_OK!!
  if (espErr == ESP_ERR_NOT_SUPPORTED)
    Serial.println("esp_ble_tx_power_set returned 'Invalid TX power type'");   
  else if (espErr == ESP_FAIL)
    Serial.println("esp_ble_tx_power_set returned 'Failure due to other reason'");
  else if (espErr == ESP_OK)
    Serial.println("esp_ble_tx_power_set returned 'Success'");
  else
    Serial.printf("esp_ble_tx_power_set returned unknown error %d\n", (int) espErr);
  delay(100);
  read_ble_tx_power();   
}

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

int lowLevelIndex = 0;
int highLevelIndex = 15;
int currentIndex = 0;
bool use_esp_set_ble_tx_power = true;
unsigned long timer = 0;

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

  Serial.println("\n\nProject: ble_led3");
  Serial.println("Purpose: Toggle an external LED on and off with Bluetooth LE");
  Serial.println("         with various BLE TX power levels if desired.");
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

  #ifdef LOW_BLE_PWR_LEVEL
    lowLevelIndex = LOW_BLE_PWR_LEVEL;
  #endif
  
  #ifdef HIGH_BLE_PWR_LEVEL
    highLevelIndex = HIGH_BLE_PWR_LEVEL;
  #endif
  
  if (highLevelIndex < lowLevelIndex) {
    int temp = highLevelIndex;
    highLevelIndex = lowLevelIndex;
    lowLevelIndex = temp;
  }    
  currentIndex = lowLevelIndex;

  // begin initialization

  Serial.println("\nInitializing LED");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 1-ledOn);

  Serial.println("Initializing BUTTON");
  pinMode(buttonPin, BUTTON_MODE);



  // BLEDevice must be initialized before setting/reading BLE TX power level
  
  if (!BLEDevice::init(BLUETOOTH_NAME)) {
    Serial.println("Could not start Bluetooth® Low Energy device!");
    while (1);
  }
  Serial.println("Bluetooth® Low Energy (BLE) device started.");


  #ifdef TEST_SETTING_BLE_PWR_LVL
 
  #ifdef RESTORE_DEFAULT
  //Serial.println("\nReading txpower, ESP_BLE_PWR_TYPE_DEFAULT");
  esp_power_level_t default_ble_tx_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
  #endif

  for (int i=0; i <= 15; i++) {
    set_BLEDevicePower( (esp_power_level_t)i);
  }

  Serial.println();

  for (int i=0; i <= 15; i++) {
    set_ble_tx_power(ESP_BLE_PWR_TYPE_DEFAULT, (esp_power_level_t)i);
  }

  #ifdef RESTORE_DEFAULT
  Serial.println("\nTest done.\nResetting default");
  set_ble_tx_power(ESP_BLE_PWR_TYPE_DEFAULT, default_ble_tx_power);
  Serial.println();
  #endif

  Serial.println("\nTest done.");
  while (true) delay(10);    // do nothing further

  #endif


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

  Serial.print("\nSetup completed, connect to ");
  Serial.println(BLUETOOTH_NAME);
  Serial.print("Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());

  Serial.printf("\n\"%s\" device now being advertised\n", BLUETOOTH_NAME);
 
  Serial.println("\nTurn the attached LED on/off with a smartphone applications such as");
  Serial.println("\n  nRF Connect for Mobile by Nordic Semiconductor ASA");
  Serial.println("    Android: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp");
  Serial.println("    IOS: https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403");
  Serial.println("\n  LightBlue - Bluetooth LE by Punch Through Design");
  Serial.println("    Android: https://play.google.com/store/apps/details?id=com.punchthrough.lightblueexplorer");
  Serial.println("    IOS: https://apps.apple.com/us/app/lightblue/id557428110");
  Serial.println("Turn the LED on or off by setting the characteristic to 'on' or 'off' in the application.");

  if (lowLevelIndex < highLevelIndex) {
    Serial.printf("\nWill advertise with BLE TX power levels from %s [index: %d] to %s [index: %d]\n", 
      powerLevelStr(lowLevelIndex).c_str(), lowLevelIndex,
      powerLevelStr(highLevelIndex).c_str(), highLevelIndex);
    Serial.println("Change the TX power by pressing and releasing the (boot) button.");
    Serial.println("Setting the BLE TX power will switch between using esp_ble_tx_power_set()");
    Serial.printf("and BLEDevice::setPower() when the TX power level cycles back to level %s.\n", powerLevelStr(lowLevelIndex).c_str());
  } else {
    Serial.printf("\nOnly BLE TX power level %s [index: %d] will be usedd\n",
      powerLevelStr(lowLevelIndex).c_str(), lowLevelIndex);
  }

  // set the initial or only BLE TX power level 
  if (use_esp_set_ble_tx_power)  
    set_ble_tx_power(ESP_BLE_PWR_TYPE_DEFAULT, (esp_power_level_t)currentIndex);  
  else
    set_BLEDevicePower( (esp_power_level_t)currentIndex);    

  Serial.println("\nSetup completed.");
}


void loop() {
  if ((lowLevelIndex < highLevelIndex) && (digitalRead(buttonPin) == buttonActive)) {
    while (digitalRead(buttonPin) == buttonActive)
      delay(100);
    delay(200);
    Serial.println("\nBoot button released");
    currentIndex++;
    if (currentIndex > highLevelIndex) {
      currentIndex = lowLevelIndex;
      use_esp_set_ble_tx_power = !use_esp_set_ble_tx_power;
    }
    if (use_esp_set_ble_tx_power)  
      set_ble_tx_power(ESP_BLE_PWR_TYPE_DEFAULT, (esp_power_level_t)currentIndex);  
    else
      set_BLEDevicePower( (esp_power_level_t)currentIndex);    
  }

  if (millis() - timer > 10000) {
    Serial.print(" - loop: ");
    if (lowLevelIndex < highLevelIndex) 
      Serial.println("press button to set new BLE TX power level");
    else  
      Serial.println("busy work");
    timer = millis();
  }
}
