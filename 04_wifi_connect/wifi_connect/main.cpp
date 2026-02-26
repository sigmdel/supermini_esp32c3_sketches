// Main module of wifi_connect
// Copyright: see notice in wifi_connect.ino

#include <Arduino.h>
#include <WiFi.h>

#include "MACs.h"
#include "secrets.h"

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board - not meaningful in this sketch
///#define HAS_OLED
///
///  Define the Wi-Fi TX power level. See
///  https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h
///  for possible values. Note that the first attempt at connecting will always be done with
///  the default TX power level. Only in subsequent connection attemps will the TX power be 
///  set.
///#define TX_POWER WIFI_POWER_11dBm
///
///  Define this macro to verify that WiFi.disconnect(true, true) truly wipes the WiFi data 
///  (ssid and password) from non volatile storage. To see the error set CORE_DEBUG_LEVEL=1
///  in Tools menu if using the Arduino IDE or in platformio.ini if using pioarduino IDE. 
///  Note that WiFi.disconnect(true, true) is always called before each attemp to connect
///  and for the first 5 connection attempts, WiFi.begin(<my_ssid>, <my_password>) is used.
///  If TEST_DISCONNECT is enabled, then all subsequent attemps will be done with 
///  WiFi.begin() and should fail because the last used ssid and password should have been
///  erased.
///#define TEST_DISCONNECT
///
///  Define the time allowed for connecting to the Wi-Fi network before giving up
#define TIMEOUT 120000   // 2 minutes
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

#ifndef TIMEOUT
#define TIMEOUT 120000
#endif

unsigned long connect_time = 0;
unsigned long etime = 0;
unsigned long dottime = 0;
int count = -1;

void start_connecting(void) {
  count++;
  Serial.printf("\nAttempt #%d\n", count);

  if (WiFi.isConnected()) {
    Serial.print("Disconnecting... ");
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true, true);  // this should delete ssid and password from non volatile memory
    while (WiFi.isConnected()) delay(5);
    delay(1000);
    Serial.println("disconnected.");
  }
  Serial.printf("Trying to connect to the Wi-Fi network for %.1f sec.\n", (1.0*TIMEOUT)/1000);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  #ifdef TX_POWER
    if (count) {
      delay(25);
      if (WiFi.getTxPower() != TX_POWER) {
        WiFi.setTxPower(TX_POWER);
        delay(25);
      }
    }
    Serial.printf("Wi-Fi TX power set to: %d\n", WiFi.getTxPower());
  #endif
  connect_time = millis();
  dottime = millis();
  #ifdef TEST_DISCONNECT
  if (count > 4)  {
    // making sure that data was wiped
    Serial.println("Testing WiFi.disconnect() by using WiFi.begin() without specifying ssid or password");
    WiFi.begin();
    /* will fail connecting to WiFi access point
     *   [ 41137][E][STA.cpp:347] connect(): STA connect failed! 0x300a: ESP_ERR_WIFI_SSID
     * proving that a valid SSID is no longer in non volatile storage (flash)
     */
  } else 
  #endif
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
 
  Serial.begin(); 
  delay(SERIAL_BEGIN_DELAY);

  Serial.println("\n\nProject: wifi_connect");
  Serial.println("Purpose: Show connection to a Wi-Fi network");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.print(ARDUINO_BOARD);
  #else 
  Serial.print("Unknown ESP32 board");
  #endif
  #ifdef HAS_OLED
    Serial.print(" with 0.42\" OLED");
  #endif  
  Serial.println();  Serial.printf("STA MAC: %s\n", STA_MAC_STR);
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif

  #ifdef TEST_DISCONNECT
  Serial.println("\nTesting WiFi.disconnect()");
  Serial.println("  The expected error code after the first 5 attempts is 0x300a");
  Serial.print("  which translates to sp_err_to_name(0x300a) -> ");
  Serial.println(esp_err_to_name(0x300a));
  #ifndef PLATFORMIO
  Serial.println("  Set 'Core Debug Level' to \"Error\" or higher in the Tools menu to see the error message");
  #endif
  #endif
  start_connecting();
}

void loop() {
  if (WiFi.isConnected()) {
    etime = millis() - connect_time;
    Serial.print("WiFi is connected with IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Time to connect: %lu ms\n", etime);
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    delay(1000);
    start_connecting();
  } else if (millis() - dottime > 2000) {
    Serial.print('.');
    dottime = millis();
  }
  if (millis() - connect_time > TIMEOUT) {
    Serial.printf("\nNot connected after %.1f seconds\n", (1.0*TIMEOUT)/1000);
    getStatus();
    Serial.println("Retrying in 1 second");
    delay(1000);
    start_connecting();
  }
}
