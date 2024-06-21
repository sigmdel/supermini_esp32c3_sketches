// Main module of async_web_led PlatformIO/Arduino sketch
// Copyright: see notice in async_web_led.ino

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>            // See: Note on Location of Async Libraries
#include <ESPAsyncWebServer.h>   // See: Note on Location of Async Libraries
#include "html.h"
#include "secrets.h"

#ifndef PLATFORMIO
// WiFi.setTxPower(TX_POWER) may be needed for some SuperMini C3 boards
// If using Arduino define TX_POWER here if needed:
#define TX_POWER WIFI_POWER_11dBm
// For PlatformIO, TX_POWER is defined in the platformio.ini config file.
// Similarly
//#define LED_ON LOW
#endif

#if defined(ARDUINO_MAKERGO_C3_SUPERMINI)
  #define TITLE "MakerGO C3 SuperMini WEB SERVER"
  //static const uint8_t BOOT_BUILTIN = 9;          // defined in pins_arduino.h
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_NOLOGO_ESP32C3_SUPER_MINI)
  #define TITLE "Nologo ESP32C3 Super Mini WEB SERVER"
  static const uint8_t BOOT_BUILTIN = 9;
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_SUPER_MINI_C3)
  #define TITLE "DfRobot Beetle ESP32-C3 WEB SERVER"  //"Modified ESP32 C3 DEVKITC 02"
  static const uint8_t BOOT_BUILTIN = 9;
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_SUPER_MINI_ESP32C3)
  #define TITLE "Custom ESP32C3 Super Mini WEB SERVER"
  //static const uint8_t BOOT_BUILTIN = 9;          //defined in pins_arduino.h
  //#define BOOT_BUILTIN BOOT_BUILTIN               //defined in pins_arduino.h
#elif defined(ARDUINO_ESP32C3_DEV)
  #define TITLE "ESP32-C3 Dev Module WEB SERVER"
  static const uint8_t BOOT_BUILTIN = 9;          // defined in pins_arduino.h
  #define BOOT_BUILTIN BOOT_BUILTIN
#elif defined(ARDUINO_XIAO_ESP32C3)
  #define TITLE "Seeed XIAO ESP32C3 WEB SERVER"
  static const uint8_t BOOT_BUILTIN = 9;          // defined in pins_arduino.h
  #define BOOT_BUILTIN BOOT_BUILTIN
#else
  #error "Unknown device"
#endif

#if defined(ARDUINO_SUPER_MINI_ESP32C3)
  static uint8_t ledPin = BUILTIN_LED;
  static uint8_t ledOn = LOW;
#elif not defined(ARDUINO_XIAO_ESP32C3)
  static uint8_t ledPin = 8;
  static uint8_t ledOn = LOW;
#else
  static uint8_t ledPin = LED_PIN;
  static uint8_t ledOn = LED_ON;
#endif

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
  if (var == "LEDSTATUS") return String(l;default_envs = seeed_xiao_esp32c3edStatus);
  if (var == "SERVERNAME") return String(TITLE);
  return String(); // empty string
}

// 404 error handler
void notFound(AsyncWebServerRequest *request) {
  request->send_P(404, "text/html", html_404, processor);
}

void setup() {
  // Set the digital pin connected to the LED as an output
  pinMode(ledPin, OUTPUT);

  Serial.begin();
  delay(1000);      // 1 second delay should be sufficient

  setLed(0);

  WiFi.mode(WIFI_STA);

  #ifdef TX_POWER
  WiFi.setTxPower(TX_POWER);
  Serial.printf("Setting Wi-Fi Tx power to %d\n", TX_POWER);
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
    request->send_P(200, "text/html", html_index, processor);
  });
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web button pressed");
    toggleLed();
    request->send_P(200, "text/html", html_index, processor); // updates the client making the request only
  });
  server.onNotFound(notFound);
  server.begin();

  setLed(0);
  Serial.println("setup completed.");
}

void loop() {
}
