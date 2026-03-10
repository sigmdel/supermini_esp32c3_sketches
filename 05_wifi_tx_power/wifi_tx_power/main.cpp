/*
 *  See wifi_tx_power.ino for license and attribution.
 */

#include <Arduino.h>
#include <WiFi.h>

#include "MACs.h"
#include "secrets.h"

//////// User configuration //////
///
///  Define the time allowed for connecting to the Wi-Fi network before giving up
#define TIMEOUT 120000   // 2 minutes
///
///  Define this when using XIAO ESP32C6 with a connected external antenna 
///#define USE_EXTERNAL_ANTENNA 
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
///#define SERIAL_BEGIN_DELAY 10000
///
//////////////////////////////////

#if !defined(ESP32)
  #error An ESP32 based board is required
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

#ifndef TIMEOUT
#define TIMEOUT 120000
#endif

/*
 from ~/.arduino15/packages/esp32/hardware/esp32/3.0.1/libraries/WiFi/src/WiFiGeneric.h
 from ~/.platformio/packages/framework-arduinoespressif32/libraries/WiFi/src/WiFiGeneric.h

typedef enum {
  WIFI_POWER_21dBm = 84,      // 21dBm
  WIFI_POWER_20_5dBm = 82,    // 20.5dBm
  WIFI_POWER_20dBm = 80,      // 20dBm
  WIFI_POWER_19_5dBm = 78,    // 19.5dBm
  WIFI_POWER_19dBm = 76,      // 19dBm
  WIFI_POWER_18_5dBm = 74,    // 18.5dBm
  WIFI_POWER_17dBm = 68,      // 17dBm
  WIFI_POWER_15dBm = 60,      // 15dBm
  WIFI_POWER_13dBm = 52,      // 13dBm
  WIFI_POWER_11dBm = 44,      // 11dBm
  WIFI_POWER_8_5dBm = 34,     // 8.5dBm
  WIFI_POWER_7dBm = 28,       // 7dBm
  WIFI_POWER_5dBm = 20,       // 5dBm
  WIFI_POWER_2dBm = 8,        // 2dBm
  WIFI_POWER_MINUS_1dBm = -4  // -1dBm
} wifi_power_t;
*/

const int powerCount = 16;

int power[powerCount] = {
  WIFI_POWER_MINUS_1dBm, WIFI_POWER_21dBm, WIFI_POWER_20_5dBm, WIFI_POWER_20dBm,
  WIFI_POWER_19_5dBm, WIFI_POWER_19dBm, WIFI_POWER_18_5dBm, WIFI_POWER_17dBm,
  WIFI_POWER_15dBm,   WIFI_POWER_13dBm, WIFI_POWER_11dBm,   WIFI_POWER_8_5dBm,
  WIFI_POWER_7dBm,    WIFI_POWER_5dBm,  WIFI_POWER_2dBm,    WIFI_POWER_MINUS_1dBm};

const char *powerstr[powerCount] = {
  "WIFI_POWER_default", "WIFI_POWER_21dBm", "WIFI_POWER_20_5dBm", "WIFI_POWER_20dBm",
  "WIFI_POWER_19_5dBm", "WIFI_POWER_19dBm", "WIFI_POWER_18_5dBm", "WIFI_POWER_17dBm",
  "WIFI_POWER_15dBm",   "WIFI_POWER_13dBm", "WIFI_POWER_11dBm",   "WIFI_POWER_8_5dBm",
  "WIFI_POWER_7dBm",    "WIFI_POWER_5dBm",  "WIFI_POWER_2dBm",    "WIFI_POWER_MINUS_1dBm"};

  // TX power can be set to the corresponding wifi_power_t
bool validpower[powerCount] = {false};
long ctimes[powerCount] = {0};
long iptimes[powerCount] = {0};
int rssi[powerCount] = {0};

void print_ctimes(void) {
  int invalidpowers = 0;
  int noconnects = 0;
  Serial.println("\n\nConnect time vs txPower\n");
  Serial.printf("\nBoard: %s\n", STA_MAC_STR);
  Serial.printf("%24s | %3s | %4s | %4s | %6s | %6s \n", "enum", "set", "dBm", "rssi", "time", "dhcp");
  Serial.printf("%26s %5s %6s %6s %8s\n", "|", "|", "|", "|", "|");
  for (int i=0; i<powerCount; i++) {
     Serial.printf("%24s | %3d | %4.1f |", powerstr[i], power[i], 0.25*power[i]);
     if (validpower[i]) {
       if (ctimes[i] < TIMEOUT) {
         Serial.printf(" %4d | %6lu | %6lu", rssi[i], ctimes[i], iptimes[i]);
       } else {
        noconnects++;
        Serial.printf(" %4s | %6s | %6s", "-", "-", "-");
       }  
     } else {
       invalidpowers++;
       Serial.printf(" %4s | %6s | %6s", "*", "*", "*");
     }
     Serial.println();
  }
  if (invalidpowers) {
    Serial.println("  (*) Invalid TX power value");
  }
  if (noconnects) {
    Serial.printf("  (-) Could not connect in %.1f seconds\n", TIMEOUT/1000.0);
  }
}


unsigned long connect_time = 0;
unsigned long etime = 0;
unsigned long dottime = 0;

void start_connecting(int pwindex = 0) {
  if ((pwindex < 0) || (pwindex >= powerCount)) return;
  
  if (WiFi.isConnected()) {
    Serial.print("Disconnecting...");
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true, true);
    while (WiFi.isConnected()) delay(5);
    Serial.println(" disconnected. Wait 1 second.");
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  delay(25);
  int cpower = power[pwindex];
  WiFi.setTxPower((wifi_power_t)cpower); // can return false, will pick up "error"
  delay(25);
  Serial.print("Connecting to Wi-Fi network ");
  connect_time = millis();
  dottime = millis();
  WiFi.begin(ssid, password);
}

void getStatus(void) {
  switch(WiFi.status()) {
      case WL_IDLE_STATUS:
        Serial.println("[WiFi] Idle");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("[WiFi] SSID not found");
        break;
      case WL_SCAN_COMPLETED:
        Serial.println("[WiFi] Scan is completed");
        break;
      case WL_CONNECTED:
        Serial.println("[WiFi] Connected");
        break;  
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Connection Failed");
        break;
      case WL_CONNECTION_LOST:
        Serial.println("[WiFi] Connection was lost");
        break;
      case WL_DISCONNECTED:
        Serial.println("[WiFi] Disconnected");
        break;
      case WL_NO_SHIELD:
        Serial.println("[WiFi] No shield"); // should never happen
        break; 
      default:
        Serial.print("[WiFi] Unknown status: ");
        Serial.println(WiFi.status());
        break;
  }
}

int txIndex = -1;

void run_test(void) {
  txIndex++;
  if (txIndex >= powerCount) return;

  Serial.printf("\nTest with %s\n", powerstr[txIndex]);

  if (!validpower[txIndex]) {
    Serial.println("This power level cannot be set");
    return;
  }

  start_connecting(txIndex);
  unsigned long cctime = 0;

  while (true) {
    if (millis() - dottime > 2000) {
      Serial.print('.');
      dottime = millis();
    }

    if (WiFi.STA.connected()) {
      if (!cctime) { 
        cctime = millis() - connect_time;
      }  
    }  
    
    if (WiFi.isConnected()) {
      etime = millis() - connect_time;
      ctimes[txIndex] = cctime;
      iptimes[txIndex] = etime;
      Serial.print("\nConnected with IP address: ");
      Serial.println(WiFi.localIP());
      rssi[txIndex] = WiFi.RSSI();
      Serial.printf("RSSI: %d dBm\n", rssi[txIndex]);   
      Serial.printf("Time to connect: %lu ms\n", cctime);
      Serial.printf("Time to IP address acquisition: %lu ms\n", etime);
      return;
    }

    if (millis() - connect_time > TIMEOUT) {
      Serial.printf("\nNot connected after %d ms\n", TIMEOUT);
      getStatus();
      ctimes[txIndex] = 2*TIMEOUT;
      return;
    }
    yield(); // keep the watchdog happy
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
  #endif  
  delay(SERIAL_BEGIN_DELAY);

  Serial.println("\n\nProject: wifi_tx_power");
  Serial.println("Purpose: Measure time to connect to a WiFi network as a function of WiFi TX power");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32 board");
  #endif

  #if defined(ARDUINO_XIAO_ESP32C6)
    Serial.print("Antenna: ");
    #if defined(USE_EXTERNAL_ANTENNA)
      //pinMode(WIFI_ANT_CONFIG, OUTPUT); //done in .../variants/XIAO_ESP32C6/variant.cpp
      digitalWrite(WIFI_ANT_CONFIG, HIGH);
      Serial.println("External");
    #else
      Serial.println("Onboard ceramic");
    #endif  
  #endif
  Serial.printf("STA MAC: %s\n", STA_MAC_STR);
  Serial.printf("Network: %s\n", ssid);

  WiFi.mode(WIFI_STA);
  delay(25);
  power[0] = WiFi.getTxPower();  // default tx power on boot
  Serial.printf("\nThe default TX power level is %d = %.1f dBm\n", power[0], power[0]*0.25);

  Serial.println("Testing setting TX power levels");
  for (int i=0; i < powerCount; i++) {
    int pwr = power[i];
    WiFi.setTxPower((wifi_power_t)pwr); // can return false, will pick up "error"
    delay(25);
    int res = WiFi.getTxPower();
    validpower[i] = (res == pwr);
    Serial.printf("Setting power level %18s %s\n", powerstr[i], (validpower[i]) ? "succeeds"  : "fails");
  }  

  while (txIndex < powerCount) {
    run_test();
  }  
  print_ctimes();
}

void loop() {
  // do nothing
}  
