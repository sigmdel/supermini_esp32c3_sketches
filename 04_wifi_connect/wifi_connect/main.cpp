// Main module of wifi_connect
// Copyright: see notice in wifi_connect.ino

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

//#define TRY_TX_POWER
  // Define the above macro to set the Wi-Fi tx power level
  // before attempting to connect to a Wi-Fi network.

#ifdef TRY_TX_POWER
  #define TX_POWER WIFI_POWER_11dBm
    // Set the above macro to the power level. See
    // https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h
#endif

//#define TEST_DISCONNECT
  // Define the above macro to verify that WiFi.disconnect(true, true)
  // truly wipes the WiFi data from non volatile storage

#define TIMEOUT 120000   // 2 minutes

unsigned long connect_time = 0;
unsigned long etime = 0;
unsigned long dottime = 0;
int count = 0;

void start_connecting(void) {
    if (WiFi.isConnected()) {
      Serial.println("Disconnecting");
      WiFi.setAutoReconnect(false);
      WiFi.disconnect(true, true);
      while (WiFi.isConnected()) delay(5);
      Serial.println("Disconnected");
      delay(5000);
    }
    Serial.printf("\nAttempting to connect to the Wi-Fi network for %.1f sec.\n", (1.0*TIMEOUT)/1000);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    #ifdef TRY_TX_POWER
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
    if (count > 4)            // making sure that data was wiped
      WiFi.begin();
      /* will fail
            Connecting to WiFi access point
            [ 42188][E][WiFiSTA.cpp:317] begin(): connect failed! 0x300a
         and esp_err_to_name(0x300a) --> "ESP_ERR_WIFI_SSID"
         proving that a valid SSID is no longer in NVS
      */
    else
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
    Serial.begin();
    delay(2000);
    Serial.println();
    #ifdef TEST_DISCONNECT
    Serial.print("sp_err_to_name(0x300a) -> ");
    Serial.println(esp_err_to_name(0x300a));
    #endif
    start_connecting();
}

void loop() {
  if (WiFi.isConnected()) {
    etime = millis() - connect_time;
    Serial.print("\nWiFi is connected with IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Time to connect: %u ms\n", etime);
    delay(1000);
    start_connecting();
    count++;
  } else if (millis() - dottime > 2000) {
    Serial.print('.');
    dottime = millis();
  }
  if (millis() - connect_time > TIMEOUT) {
    Serial.printf("\nNot connected after %.1f seconds\n", (1.0*TIMEOUT)/1000);
    getStatus();
    Serial.println("Retrying in 1 second");
    delay(1000);
    count++;
    start_connecting();
  }
}
