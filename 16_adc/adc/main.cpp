/*
 *  See adc.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Defining this macro to include A5 (gpio 5) in the analogue inputs will result in
///  error messages each time an attempt is made to use analogRead(A5).
///#define TEST_A5
///
///  Define this macro to use a single analogue pin instead of probing all valid
///  analogue inputs in round-robbin fashion. The TEST_A5 macro overrides the
///  ADC_PIN macro.
///#define ADC_PIN 3
///
///  Define this macro to displays the raw (integer) ADC data obtained by analogRead(). 
///  If the macro define then raw data is internally processed and obtained using 
///  analogReadMilliVolts(). The millivolts are then converted to volts for display.
#define RAW 
///
/// Define this macro if the data is to be plotted in the IDE. The effect is different 
/// depending on the IDE.
/// Arduino IDE - All serial output except for the numeric data with the variable names 
///   is suppressed to avoid clutting up the list of variables in the Serial plotter.
/// pioarduino IDE - the numeric data is printed in the teleplot format (">name:%d\n" 
/// or ">name:%f\n") while the rest of the messages is send out to the serial port 
/// and will be displayed in the where it will be shown 
#define PLOTTER 
///
///  Define to also send min (0) and max (3.3v or 4095) values as plot data 
///#define MIN_MAX
///
///  Define delay in milliseconds at end of loop which amount to the delay 
///  between analogue reads. If plotter is not defined or if using 
///  the Arduino IDE then 100 is a good value, 5 would work with the 
///  LasecPlot add on in pioarduino
#define READ_DELAY 10
///
/// Time in milliseconds during which the current io pin is active
#define ACTIVE_PERIOD 20000
///
/// Time of inactivity in milliseconds between pins
#define INACTIVE_PERIOD 5000
///
///  Rate of USB to Serial chip if used on the development board.
///  This is ignored when the native USB peripheral of the 
///  ESP SoC is used.
#define SERIAL_BAUD 115200
///
///  Time in milliseconds to wait after Serial.begin() in 
///  the setup() function. If not defined, it will be set
///  to 5000 if running in the PlaformIO IDE to manually switch
///  to the serial monitor otherwise to 2000 if an native USB 
///  peripheral is used or 1000 if a USB-serial adpater is used.
///#define SERIAL_BEGIN_DELAY 8000
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

// timing constants all in milliseconds

const int activeperiod  = ACTIVE_PERIOD;   // time during which the current io pin is read
const int blankperiod   = INACTIVE_PERIOD; // time during which no io pin is active

unsigned long activetimer = 0;  // used to time active period

// Checking Super Mini ESP32-C5 dev board

const int padcount = 5;
const char *padlabels[padcount+1] = {"A0", "A1", "A2", "A3", "A4", "A5"};   
const int iopins[padcount+1] = {A0, A1, A2, A3, A4, A5};   

int currentindex = -1;

void nextIndex(void) {
  if (currentindex >= 0) {
    delay(blankperiod);
  }
  currentindex++;
  if (currentindex >= padcount) 
    currentindex = 0;
  #if defined(PLATFORMIO) || !defined(PLOTTER)
  Serial.printf("\nConnect board pad labeled %d (%s) to an analogue signal.\n", iopins[currentindex], padlabels[currentindex]);
  #endif
}  


//----------------------------------------------

void setup() {
  #if (ARDUINO_USB_CDC_ON_BOOT > 0)
  Serial.begin();
  #else 
  Serial.begin(SERIAL_BAUD);
  #endif  

  #if !defined(SERIAL_BEGIN_DELAY)
    #if defined(PLATFORMIO)
      #define SERIAL_BEGIN_DELAY 5000    // 5 seconds
    #else
      #define SERIAL_BEGIN_DELAY 2000    // 2 seconds
    #endif
  #endif 
  delay(SERIAL_BEGIN_DELAY);

  #if defined(PLATFORMIO) || !defined(PLOTTER)

  Serial.println("\n\nProject: adc");
  Serial.println("Purpose: Check analogue input capabilities of some GPIO pins of the Super Mini");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32 board");
  #endif


  #ifdef TEST_A5
  #ifdef ADC_PIN
  #undef ADC_PIN
  #endif
  #define ADC_PIN A5
  #endif

  #ifdef ADC_PIN
    currentindex = ADC_PIN;
    #ifdef TEST_A5
      Serial.println("\nWill attempt to use GPIO 5 (A5) as an analogue input pin");
    #else
      if ((currentindex < 0) or (currentindex > 5)) {
        Serial.printf("\n***Error*** A%d is an invalid analogue pin\n", currentindex);
        Serial.println("Restarting in 10 seconds");
        delay(10000);
        ESP.restart();
      }  
      Serial.printf("\nConnect an analogue signal source to GPIO pin %d (A%d).\n", currentindex, currentindex);
      currentindex--;
    #endif  
  #else
    Serial.println("\nConnect an analogue signal source to each GPIO pin as prompted on the serial monitor.");
  #endif

  Serial.println("Be careful, the signal should be in the range of 0 to 3,3 votls.");
  #ifndef TEST_A5
  Serial.println("The measured voltage on the analogue pin (or its raw integer value if the");
  Serial.println("RAW macro is defined) will be displayed in the serial monitor or it can be");
  Serial.println("viewed in the serial ploter.\n");
  #endif

  #ifdef RAW
  Serial.println("The raw input ADC data will be displayed");
  #else
  Serial.println("The input data converted to volts will be displayed");
  #endif

  /* --- CPU runs at highest frequency by default ---
  
  uint32_t freq = getCpuFrequencyMhz();
  Serial.printf("CPU frequency by default: %d Mhz\n", freq);
  if (freq != 240) {
    if (setCpuFrequencyMhz(240)) 
      Serial.println("CPU frequency set to 240 Mhz");
    else
      Serial.println("Unable to set the CPU frequency to 240 Mhz");
  }      

  CPU frequency by default: 160 Mhz
  [  5013][E][esp32-hal-cpu.c:270] setCpuFrequencyMhz(): CPU clock could not be set to 240 MHz. Supported frequencies: 160, 120, 80, 40, 20 MHz
  Unable to set the CPU frequency to 240 Mhz
  */

  #endif
  
  delay(3000); // time to read above message

  // Set the resolution to 12 bits (0-4095)
  analogReadResolution(12);  // this is the default anyway
  
  #ifndef TEST_A5
  // start process
  nextIndex();
  #endif

  activetimer = millis();
}


bool with_prefix = 
#if defined(PLOTTER) && defined(PLATFORMIO) 
true
#else 
false 
#endif
;

void loop() {
  #ifndef ADC_PIN
  if (millis() - activetimer > activeperiod) {
    nextIndex();
    activetimer = millis();
  }   
  #endif
  
  /* This is an elegant way to end up with a single 
     Serial.println() statement per analogue reading
     but the actual implementation below may be faster

  String inValue;

  #ifdef RAW
  inValue = ">Raw:";
  inValue += analogRead(iopins[currentindex]);
  #else
  inValue = ">Volts:";
  inValue += analogReadMilliVolts(iopins[currentindex])/1000.0;
  #endif

  #ifdef MIN_MAX
  #ifdef RAW
    inValue += "\n>max:4095";
  #else
    inValue += "\n>max:3.3";
  #endif
  inValue += "\n>min:0";
  #endif

  #if !defined(PLOTTER) || !defined(PLATFORMIO)
  inValue.replace("\n", "\t");  // not using teleplot or fork
  inValue.replace(">", "");
  #endif

  Serial.println(inValue.c_str());

  delay(READ_DELAY);      
  */

  #ifdef RAW
  uint16_t inValue = analogRead(iopins[currentindex]);
  if (with_prefix) {
    Serial.print(">Raw:");
    Serial.println(inValue);
    #ifdef MIN_MAX
    Serial.println(">max:4095\n>min:0");
    #endif
  } else {
    Serial.print("Raw:");
    Serial.print(inValue);
    #ifdef MIN_MAX
    Serial.print("\tmax:4095\tmin:0");
    #endif
    Serial.println();
  }  
	#else
  uint32_t inValue = analogReadMilliVolts(iopins[currentindex]);
  if (with_prefix) {
    Serial.print(">Volts:");
    Serial.println(inValue/1000.0);
    #ifdef MIN_MAX
    Serial.println(">max:3.3\n>min:0");
    #endif
  } else {
    Serial.print("Volts:");
    Serial.print(inValue/1000.0);
    #ifdef MIN_MAX
    Serial.print("\tmax:3.3\tmin:0");
    #endif
    Serial.println();
  }  
  #endif  

  #ifdef READ_DELAY
  delay(READ_DELAY); 
  #endif     
}
