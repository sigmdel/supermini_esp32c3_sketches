/*
 *  See dac.ino for license and attribution.
 */

#include <Arduino.h>
#include <driver/dac_oneshot.h>
#include <driver/dac_cosine.h>
//#include <driver/dac_continuous.h>


/// Configuration //////
///
#define FREQ 150
///
///#define WITH_TRIANGLE
///
//////////////////////////

#define DAC1 25 // DAC 1 on gpio 25
#define DAC2 26 // DAC 2 on gpio 26

dac_cosine_handle_t dac_chan_handle;  

void setup() {
  Serial.begin(115200);  
  delay(2000);
 
  // Enable DAC_0 cosine wave generator
  dac_cosine_config_t cw_config = {
    .chan_id = DAC_CHAN_0,        // DAC channel 0 is GPIO25(ESP32) / GPIO17(ESP32S2) 
    .freq_hz = FREQ,               // Hz the minimum frequency of cosine wave is about 130 Hz,
    .clk_src = DAC_COSINE_CLK_SRC_DEFAULT,
    .atten = DAC_COSINE_ATTEN_DEFAULT,
    .phase = DAC_COSINE_PHASE_0,    // 0 degree phase
    .offset = 0,                 // No DC offset
    .flags = {.force_set_freq = true }  // default
  };

   // Configure the DAC-CW channel
  dac_cosine_new_channel(&cw_config, &dac_chan_handle );
  // Start te DAC-CW Generator on DAC channel
  dac_cosine_start(dac_chan_handle);

  Serial.printf("Hardware %d Hz cosine wave output signal on GPIO 25\n", FREQ);  

  #ifdef WITH_TRIANGLE
  Serial.println("Software triangle wave output signal on GPIO 26");
  #endif

  Serial.println("setup() completed");
}

void destroy_dac_chan(void) {
  if ( dac_chan_handle != NULL ) {
    dac_cosine_stop( dac_chan_handle );
    dac_cosine_del_channel( dac_chan_handle );
  }  
}
  
void loop() {
  #ifdef WITH_TRIANGLE
  // Sweep DAC2 from 0V to 3.3V
  for (int i = 0; i <= 255; i++) {
    dacWrite(DAC2, i);
    delay(20);
  }
  // Sweep back down
  for (int i = 255; i >= 0; i--) {
    dacWrite(DAC2, i);
    delay(20);
  }
  #endif
}
