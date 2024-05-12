// Main module of blink_pulse_led
// Copyright: see notice in blink_pulse.ino

#include <Arduino.h>

#if defined(ARDUINO_SUPER_MINI_ESP32C3)
  static uint8_t ledPin = BUILTIN_LED;   // LED connected to digital pin
#else
  static uint8_t ledPin = 8;             // LED connected to digital pin
#endif

static uint8_t ledOn = LOW;

void setup() {
  // declaring LED pin as output
  pinMode(ledPin, OUTPUT);
}

#define DELTA 10
#define DELAY 50

unsigned long delaytime = 0;
int delta = 5;
int fade = 0;

void pulse(void) {
  if (millis() - delaytime > DELAY) {
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


#define ON_TIME  80
#define OFF_TIME 840

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

unsigned long timer = 0;
bool doHeartBeat = true;

void loop() {
  if (doHeartBeat)
    heartbeat();
  else
    pulse();
  if (millis() - timer > 10000) {
    doHeartBeat = (!doHeartBeat);
    pinMode(ledPin, OUTPUT);
    timer = millis();
  }
  // do other stuff here
}
