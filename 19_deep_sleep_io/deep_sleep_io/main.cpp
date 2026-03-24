/*
 *  See deep_sleep_io.ino for license and attribution.
 */

/**************************************************************
OPERATION
  Each time the board wakes up from deep sleep, it executes the
  setup() function. Near the end of the setup() function, the
  board is put into deep sleep. Consequently the loop() function
  is never executed and anything in the setup() function after 
  when deep sleep is enabled will also not be executed.

  The board is woken from deep sleep mode by setting an RTC IO 
  pin either HIGH or LOW. A prompt on the serial monitor will 
  give instructions as to which pin to activate and the needed
  signal to wake the board.

 
NOTE
  The sketch can report the state of the machine to the 
  serial monitor. However, the serial peripheral is shut
  down when the SoC goes into sleep mode. That means 
  that the serial port has to be reopened when comming
  out of sleep mode. The Arduino IDE automatically 
  reconnects to the serial port quickly enough that 
  the messages from the sketch will be seen. In 
  PlatformIO it is necessary to manually reconnect 
  to the SoC built-in USB of the SoC so that it may 
  not be possible to see the messages.


NOTE
  #if SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP
  *
  * @brief Enable wakeup using specific gpio pins
  *
  * This function enables an IO pin to wake up the chip from deep sleep.
  *
  * @note You don't need to worry about pull-up or pull-down resistors before
  *       using this function because the ESP_SLEEP_GPIO_ENABLE_INTERNAL_RESISTORS
  *       option is enabled by default. It will automatically set pull-up or pull-down
  *       resistors internally in esp_deep_sleep_start based on the wakeup mode. However,
  *       when using external pull-up or pull-down resistors, please be sure to disable
  *       the ESP_SLEEP_GPIO_ENABLE_INTERNAL_RESISTORS option, as the combination of internal
  *       and external resistors may cause interference. BTW, when you use low level to wake up the
  *       chip, we strongly recommend you to add external resistors (pull-up).
  *
  esp_err_t esp_deep_sleep_enable_gpio_wakeup(uint64_t gpio_pin_mask, esp_deepsleep_gpio_wake_up_mode_t mode);
  #endif 

*************************************************************/

#include <Arduino.h>

//////// User configuration //////
///
///  Settle time in milliseconds after calling esp_deep_sleep_enable_gpio_wakeup(). 
#define SETTLE_TIME 500 //250
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

#if !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32C5)
  #error An ESP32-C3, ESP32-C5 or ESP32-C6 based board is required
#endif

#if !defined(SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP)
  #error Deep sleep wake up with GPIO event not supported
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

//---- RTC ram data ------------------------------
/*
 * Three integer variables that must be stored in the reat time
 * clock ram which does not lose its content during deep sleep 
 * and during the wake up process from deep sleep
 * 
 * from esp32/tools/esp32c5-libs/3.3.6-RC1/include/esp_hw_support/include/esp_sleep.h
 *
 * #if SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP
 * typedef enum {
 *     ESP_GPIO_WAKEUP_GPIO_LOW = 0,
 *     ESP_GPIO_WAKEUP_GPIO_HIGH = 1
 * } esp_deepsleep_gpio_wake_up_mode_t;
 * #endif
 */

// Wake mode, start with ESP_GPIO_WAKEUP_GPIO_HIGH = 1
RTC_DATA_ATTR int wakemode = 0;     

// Counter for restarts stored in non-volatile memory
RTC_DATA_ATTR int bootCount = 0;

// Current index of the wake pin
RTC_DATA_ATTR int currentindex = -1; //initially, no gpio pin is active

//---- WAKE PIN DEFINITIONS --------------------------------------
//
// padcount = number of pin/pads that can be used to wake the SoC
// padlables[] array with board labels of the wake pins
// iopins[] corresponding gpio pin number of wake pins
//
//
#if defined(ARDUINO_NOLOGO_ESP32C3_SUPER_MINI)
// only the pins (GPIO0 ~ GPIO5) in VDD3P3_RTC domain can be used to wake up the chip from Deep-sleep mode.
// ESP32-C3 Technical Reference Manual Verson 1.3 2025-05-08 section 5.10.1 Power Supplies of GPIO Pins, p167
const int padcount = 6;
const char *padlabels[padcount] = {"0", "1", "2", "3", "4", "5"};
const int iopins[padcount] = {0, 1, 2, 3, 4, 5};
#endif
//
//
#if defined(ARDUINO_XIAO_ESP32C3)
const int padcount = 4;
const char *padlabels[padcount] = {"D0", "D1", "D2", "D3"};
const int iopins[padcount] = {D0, D1, D2, D3};
#endif
//
//
#if defined(ARDUINO_XIAO_ESP32C6)
const int padcount = 7;
const char *padlabels[padcount] = {"LP_GPIO0 [D0]",  
                                   "LP_GPIO1 [D1]",
                                   "LP_GPIO2 [D2])",  
                                   "LP_GPIO4 [MTMS]",
                                   "LP_GPIO5 [MTDI]",    
                                   "LP_GPIO6 [MTCK]", 
                                   "LP_GPIO7 [MTDO]"};
const int iopins[padcount] = {A0, A1, A2, 4, 5, 6, 7};
#endif


void nextindex(void) {
  currentindex++;
  if (currentindex >= padcount) 
    currentindex = 0;
 
  if (!currentindex) {
    wakemode = 1 - wakemode;
    Serial.printf("\n*** For this round, wake the SoC from sleep mode by setting the wake pin %s ***\n", (wakemode) ? "HIGH" : "LOW");
  }  

  int wkpin = iopins[currentindex];
  Serial.printf("\nWake the SoC by setting %s (%d) %s.\n", padlabels[currentindex], iopins[currentindex], (wakemode) ? "HIGH" : "LOW");
  
  /* 
  This actually works
    pinMode(wkpin, OUTPUT);
    digitalWrite(wkpin, (wakemode) ? LOW : HIGH);

  but this will not work
    pinMode(wkpin, OUTPUT);
    digitalWrite(wkpin, (wakemode) ? HIGH : LOW);
  because wakeup occurs as soon as deep sleep is started.
  
  Both these work
    pinMode(wkpin, (wakemode) ? INPUT_PULLDOWN : INPUT_PULLUP);
    pinMode(wkpin, (wakemode) ? INPUT_PULLUP : INPUT_PULLDOWN);
    pinMode(wkpin, INPUT);  - see note above
  and according to some examples @ https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#deep-sleep-mode-and-wake-up
  not specifying a pinMode works.
  */

  pinMode(wkpin, INPUT); // just making sure
  esp_deep_sleep_enable_gpio_wakeup(BIT(wkpin), (esp_deepsleep_gpio_wake_up_mode_t)wakemode); // ESP_GPIO_WAKEUP_GPIO_HIGH) or _LOW;
  delay(SETTLE_TIME);
  Serial.println("Going to sleep now");
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
    if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO)
      Serial.println("Wakeup from deep sleep caused by GPIO event");
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
    Serial.println("\n\nProject: Test waking the SoC from deep sleep with a I/O pin");
    Serial.print(  "  Board: ");
    #ifdef ARDUINO_BOARD
    Serial.println(ARDUINO_BOARD);
    #else 
    Serial.println("Unknown ESP32 board");
    #endif
    Serial.println("Instructions:");
    Serial.println("  Set each I/O pin either HIGH or LOW as instructed to wake");
    Serial.println("  the board from deep sleep.");
  }

  // Increment boot count
  bootCount++;

  Serial.print("\nBoot number: ");
  Serial.println(bootCount);
  print_wakeup_reason();
  
  nextindex();
  esp_deep_sleep_start();
  delay(50);
  Serial.println("This will never be printed");
}

void loop() {
  Serial.println("This is the loop talking. Should never happen.");
}
