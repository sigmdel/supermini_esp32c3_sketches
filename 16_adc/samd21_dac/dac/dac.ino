// Resolution
//
#define DAC_RESOLUTION  10  
   // 2^10 = 1024  (hence possible output value 0 to 1023)

#define DAC_PIN A0  // DAC output pin

int step = 16;

void setup() {
  Serial.begin(115200);  
  delay(2000);
  //pinMode(DAC_PIN, OUTPUT);     // ***** DO NOT set A0 mode to OUTPUT when using the DAC *****
  analogWriteResolution(DAC_RESOLUTION);
  Serial.println("setup() completed");
}

int outvalue = 0;

void loop() {
  // put your main code here, to run repeatedly:
    float volts = (outvalue / 1023.0)* 3.3;
    Serial.printf("DAC value: %d, volts=%.3f\n", outvalue, volts);
    analogWrite(DAC_PIN, outvalue);
    outvalue += step;
    if ((outvalue < 0) || (outvalue > 1023)) {
      step = -step;
      if (outvalue < 0)
        outvalue = 0;
      else
        outvalue = 1023;
    }
    delay(20);      
}
