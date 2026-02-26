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

int rssi[powerCount] = {0};

void print_ctimes(void) {
  bool err = false;
  Serial.println("Connect time vs txPower\n");
  Serial.printf("\nBoard: %s\n", STA_MAC_STR);

  Serial.printf("%24s | %5s | %5s | %5s | %8s\n", "enum", "want", "set", "rssi", "time (ms)");
  Serial.printf("%26s %7s %7s %7s\n", "|", "|", "|", "|");
  for (int i=0; i<powerCount; i++) {
     Serial.printf("%24s | %5d | %5d | %5d | %8lu", powerstr[i], power[i], truepower[i], rssi[i], ctimes[i]);
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
  
  Serial.printf("\nTest #%d/%d\n", pwindex, powerCount-1);
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
  int tpower = WiFi.getTxPower();
  truepower[pwindex] = tpower;
  Serial.printf("Power level index: %d. Wanted txPower %s (%d), set to %d = %.1f dBm\n", pwindex, powerstr[pwindex], cpower, tpower, tpower*0.25);
  Serial.print("Connecting to Wi-Fi network... ");
 
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
  start_connecting(0);
}

int txIndex = 0;
unsigned long cctime = 0;

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

  if (WiFi.STA.connected()) {
    if (!cctime) { 
      cctime = millis() - connect_time;
    }  
  }  
  if (WiFi.isConnected()) {
    etime = millis() - connect_time;
    delay(1500); // see wifi_blackhole
    Serial.print(" connected with IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Time to connect: %lu ms\n", cctime);
    Serial.printf("Time to IP address acquisition: %lu ms\n", etime);
    ctimes[txIndex] = cctime;
    rssi[txIndex] = WiFi.RSSI();
    Serial.printf("RSSI: %d dBm\n", rssi[txIndex]);   
    int txpower = WiFi.getTxPower(); // just checking
    Serial.printf("Tx Power: %d  %.1f dBm\n", txpower, txpower*0.25);  

    delay(1000);
    txIndex++;
    if (txIndex >= powerCount) return;
    cctime = 0;
    start_connecting(txIndex);
  }

  if (millis() - connect_time > TIMEOUT) {
    Serial.printf("\nNot connected after %d ms\n", TIMEOUT);
    getStatus();
    ctimes[txIndex] = 2*TIMEOUT;
    txIndex++;
    if (txIndex >= powerCount) return;
    Serial.println("Retrying in 5 seconds with different txPower");
    delay(5000);
    cctime = 0;
    start_connecting(txIndex);
  }
}
