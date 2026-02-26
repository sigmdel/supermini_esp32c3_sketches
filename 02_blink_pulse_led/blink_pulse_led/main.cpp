/*
 *  See blink_pulse_led.ino for license and attribution.
 */

#include <Arduino.h>

/////// User configuration //////
///
///  Define the io pin to which the momentary push button is connnected, default is the onboard BOOT button
#define BUTTON_PIN BOOT_PIN
///
///  Define value of button pin (HIGH or LOW) when the button is not pressed, default is HIGH
#define BUTTON_OFF HIGH
///
///  Define the io pin to which a LED is connected, default is the onboard user LED 
#define LED_PIN LED_BUILTIN
///
///  What signal (HIGH or LOW) turns the LED on
#define LED_ON LOW
///
///  Time in milliseconds the LED is on during the heartbeat
#define ON_TIME  80
///
///  Time in millisecods the LED is off during the heartbeat
#define OFF_TIME 840
///
/// Time in milliseconds during which duty cycle value is used when pulsing LED.
#define LEVEL_PERIOD 50 
///
/// The increment or decrement in the duty cycle between levels when pulsing LED.
#define DELTA 10
///
/// The initial duty cycle above 0 when pulsing LED.
#define INIT_DELTA 5
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

static uint8_t buttonPin = BUTTON_PIN;
static uint8_t ledPin = LED_PIN;
static uint8_t ledOn = LED_ON;

#if BUTTON_OFF == HIGH
  #define BUTTON_MODE INPUT_PULLUP
  static uint8_t buttonActive = LOW;
#elif BUTTON_OFF == LOW
  #define BUTTON_MODE INPUT_PULLDOWN
  static uint8_t buttonActive = HIGH;
#else
  #error BUTTON_OFF value invalid (wanted HIGH or LOW)  
#endif


/// -- pulsing LED with PWM ---

unsigned long delaytime = 0;
int delta = 5;
int fade = 0;

void pulse(void) {
  if (millis() - delaytime > LEVEL_PERIOD) {
    fade += delta;
    if (fade <= 0) {
      fade = 0;
      delta = DELTA;
    } else if (fade >= 255) {
      fade = 255;
      delta = - DELTA;
    }
    analogWrite(ledPin, fade);
    delaytime = millis();
  }
}

void initPulse(void) {
  delta = 5;
  fade = 0;
  delaytime = millis();
}

/// --- toggle LED on/off in heartbeat fashion ---

unsigned long beattime = 0;
int beatcount = 0;
int beatdelay = 0;

/* beatcount
  0 = on short
  1 = off short
  2 = on short
  3 = off long
*/

void heartbeat(void) {
  if (millis() - beattime > beatdelay) {
    if ((beatcount & 1) == 1)
       digitalWrite(ledPin, 1-ledOn);
    else
       digitalWrite(ledPin, ledOn);
    if (beatcount == 3) {
      beatdelay = OFF_TIME;
      beatcount = 0;
    } else {
      beatdelay = ON_TIME;
      beatcount++;
    }
    beattime = millis();
  }
}

void initHeartbeat(void) {
  beatcount = 0;
  beatdelay = 0;
  beattime = millis();
}

unsigned long timer;

void showinstructions(void) {
  if (millis() - timer > 15000)  {
    Serial.println("\nPress push button to switch mode.");
    timer = millis();
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

  Serial.println("\n\nProject: blink_pulse_led");
  Serial.println("Purpose: Show use of digitalWrite(), analogWrite(), and digitalRead()");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else
  Serial.println("Unknown ESP32 board");
  #endif
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif

  pinMode(buttonPin, BUTTON_MODE);

  // declaring LED pin as output
  pinMode(ledPin, OUTPUT);

  // start instruction timer
  timer = millis();
}

bool doHeartBeat = true;


void loop() {
  showinstructions();
  if (doHeartBeat)
    heartbeat();
  else
    pulse();

  if (digitalRead(buttonPin) == buttonActive) {
    while (digitalRead(buttonPin) == buttonActive)
      delay(100);
    delay(200);
    Serial.println("\nBoot button released");    
    doHeartBeat = (!doHeartBeat);
    if (doHeartBeat) 
      initHeartbeat();
    else
      initPulse();  
    Serial.printf("Switching to %s\n", (doHeartBeat) ? "heartbeat" : "pulsing");
    pinMode(ledPin, OUTPUT);
    timer = millis(); // restart timer
  }
}
