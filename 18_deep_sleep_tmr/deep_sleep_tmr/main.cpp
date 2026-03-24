/*
 *  See deep_sleep_tmr.ino for license and attribution.
 */

/**************************************************************
OPERATION
  Each time the board wakes up from deep sleep, it executes the
  setup() function. Near the end of the setup() function, the
  board is put into deep sleep. Consequently the loop() function
  is never executed and anything in the setup() function after 
  when deep sleep is enabled will also not be executed.

LED 
  In correct operation, the LED does two things.
  First, as the board wakes from deep sleep, it slowly 
  flashes out the number of times the setup() function
  has been started since the SoC was powered up.
  Then the LED quickly flashes for 5 times announcing
  that the SoC is going into sleep mode.
  
NOTE
  The sketch can report the state of the machine to the 
  serial monitor. However, the SoC serial peripheral is 
  shut down when the SoC goes into sleep mode. For boards
  with an USB-serial adapter, such as the Lolin32 the
  serial connection with the computer remains open even
  when the board is in deep sleep mode. So when the 
  board wakes up, its serial peripheral is quickly 
  connected to the computer and the sketch works 
  exactly as intended even if USE_SERIAL is defined.

  For boards that support an internal USB CDC and do 
  not use a USB-serial adpater, such as the 
  ESP32C3 Super Mini or the XIAO ESP32Cx (x = 3, 5, 6)
  the serial connection to the computer goes down
  when the SoC is put in deep sleep mode. That means
  that the serial connection from the computer has
  to be reestablished when the SoC wakes to see the
  serial output of the sketch. The Arduino IDE 
  automatically and quickly enough that the messages 
  from the sketch will be seen. That is most often
  not the case when using the platformIO IDE.

  If using the platformIO IDE, try shortening the 
  SLEEP_PERIOD to say 5 seconds (5000000) and define
  a SERIAL_BEGIN_DELAY of 10 seconds (10000). Then
  when a wake message appears such as 

  Boot number: 6
  Wakeup from deep sleep caused by timer
  Going into sleep mode for 5.0 seconds

  wait about 8 seconds, close the serial monitor
  and open it again in the hope of connecting during
  the SERIAL_BEGIN_DELAY and seeing the next boot
  wake message.

  **In summary**, it is better to run this sketch in 
  the Arduino IDE with USE_SERIAL defined.
*************************************************************/

#include <Arduino.h>

//////// User configuration //////
///
///  Send message on waking from deep sleep to the serial monitor.
///  Works well in Arduino IDE, but not as well in PlaformIO.
#define USE_SERIAL
///
/// 15 seccond sleep period in microseconds
#define SLEEP_PERIOD 15000000
///
///  Settle time in milliseconds after calling 
///  esp_deep_sleep_enable_timer_wakeup(). This delay appears to be
///  necessary if USE_SERIAL is defined and the SLEEP_PEIOD is 
///  too short.
#define SETTLE_TIME 250
///
///  Define the number of boots after which the board will be restarted.
///  If RESTART_COUNT is not defined, the board will cycle into deep sleep
///  mode indefinitely.
///#define RESTART_COUNT 5
///
///  By default LED_PIN is set to BUILTIN_LED when the latter is defined 
///  as it is in the Nologo Super Mini and XIAO ESP32C6 variants.
///  For the XIAO ESP32C3, this macro must be define and a LED must be
///  connected to the pin because the board does not have a builtin LED.
///#define LED_PIN D0
///
///  Define the signal needed to turn on the LED. Must be defined.
#define LED_ON LOW 
///
///  Rate of USB to Serial chip if used on the development board.
///  This is ignored when the native USB peripheral of the 
///  ESP SoC is used.
#define SERIAL_BAUD 115200
///
///  Time in milliseconds to wait after Serial.begin() in 
///  the setup() function. If not defined, it will be set
///  to 5000 if running in the PlaformIO IDE and 2000 in the
///  Arduino IDE
///#define SERIAL_BEGIN_DELAY 10000
///
//////////////////////////////////

// This deep sleep example should work with any ESP32
//#if !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32C5)
//  #error An ESP32-C3, ESP32-C5 or ESP32-C6 based board is required
//#endif

#ifndef ESP32
  #error An ESP32 based board is required
#endif

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

#if !defined(LED_PIN) && defined(BUILTIN_LED)
#define LED_PIN BUILTIN_LED
#endif

const int ledPin = LED_PIN;
const int ledOn = LED_ON;

// Counter for restarts stored in non-volatile memory
RTC_DATA_ATTR int bootCount = 0; 


// Flash LED for count times with ms on and off times
void blink(int count=1, int ms=50) {
  for (int i=0; i<count; i++) {
    digitalWrite(ledPin, ledOn);
    delay(ms);
    digitalWrite(ledPin, 1-ledOn);
    delay(ms);
  }
}  

/* 
 esp_sleep.h from three different librairies
   ESP32     ~/.platformio/packages/framework-arduinoespressif32-libs/esp32/include/esp_hw_support/include/esp_sleep.h
   ESP32-C6  ~/.platformio/packages/framework-arduinoespressif32-libs/esp32c6/include/esp_hw_support/include/esp_sleep.h
   ESP32-C3  ~/.platformio/packages/framework-arduinoespressif32-libs/esp32c3/include/esp_hw_support/include/esp_sleep.h
 all have the same enumerated type for wake up reasons although it's obvious not all wake up causes are possible
 with the ESP32-C3 (no CO CPU, no touch pads etc.):

typedef enum {
    ESP_SLEEP_WAKEUP_UNDEFINED,    //!< In case of deep sleep, reset was not caused by exit from deep sleep
    ESP_SLEEP_WAKEUP_ALL,          //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
    ESP_SLEEP_WAKEUP_EXT0,         //!< Wakeup caused by external signal using RTC_IO
    ESP_SLEEP_WAKEUP_EXT1,         //!< Wakeup caused by external signal using RTC_CNTL
    ESP_SLEEP_WAKEUP_TIMER,        //!< Wakeup caused by timer
    ESP_SLEEP_WAKEUP_TOUCHPAD,     //!< Wakeup caused by touchpad
    ESP_SLEEP_WAKEUP_ULP,          //!< Wakeup caused by ULP program
    ESP_SLEEP_WAKEUP_GPIO,         //!< Wakeup caused by GPIO (light sleep only on ESP32, S2 and S3)
    ESP_SLEEP_WAKEUP_UART,              //!< Wakeup caused by UART0 (light sleep only)
    ESP_SLEEP_WAKEUP_UART1,             //!< Wakeup caused by UART1 (light sleep only)
    ESP_SLEEP_WAKEUP_UART2,             //!< Wakeup caused by UART2 (light sleep only)
    ESP_SLEEP_WAKEUP_WIFI,              //!< Wakeup caused by WIFI (light sleep only)
    ESP_SLEEP_WAKEUP_COCPU,             //!< Wakeup caused by COCPU int
    ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG,   //!< Wakeup caused by COCPU crash
    ESP_SLEEP_WAKEUP_BT,           //!< Wakeup caused by BT (light sleep only)
    ESP_SLEEP_WAKEUP_VAD,          //!< Wakeup caused by VAD
    ESP_SLEEP_WAKEUP_VBAT_UNDER_VOLT, //!< Wakeup caused by VDD_BAT under voltage.
} esp_sleep_source_t;
*/

void print_wakeup_reason(){
  if (bootCount <= 1) 
    Serial.println("The SoC has just been powered up or reset");
  else {
    esp_sleep_source_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
      Serial.println("Wakeup from deep sleep caused by timer");
    else
      Serial.printf("SoC wake up cause: #%d\n", wakeup_reason);  // should not be necessary in normal operation
  }    
}

//----------------------------------------------

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
  #else 
  Serial.begin(SERIAL_BAUD);
  #endif  

  delay(SERIAL_BEGIN_DELAY);

  if (!bootCount) {
    // this is printed no matter if USE_SERIAL is defined or not
    Serial.println("\n\nProject: Test waking the SoC from deep sleep with a timer");
    Serial.print(  "  Board: ");
    #ifdef ARDUINO_BOARD
    Serial.println(ARDUINO_BOARD);
    #else 
    Serial.println("Unknown ESP32 board");
    #endif
  }

  // Increment boot count
  bootCount++;

  #ifdef USE_SERIAL
  Serial.print("\nBoot number: ");
  Serial.println(bootCount);
  print_wakeup_reason();
  #endif

  // Set builtin led pin as an output
  pinMode(ledPin, OUTPUT); 

  // Display bootcount with slow LED blinks
  blink(bootCount, 750); 

  #ifdef RESTART_COUNT
  if (bootCount >= RESTART_COUNT) {
    blink(8, 35);
    Serial.println("\nThe board will be restarted in five seconds\n");
    for (int i=0; i < 5; i++) {
      blink(1, 100);
      delay(800);
    }
    ESP.restart();
  }
  #endif

  if (esp_sleep_enable_timer_wakeup(SLEEP_PERIOD) == ESP_OK) {
    delay(SETTLE_TIME);
    #ifdef USE_SERIAL
    Serial.printf("Going into sleep mode for %.1f seconds\n", SLEEP_PERIOD/(1000*1000.0));
    #endif
    // Quick LED blinks announcing start of deep sleep
    blink(5);
    esp_deep_sleep_start();
  } else {
    // delay 10 seconds to give time for manual reconnection
    delay(10000);
    Serial.println("Could not enable timer wakeup");
    Serial.println("Will reboot in 10 seconds");
    unsigned long errortimer = millis();
    while (millis() - errortimer < 10000) {
      blink(1);
      delay(100);
    }  
    ESP.restart();
  }  

  Serial.println("This message in setup() should never be printed");
  while (true) {
    blink(5);
    delay(250);
  }  

}

void loop() {
  Serial.println("This is the loop talking. Should never happen.");
  // This odd flashing pattern should never be seen because the 
  // device is put in deep sleep in setup() before loop() starts.
  blink(4, 50);
  delay(125);
  blink(2, 100);
  delay(5000);
}
