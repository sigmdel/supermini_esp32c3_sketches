#include <Arduino.h>
#include "esp_mac.h"

String getDefaultMacAddress() {

  String mac = "";

  unsigned char mac_base[8] = {0};
 
  if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
    char buffer[24];  // 8*3 characters for 2 digit hex + (separating colon or null terminator)
#ifdef CONFIG_SOC_IEEE802154_SUPPORTED
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5], mac_base[6], mac_base[7]);
#else 
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
#endif
    mac = buffer;
  }

  return mac;
}

String getInterfaceMacAddress(esp_mac_type_t interface) {

  String mac = "";
  unsigned char mac_base[8] = {0x5A};
  char buffer[24];  // 8*3 characters for 2 digit hex + (separating colon or null terminator)

  // Intercept an 'esp_read_mac(mac_base, ESP_MAC_EFUSE_CUSTOM)' call which
  // raises many error messages if there is no custom MAC address. What happens
  // if there is a custom MAC address when esp_read_mac(mac_base, ESP_MAC_EFUSE_CUSTOM)
  // is called is not unknown.
  if (interface == ESP_MAC_EFUSE_CUSTOM) {
    // If a custom MAC address has not been set, a single error message will be
    // printed. There does not seem to be a function to verify if a custom
    // MAC address exists. Using 'esp_mac_addr_len_get(ESP_MAC_EFUSE_CUSTOM)'
    // hoping to get 0 or something other than 6 will not work. So no known way
    // to avoid the next call.
    if (esp_efuse_mac_get_custom(mac_base) == ESP_OK) {
      sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
      mac = buffer;  
    }  
    return mac;  
  } 

#ifdef CONFIG_SOC_IEEE802154_SUPPORTED
  if (esp_read_mac(mac_base, interface) == ESP_OK) {
    if (interface == ESP_MAC_IEEE802154) {
      // 8 bytes
      sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5], mac_base[6], mac_base[7]);
    } else if(interface == ESP_MAC_EFUSE_EXT) {
      // 2 bytes
      sprintf(buffer, "%02X:%02X", mac_base[0], mac_base[1]);
    } else {
      // others are 6 bytes
      sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    }
    mac = buffer;
  }  
#else
  if ((interface != ESP_MAC_IEEE802154) && (interface != ESP_MAC_EFUSE_EXT)) {
    if (esp_read_mac(mac_base, interface) == ESP_OK) {
      // all 6 bytes
      sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
      mac = buffer;
    }  
  }
#endif

  return mac;
}
