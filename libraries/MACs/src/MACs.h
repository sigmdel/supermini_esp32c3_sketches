#pragma once

#include "esp_mac.h"

/*
 * Based on the arduino-esp32 GetMacAddress.ino sketch written by Daniel Nebert
 * and updated by me-no-dev in version 3.3.6.
 *
 * URL: https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/MacAddress/GetMacAddress/GetMacAddress.ino
 *
 * Michel Deslierres (sigmdel)
 * 2026-01-21
 */



// Returns as a String the default mac address factory burned into eFuse of
// the ESP32. The string will contain 23 characters representing an 8 byte
// MAC address if the ESP32 supports IEEE 802.15.4 (Zigbee and other protocols)
// otherwise the string will contain 17 characters representing a 6 byte
// MAC address. Calls on 'esp_efuse_mac_get_default' to get the MAC.
//
extern String getDefaultMacAddress();


// Returns as a String the mac address for the specified interface. The
// type of the `interface` parameter is `esp_mac_type_t`.
//
// If 'interface' is ESP_MAC_IEEE802154 or ESP_MAC_EFUSE_EXT and the ESP32 does
// not supports IEEE 802.15.4, the returned string will be empty.
//
// If 'interfaceq' is ESP_MAC_EFUSE_CUSTOM and a custom MAC has not been burned
// in the ESP32 eFuse, the returned string will be empty and an error message
// will be posted. No means was found to avoid this.

/*
  esp_mac_type_t values:

  ESP_MAC_WIFI_STA - MAC for WiFi Station (6 bytes)
  ESP_MAC_WIFI_SOFTAP - MAC for WiFi Soft-AP (6 bytes)
  ESP_MAC_BT - MAC for Bluetooth (6 bytes)
  ESP_MAC_ETH - MAC for Ethernet (6 bytes)
  ESP_MAC_IEEE802154 - if CONFIG_SOC_IEEE802154_SUPPORTED=y, MAC for IEEE802154 (8 bytes)
  ESP_MAC_BASE - Base MAC for that used for other MAC types (6 bytes)
  ESP_MAC_EFUSE_FACTORY - MAC_FACTORY eFuse which was burned by Espressif in production (6 bytes)
  ESP_MAC_EFUSE_CUSTOM - MAC_CUSTOM eFuse which was can be burned by customer (6 bytes)
  ESP_MAC_EFUSE_EXT - if CONFIG_SOC_IEEE802154_SUPPORTED=y, MAC_EXT eFuse which is used as an extender for IEEE802154 MAC (2 bytes)
  */

extern String getInterfaceMacAddress(esp_mac_type_t interface);


// Macro definitions for commonly used interfaces, returns a String

#define STA_MAC getInterfaceMacAddress(ESP_MAC_WIFI_STA)
#define SOFTAP_MAC getInterfaceMacAddress(ESP_MAC_WIFI_SOFTAP)
#define BT_MAC getInterfaceMacAddress(ESP_MAC_BT)
#define ETH_MAC getInterfaceMacAddress(ESP_MAC_ETH)
#define ZIGBEE_MAC getInterfaceMacAddress(ESP_MAC_IEEE802154) // who would remember IEEE802154_MAC?


// Macro definitions for commonly used interfaces, returns a C string

#define STA_MAC_STR getInterfaceMacAddress(ESP_MAC_WIFI_STA).c_str()
#define SOFTAP_MAC_STR getInterfaceMacAddress(ESP_MAC_WIFI_SOFTAP).c_str()
#define BT_MAC_STR getInterfaceMacAddress(ESP_MAC_BT).c_str()
#define ETH_MAC_STR getInterfaceMacAddress(ESP_MAC_ETH).c_str()
#define ZIGBEE_MAC_STR getInterfaceMacAddress(ESP_MAC_IEEE802154).c_str() 

