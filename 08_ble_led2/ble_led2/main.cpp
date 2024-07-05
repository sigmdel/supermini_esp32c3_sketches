
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// This is a simplified example which can use custom UUIDs in which case
// the client will probably show the UUID for the service and characteristic
// or it can use some more or less valid reserved UUID from the Bluetooth(R) 
// Assigned Numbers document https://www.bluetooth.com/specifications/assigned-numbers/ 
//
#define USE_CUSTOM_UUIDS

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

#if defined(BUILTIN_LED)
  const uint8_t ledPin = BUILTIN_LED;
  const uint8_t ledOn = LOW;
#else
  #error "ledPin not defined"
#endif

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
  "ESP_PWR_LVL_P21"    // 15,     +21dbm
};

String PWR_LEVEL_STR(esp_power_level_t level) {
  if ((level < ESP_PWR_LVL_N24) || (level > ESP_PWR_LVL_P21)) {
    return String("ESP_PWR_LVL_INVALID");
  } else {
    return String(ESP_PWR_LEVELS[level]);  
  }
}  

// leave undefined to use default BLE power level
#define BLE_PWR_LEVEL  ESP_PWR_LVL_N21

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
      digitalWrite(ledPin, ledOn); 
      Serial.println("Received \"on\" value");
    } else if(value == "off"){
      digitalWrite(ledPin, 1-ledOn);
      Serial.println("Received \"off\" value");
    } else {
      Serial.printf("Received non valid \"%s\" value \n", value.c_str());
    }  
  }
};


void setup() {
  #if defined(ARDUINO_MAKERGO_C3_SUPERMINI)
  Serial.begin();
  delay(1000); // should be enough for the USB CDC to initialize
  #else
  Serial.begin(115200);
  while (!Serial) delay(10);
  #endif

  Serial.println("Setup");

  Serial.println("Initializing LED");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 1-ledOn);

  Serial.println("Initializing BLEDevice");
  BLEDevice::init(BLUETOOTH_NAME);

  // defined in <esp_bt.h> 
  esp_power_level_t esp_ble_tx_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
  Serial.print("Default BLE power level ");
  Serial.println(PWR_LEVEL_STR(esp_ble_tx_power));

  #if defined(BLE_PWR_LEVEL) 
    // https://github.com/nkolban/esp32-snippets/issues/197#issuecomment-344115566
    // http://esp-idf.readthedocs.io/en/latest/api-reference/bluetooth/controller_vhci.html?highlight=esp_ble_tx_power_set#_CPPv220esp_ble_tx_power_set20esp_ble_power_type_t17esp_power_level_t

    BLEDevice::setPower(BLE_PWR_LEVEL);
    delay(10);
    esp_ble_tx_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
    Serial.print("BLE power level set to ");
    Serial.print(PWR_LEVEL_STR(esp_ble_tx_power));
    Serial.print(" wanted ");
    Serial.println(PWR_LEVEL_STR(BLE_PWR_LEVEL));
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
}

unsigned long timer = 0;

void loop() {
  if (millis() - timer > 5000) {
    Serial.println(" - loop busy work");
    timer = millis();
  }
}  
