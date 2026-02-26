/*
 *  See async_web_led.ino for license and attribution.
 */

#include <Arduino.h>
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "html.h"
#include "secrets.h"
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
///  Define the Wi-Fi TX power level. See
///  https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h
///  for possible values. Note that the first attempt at connecting will always be done with
///  the default TX power level. Only in subsequent connection attemps will the TX power be 
///  set.
#define TX_POWER WIFI_POWER_11dBm
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

 #ifdef ARDUINO_BOARD
  #define TITLE ARDUINO_BOARD
#else
  #define TITLE "Unknown ESP32 board"
#endif

static uint8_t ledPin = LED_PIN;
static uint8_t ledOn = LED_ON;
int ledState = 0;
String ledStatus = "OFF";

void setLed(int value) {
  digitalWrite(ledPin, (value)? ledOn : 1-ledOn);
  ledState = value;
  ledStatus = ((digitalRead(ledPin) == ledOn) ? "ON" : "OFF");
  Serial.printf("LED now %s.\n", ledStatus.c_str());
}

void toggleLed(void) {
  setLed(1-ledState);
}

// Webserver instance using default HTTP port 80
AsyncWebServer server(80);

// Template substitution function
String processor(const String& var){
  if (var == "LEDSTATUS") return String(ledStatus);
  if (var == "SERVERNAME") return String(TITLE);
  return String(); // empty string
}

// 404 error handler
void notFound(AsyncWebServerRequest *request) {
  Serial.printf("\"%s\" not found\n", request->url().c_str());
  request->send(404, "text/html", html_404, processor);
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

  Serial.println("\n\nProject: async_web_led");
  Serial.println("Purpose: Run a Web server to control a LED from an HTML page");
  Serial.printf("  Board: %s/n", TITLE);
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif
  Serial.printf("STA MAC: %s\n", STA_MAC_STR);
  Serial.printf("Network: %s\n", ssid);

  // Set the digital pin connected to the LED as an output
  pinMode(ledPin, OUTPUT);
  setLed(0);

  WiFi.mode(WIFI_STA);

  #ifdef TX_POWER
  WiFi.setTxPower(TX_POWER);
  int txpower = WiFi.getTxPower();
  Serial.printf("Wi-Fi Tx power set to %d (%.1f dBm)\n", txpower, txpower*0.25);
  #endif

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup and start Web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Index page requested");
    request->send(200, "text/html", html_index, processor);
  });
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web button pressed");
    toggleLed();
    request->send(200, "text/html", html_index, processor); // updates the client making the request only
  });
  server.onNotFound(notFound);
  server.begin();

  setLed(0);
  Serial.println("setup completed.");
}

void loop() {
}
