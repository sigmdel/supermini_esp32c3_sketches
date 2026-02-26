/*
 *  See scan_wifi.ino for license and attribution.
 */

#include <Arduino.h>
#include "WiFi.h"

#include "MACs.h"

//////// User configuration //////
///
///  Define if there is a 0.42" OLED on the board
///#define HAS_OLED
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

  Serial.println("\n\nProject: scan_wifi");
  Serial.println("Purpose: Show use of digitalWrite(), analogWrite(), and digitalRead()");
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

  Serial.println("\nStarting Wi-Fi in STA(tion) mode");
  WiFi.STA.begin();
  WiFi.disconnect();
  delay(2000);
}

void loop() {
  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             |       BSSID       | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d | ", i + 1);
      Serial.printf("%-32.32s | ", WiFi.SSID(i).c_str());
      Serial.printf("%-17.17s | ", WiFi.BSSIDstr(i).c_str());
      Serial.printf("%4ld | ", WiFi.RSSI(i));
      Serial.printf("%2ld | ", WiFi.channel(i));
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }
  Serial.println("");

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  // Wait a bit before scanning again.
  delay(5000);
}
