/*
 *  Generating a cosine signal with the DAC Cosine Wave Generator with an

  ESP32 based development card

  Ref: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html

  Most ESP32 Arduino examples found so far use the deprecated <driver/dac.h> DAC driver.
  if #include <driver/dac.h> is in the source code the following warning is emitted
    #warning "The legacy DAC driver is deprecated, please use `driver/dac_oneshot.h`, `driver/dac_cosine.h` or `driver/dac_continuous.h` instead"

  I could not find an example based on the <dac_cosine.h> code> except for an espressif arduino-esp32 issue 
  Frequency incorrect when using new style DAC dac_cosine libraries, also glitches on channel 1 #10192
  @ https://github.com/espressif/arduino-esp32/issues/10192 by jay-esp in August 2024. 
  The following is borrowed directly for that source.
* Michel Deslierres
 * March 17, 2026
 *
 * Copyright 2026, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html>*/
// SPDX-License-Identifier: 0BSD
