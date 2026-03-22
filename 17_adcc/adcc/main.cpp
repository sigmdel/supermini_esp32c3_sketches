/*
 *  See adcc.ino for license and attribution.
 */

#include <Arduino.h>

//////// User configuration //////
///
///  Define the analogue input pin, must be one of A0, A1, A2, A3, A4 or 0, 1, 2, 3, 4.
#define ADC_INPUT_PIN 0
///
///  Define how many readings (analog to digital conversions) of the value of the INPUT_PIN 
///  will happen. Reading the data will an and reading the data will be and average of all conversions
#define CONVERSIONS 3
///
///  Define the sampling frequency in herz. Supported sampling frequency from 611 to 83333 Hz.
///  Note: setting CONVERSIONS < 3 with SAMPLE_FREQ = 78000 will result in a lower sampling 
///  frequency
#define SAMPLE_FREQ 78000
///
///  Define to optionally stop ADC continuous input while printing readings
///  This is defined by default in the original example
///#define SUSPEND_WHEN_PRINTING 
///
///  Define this macro to displays the raw (integer) ADC data obtained by analogRead(). 
///  If the macro define then raw data is internally processed and obtained using 
///  analogReadMilliVolts(). The millivolts are then converted to volts for display.
///#define RAW 
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

#ifndef ESP32
  #error An ESP32 based board is required
#endif  

#ifndef CONFIG_IDF_TARGET_ESP32C3
  #error An ESP32-C3 based board is required
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

uint8_t adc_pin = ADC_INPUT_PIN;

// Result structure for ADC Continuous reading
adc_continuous_result_t *result = NULL;

// Flag which will be set in ISR when conversion is done
volatile bool adc_coversion_done = false;

// ISR Function that will be triggered when ADC conversion is done
void ARDUINO_ISR_ATTR adcComplete() {
  adc_coversion_done = true;
}

void errReboot(void) {
  pinMode(OUTPUT, LED_BUILTIN);
  for (int i = 0; i < 15; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(75);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(925);
  }
  delay(5000);
  ESP.restart();
}


const int cap = 25000;
int datandx = 0;
int data[cap] = {0};
unsigned long starttick;
unsigned long endtick;

#if defined(PLOTTER) && defined(PLATFORMIO) 
bool with_prefix = true;
#else 
bool with_prefix = false;
#endif

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

  Serial.println("\n\nProject: adcc");
  Serial.println("Purpose: Check continuous analogue input capabilities of the ESP32-C3 Super Mini");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32 board");
  #endif

  if (adc_pin > 4) {
    Serial.printf("\n\n**Error** GPIO pin #%d is not a valid analogue input pin\n", adc_pin);
    errReboot();
  }
  
  Serial.printf("\nConnect an analogue signal source to io pin %d.\n", adc_pin);
  Serial.println("Be careful, the signal should be in the range of 0 to 3,3 votls.");
  Serial.println("The measured voltage on the analogue pin (or its raw integer value if the");
  Serial.println("RAW macro is defined) will be displayed in the serial monitor or it can be");
  Serial.println("viewed in the serial ploter.\n");

  Serial.printf("\nSupported sampling frequencies range: %d Hz to %d Hz\n", SOC_ADC_SAMPLE_FREQ_THRES_LOW, SOC_ADC_SAMPLE_FREQ_THRES_HIGH);
  
  if ((SAMPLE_FREQ < SOC_ADC_SAMPLE_FREQ_THRES_LOW) || (SAMPLE_FREQ > SOC_ADC_SAMPLE_FREQ_THRES_HIGH)) {
    Serial.printf("\n\n**Error** Sampling frequency %d Hz is out of supported range", SAMPLE_FREQ);
    errReboot();
  }

  if ((CONVERSIONS < 1) || (CONVERSIONS*SOC_ADC_DIGI_RESULT_BYTES > 4092))  {
    Serial.print("\n\n**Error** ");
    if (CONVERSIONS < 1) 
      Serial.println("Conversion must be greater than 0");
    else 
      Serial.printf("Buffer overrun. Conversion value %d is out of range which is 1 to %d when 1 pin is being sampled\n", 4092/SOC_ADC_DIGI_RESULT_BYTES);
    errReboot();
  }  

  #ifdef RAW
  Serial.println("The raw input ADC data will be read");
  #else
  Serial.println("The data converted to millivolts will be read");
  #endif

  #endif  // PLATFORMIO or not PLOTTER

  // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
  analogContinuousSetWidth(12);

  // Optional: Set different attenaution (default is ADC_11db)
  analogContinuousSetAtten(ADC_11db);

  // Setup ADC Continuous with following input:
  // array of pins, count of the pins, how many conversions per pin in one cycle will happen, sampling frequency, callback function
  analogContinuous(&adc_pin, 1, CONVERSIONS, SAMPLE_FREQ, &adcComplete);

  // Start ADC Continuous conversions
  analogContinuousStart();

  datandx = 0;
  starttick = millis();
  while (datandx < cap) {
    // Check if conversion is done and try to read data
    if (adc_coversion_done == true) {
      // Set ISR flag back to false
      adc_coversion_done = false;

      // Read data from ADC
      if (analogContinuousRead(&result, 0)) {
  
        #ifdef SUSPEND_WHEN_READING
        // Optional: Stop ADC Continuous conversions to have more time to process (print) the data
        analogContinuousStop();
        #endif

        #ifdef RAW
        data[datandx] = result[0].avg_read_mvolts;
        #else
        data[datandx] = result[0].avg_read_mvolts;
        #endif
        datandx++;
             
        #ifdef SUSPEND_WHEN_READING
        // Optional: If ADC was stopped, start ADC conversions and wait for callback function to set adc_coversion_done flag to true
        analogContinuousStart();
        #endif

      } else {
        Serial.println("Error occurred during reading data. Set Core Debug Level to error or lower for more information.");
      } // if read
    } // if conversion true
    yield(); 
  } // while
  endtick = millis();
  
  Serial.printf("%d readings in %lu milliseconds,", cap*CONVERSIONS, endtick - starttick);
  Serial.printf("equates to a sampling frequency of %d Hz.\n", (1000*cap*CONVERSIONS)/(endtick-starttick));
  delay(12000);
  datandx = 0;
} 

void loop() {
  Serial.print(">Volts:");
  Serial.println(data[datandx]/1000.0);
  datandx++;
  if (datandx >= cap) {
    datandx = 0;
    delay(1000);
  }  
}
