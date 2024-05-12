// Main module of wifi_tx_power
// Copyright: see notice in wifi_tx_power.ino

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"


// Define the title macro as needed for each supported board

#if defined(ARDUINO_MAKERGO_C3_SUPERMINI)
  #define TITLE "MakerGO C3 SuperMini"
#elif defined(ARDUINO_NOLOGO_ESP32C3_SUPER_MINI)
  #define TITLE "Nologo ESP32C3 Super Mini"
#elif defined(ARDUINO_SUPER_MINI_C3)
  #define TITLE "DfRobot Beetle ESP32-C3"
#elif defined(ARDUINO_SUPER_MINI_ESP32C3)
  #define TITLE "Custom ESP32C3 Super Mini"
#elif defined(ARDUINO_XIAO_ESP32C3)
  #define TITLE "Seeed XIAO ESP32C3"
elif defined(ARDUINO_ESP32C3_DEV)
  #define TITLE "ESP32-C3 Dev Module"  
#else
  #error "Unknown device"
#endif

#define TIMEOUT 120000   // 2 minutes, time until connection deemed impossible


/*
 from ../.platformio/packages/framework-arduinoespressif32/libraries/WiFi/src/WiFiGeneric.h

typedef enum {
    WIFI_POWER_19_5dBm = 78,// 19.5dBm
    WIFI_POWER_19dBm = 76,// 19dBm
    WIFI_POWER_18_5dBm = 74,// 18.5dBm
    WIFI_POWER_17dBm = 68,// 17dBm
    WIFI_POWER_15dBm = 60,// 15dBm
    WIFI_POWER_13dBm = 52,// 13dBm
    WIFI_POWER_11dBm = 44,// 11dBm
    WIFI_POWER_8_5dBm = 34,// 8.5dBm
    WIFI_POWER_7dBm = 28,// 7dBm
    WIFI_POWER_5dBm = 20,// 5dBm
    WIFI_POWER_2dBm = 8,// 2dBm
    WIFI_POWER_MINUS_1dBm = -4// -1dBm
} wifi_power_t;
*/

const int powerCount = 13;

int power[powerCount] = {
  0,
  WIFI_POWER_19_5dBm, WIFI_POWER_19dBm, WIFI_POWER_18_5dBm, WIFI_POWER_17dBm,
  WIFI_POWER_15dBm,   WIFI_POWER_13dBm, WIFI_POWER_11dBm,   WIFI_POWER_8_5dBm,
  WIFI_POWER_7dBm,    WIFI_POWER_5dBm,  WIFI_POWER_2dBm,    WIFI_POWER_MINUS_1dBm};

const char *powerstr[powerCount] = {
  "WIFI_POWER_default",
  "WIFI_POWER_19_5dBm", "WIFI_POWER_19dBm", "WIFI_POWER_18_5dBm", "WIFI_POWER_17dBm",
  "WIFI_POWER_15dBm",   "WIFI_POWER_13dBm", "WIFI_POWER_11dBm",   "WIFI_POWER_8_5dBm",
  "WIFI_POWER_7dBm",    "WIFI_POWER_5dBm",  "WIFI_POWER_2dBm",    "WIFI_POWER_MINUS_1dBm"};

int truepower[powerCount] = {0};

long ctimes[powerCount] = {0};

void print_ctimes(void) {
  bool err = false;

  Serial.printf("\nBoard: %s\n", TITLE);
  Serial.println("Connect time vs txPower\n");
  Serial.printf("%24s | %5s | %5s | %8s\n", "enum", "want", "set", "time (ms)");
  Serial.printf("%26s %7s %7s\n", "|", "|", "|");
  for (int i=0; i<powerCount; i++) {
     Serial.printf("%24s | %5d | %5d | %8d", powerstr[i], power[i], truepower[i], ctimes[i]);
     if (truepower[i] != power[i] ) {
      Serial.print(" *");
      err = true;
     }
     Serial.println();
  }
  if (err)
    Serial.println("  * = unable to set transmit power to desired value");
}

unsigned long connect_time = 0;
unsigned long etime = 0;
unsigned long dottime = 0;

void start_connecting(int pwindex = 0) {
  if ((pwindex < 0) || (pwindex >= powerCount)) return;

  if (WiFi.isConnected()) {
    Serial.println("Disconnecting");
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true, true);
    while (WiFi.isConnected()) delay(5);
    Serial.println("Disconnected");
    delay(5000);
  }
  Serial.println();
  WiFi.mode(WIFI_STA);
  delay(25);
  int cpower = power[pwindex];
  WiFi.setTxPower((wifi_power_t)cpower);
  delay(25);
  Serial.printf("Settting txPower to %s (%d)\n", powerstr[pwindex], cpower);
  int tpower = WiFi.getTxPower();
  if (tpower != cpower) Serial.println("Unable to set txPower");
  truepower[pwindex] = tpower;
  Serial.printf("Running pwindex %d, wanted txPower %s (%d), set to %d\n", pwindex, powerstr[pwindex], cpower, tpower);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  connect_time = millis();
  dottime = millis();
  WiFi.begin(ssid, password);
}

void getStatus(void) {
  switch(WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        Serial.println("[WiFi] SSID not found");
        break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return;
        break;
      case WL_CONNECTION_LOST:
        Serial.println("[WiFi] Connection was lost");
        break;
      case WL_SCAN_COMPLETED:
        Serial.println("[WiFi] Scan is completed");
        break;
      case WL_DISCONNECTED:
        Serial.println("[WiFi] WiFi is disconnected");
        break;
      case WL_CONNECTED:
        Serial.print("[WiFi] WiFi is connected. IP address: ");
        Serial.println(WiFi.localIP());
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;
  }
}


void setup() {
    Serial.begin();
    delay(2000);
    Serial.println();
    #ifdef TEST_DISCONNECT
    Serial.print("sp_err_to_name(0x300a) -> ");
    Serial.println(esp_err_to_name(0x300a));
    #endif
    WiFi.mode(WIFI_STA);
    delay(25);
    power[0] = WiFi.getTxPower();  // default tx power on boot
    //print_ctimes();
    start_connecting(0);
}

int txIndex = 0;

void loop() {
  if (txIndex == powerCount)  {
    print_ctimes();
    txIndex++;
  }
  if (txIndex > powerCount) return;

  if (millis() - dottime > 2000) {
    Serial.print('.');
    dottime = millis();
  }

  if (WiFi.isConnected()) {
    etime = millis() - connect_time;
    Serial.print("\nWiFi is connected with IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Time to connect: %u ms\n", etime);
    ctimes[txIndex] = etime;
    delay(1000);
    txIndex++;
    if (txIndex >= powerCount) return;
    start_connecting(txIndex);
  }

  if (millis() - connect_time > TIMEOUT) {
    Serial.printf("\nNot connected after %u ms\n", TIMEOUT);
    getStatus();
    ctimes[txIndex] = -1;
    txIndex++;
    if (txIndex >= powerCount) return;
    Serial.println("Retrying in 5 seconds with different txPower");
    delay(5000);
    start_connecting(txIndex);
  }
}
