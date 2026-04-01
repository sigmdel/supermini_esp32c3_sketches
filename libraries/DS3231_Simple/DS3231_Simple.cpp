#include <DS3231_Simple.h>

#define INPUT_DELAY 250



void DS3231_Simple::begin()
{  
  Wire.begin();
   
  // Setup the clock to make sure that it is running, that the oscillator and 
  // square wave are disabled, and that alarm interrupts are disabled
  rtc_i2c_write_byte(0xE, 0b00000000);
  disableAlarms();
}

// Page 11 of Datasheet shows that the LSB 4 bits of the data types are always the last BCD digit
//  the upper 3 bits are sometimes the upper digit, sometimes a mix of that and some other data
//  so be sure to mask your input for bcd2bin
uint8_t DS3231_Simple::bcd2bin (uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }
uint8_t DS3231_Simple::bin2bcd (uint8_t val) { return (val / 10) << 4 | (val % 10);     }

void DS3231_Simple::print_zero_padded(Stream &Printer, uint8_t x)
{
  if(x < 10) Printer.print('0');
  Printer.print(x);
}


uint8_t DS3231_Simple::rtc_i2c_seek(const uint8_t Address)
{
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(Address);
  return Wire.endTransmission();
}

uint8_t DS3231_Simple::rtc_i2c_write_byte(const uint8_t Address, const uint8_t Data)
{
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(Address);
  Wire.write(Data);
  return Wire.endTransmission();
}

uint8_t DS3231_Simple::rtc_i2c_read_byte(const uint8_t Address, uint8_t &Data)
{
  rtc_i2c_seek(Address);
  #if 0
  // hardware/arduino/avr/libraries/Wire/utilities/twi.cpp::twi_readFrom()
  // returns 0 ONLy if the buffer is too small for the number of bytes we
  // want.  Since we are reading 1 byte, that's never going to happen
  // so there is no point in this comparison.  
  //
  // Short Version: Wire.requestFrom(x, n) can only ever return n
  //
  // Leaving this here in case twi.cpp changes in the future to return 
  // 0 on other cases.
  
  if(Wire.requestFrom(RTC_ADDRESS,(uint8_t) 1) == 1)
  {
    Data = Wire.read();
    return 1;
  }
  return 0;
  #else
  Wire.requestFrom(RTC_ADDRESS,(uint8_t) 1);
  Data = Wire.read();
  return 1;
  #endif  
}

// Compare the datetime of two objects to put i ascending date order 
//  -1 A is befre B, 1 B is before A, 0 identical
int8_t DS3231_Simple::compareTimestamps(const DateTime &A, const DateTime &B)
{
  if(A.Year < B.Year)       return -1;
  if(A.Year > B.Year)       return  1;

  if(A.Month < B.Month)     return -1;
  if(A.Month > B.Month)     return 1;

  if(A.Day < B.Day)         return -1;
  if(A.Day > B.Day)         return 1;

  if(A.Hour < B.Hour)       return -1;
  if(A.Hour > B.Hour)       return 1;

  if(A.Minute < B.Minute)   return -1;
  if(A.Minute > B.Minute)   return 1;

  if(A.Second < B.Second)   return -1;
  if(A.Second > B.Second)   return 1;

  return 0;
}

uint8_t DS3231_Simple::formatEEPROM()
{
  eepromWriteAddress = 0;
  writeBytePagewizeStart();
  for(uint16_t x = 0; x < EEPROM_BYTES; x++)
  {
    writeBytePagewize(0);
  }
  writeBytePagewizeEnd();
  
  eepromWriteAddress = 0;
  eepromReadAddress  = 0; 
  return 1;
}

uint8_t DS3231_Simple::readEEPROMByte(const uint16_t address)
{
  uint8_t b = 0;
  Wire.beginTransmission(EEPROM_ADDRESS); // DUMMY WRITE
  Wire.write((uint8_t) ((address>>8) & 0xFF)); 
  Wire.write((uint8_t) ((address) & 0xFF)); 
  
  if(Wire.endTransmission(false)) // Do not send STOP, just restart
  {
    return 0;
  }
  
  if(Wire.requestFrom(EEPROM_ADDRESS, (uint8_t) 1))
  {
    b = Wire.read();
  }
  
  Wire.endTransmission(); // Now send STOP
  
  return b;
}

// Locate the NEXT place to store a block
uint16_t DS3231_Simple::findEEPROMWriteAddress()
{   
  uint8_t t = 0;
  
  for(eepromWriteAddress = 0; eepromWriteAddress < EEPROM_BYTES; )
  {
    t = readEEPROMByte(eepromWriteAddress);

    // If the byte read is a zero, then this is the top of the stack.
    if(t == 0) break;

    // If not zero, then this must be a start byte for the block (we will assert that
    // blocks are always aligned to byte zero of the EEPROM, there is no "wrapping" 
    // of a block starting at the top of the EEPROM and finishing in the bottom
    // so we will add the length of this block to x
    
    // The upper 3 bits store the number of data bytes
    // plus 5 header bytes
    eepromWriteAddress = eepromWriteAddress + (t >> 5) + 5;      
  }

  // If we have filled up as much as we can... reset back to the bottom as the stack top.
  if(eepromWriteAddress >= EEPROM_BYTES-5) 
  {
    eepromWriteAddress = 0;
  }
  
  return eepromWriteAddress;
}

// Locate the NEXT block to read from
uint16_t DS3231_Simple::findEEPROMReadAddress()
{   
  // This is going to be really memory hungry :-/
  // Anybody care to think of a better way.
  uint16_t nxtPtr, x      = 0;
  
  DateTime currentOldest;
  DateTime compareWith;
  currentOldest.Year = 255; // An invalid year the highest we can go so that any valid log is older.
  
  // Find the oldest block, that is the bottom
  for(x = 0; x < EEPROM_BYTES; )
  {
    if(readEEPROMByte(x) == 0) { x++; continue; }
        
    // readLogFrom will return the address of the next log entry if any
    // or EEPROM_BYTES if not.
    
    nxtPtr = readLogFrom(x, compareWith, 0, 0);    
    if(compareTimestamps(currentOldest,compareWith) > 0)
    {
      currentOldest        = compareWith;
      eepromReadAddress    = x;
    }
    
    if(nxtPtr > x) 
    {
      x = nxtPtr;
    }
    else
    {
      break; // no more entries
    }
  }

  return eepromReadAddress;
}

// Clear some space int he EEPROM to record BytesRequired bytes, nulls
//  any overlappig blocks.
uint8_t DS3231_Simple::makeEEPROMSpace(uint16_t Address, int8_t BytesRequired)
{
  if((Address+BytesRequired) >= EEPROM_BYTES) 
  {
    return 0;  // No can do.   
  }
  uint8_t x;
  while(BytesRequired > 0)
  {
    x = readEEPROMByte(Address);
    if(x == 0) // Already blank
    {
      BytesRequired--;
      Address++;
      continue;
    }
    else
    {  
      uint16_t oldEepromWriteAddress = eepromWriteAddress;
      eepromWriteAddress = Address;
      
      writeBytePagewizeStart();
      for(x = ((x>>5) + 5); x > 0; x-- )
      {
        writeBytePagewize(0);
      }
      writeBytePagewizeEnd();
      eepromWriteAddress = oldEepromWriteAddress;                
    }
  }
  
  return 1;
}

uint8_t DS3231_Simple::writeBytePagewizeStart()
{
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((eepromWriteAddress >> 8) & 0xFF);
  Wire.write(eepromWriteAddress & 0xFF);
  return 1;
}

uint8_t DS3231_Simple::writeBytePagewize(const uint8_t data)
{
  Wire.write(data);
    
  // Because of the 32 byte buffer limitation in Wire, we are 
  //  using 4 bits as the page size for a page of 16 bytes
  //  even though the actual page size is probably higher
  //  (it needs to be a binary multiple for this to work).  
  eepromWriteAddress++;
  
  if(eepromWriteAddress < EEPROM_BYTES && ((eepromWriteAddress >>4) & 0xFF) != (((eepromWriteAddress-1)>>4) & 0xFF))
  {
    // This is a new page, finish the previous write and start a new one
    writeBytePagewizeEnd();
    writeBytePagewizeStart();
  }

  return 1;
}

uint8_t DS3231_Simple::writeBytePagewizeEnd()
{
  if(Wire.endTransmission() > 0)
  {
    // Failure
    return 0;
  }
  
  // Poll for write to complete
  while(!Wire.requestFrom(EEPROM_ADDRESS,(uint8_t) 1));
  return 1;
}

uint8_t  DS3231_Simple::writeLog( const DateTime &timestamp,   const uint8_t *data, uint8_t size )
{
  if(size > 7) return 0; // Limit is 7 data bytes.
  
  if(eepromWriteAddress >= EEPROM_BYTES) findEEPROMWriteAddress();            // Uninitialized stack top, find it.
  if((eepromWriteAddress + 5 + size) >= EEPROM_BYTES) eepromWriteAddress = 0; // Would overflow so wrap to start
  
  if(!makeEEPROMSpace(eepromWriteAddress, 5+size))   
  {
    return 0;
  }
  
  writeBytePagewizeStart();  
  writeBytePagewize((size<<5) | (timestamp.Dow<<2) | (timestamp.Year >> 6));     
  writeBytePagewize((timestamp.Year<<2)  | (timestamp.Month >> 2));
  writeBytePagewize((timestamp.Month<<6) | (timestamp.Day << 1) | (timestamp.Hour >>4));
  writeBytePagewize((timestamp.Hour<<4)  | (timestamp.Minute>>2));
  writeBytePagewize(((timestamp.Minute<<6)| (timestamp.Second)) & 0xFF);  
    
  for(; size > 0; size--)
  {
    writeBytePagewize(*data);
    data++;
  }    
  writeBytePagewizeEnd();
  
  // We must also clear any existing block in the next write address
  //  this ensures that if the reader catches up to us that it will only
  //  read a blank block
  makeEEPROMSpace(eepromWriteAddress, 5);
  
  return 1;  
}

uint16_t DS3231_Simple::readLogFrom( uint16_t Address, DateTime &timestamp,   uint8_t *data, uint8_t size )
{
  uint8_t b1, b2, datalength;
     
  b1 = readEEPROMByte(Address++);
  b2 = readEEPROMByte(Address++);

  if(!b1) return EEPROM_BYTES+1;

  datalength = (b1 >> 5);
    
  // <Timestamp> ::= 0Bzzzwwwyy yyyyyymm mmdddddh hhhhiiii iissssss 
  timestamp.Dow   =  (b1 >> 2) | 0x03;
  timestamp.Year  =  (b1 << 6) | (b2>>2);// & 0b11111111

  b1 = readEEPROMByte(Address++);
  timestamp.Month =  ((b2 << 2) | (b1 >> 6)) & 0b00001111; 
  timestamp.Day   =  (b1 >> 1) & 0b00011111;
  
  b2 = readEEPROMByte(Address++);
  timestamp.Hour =  ((b1 << 4) | (b2 >> 4)) & 0b00011111;

  b1 = readEEPROMByte(Address++);
  timestamp.Minute = ((b2 << 2) | (b1 >> 6)) & 0b00111111;
  timestamp.Second = b1 & 0b00111111;

  while(datalength--)
  {
    // If our supplied buffer has room, copy the data byte into it
    if(size) 
    {
      size--;
      *data = readEEPROMByte(Address);
      data++;
    }
           
    Address++;
  }

  // If we have caught up with the writer, return that as the next read
  if(Address == eepromWriteAddress) return Address;
  
  while(Address < EEPROM_BYTES && (readEEPROMByte(Address) == 0))
  {
    Address++;
  }

  if(Address == EEPROM_BYTES && eepromWriteAddress < Address)
  {
    // There was nothing ahead of us, and the writer is behind us
    //  which means this is all empty unusable space we just walked
    //  so go to zero position
    Address = 0;
  }
  
  return Address;
}

uint8_t DS3231_Simple::readLog( DateTime &timestamp,   uint8_t *data, uint8_t size )
{
  // Initialize the read address
  if(eepromReadAddress >= EEPROM_BYTES) findEEPROMReadAddress();

  // Is it still empty?
  if(eepromReadAddress >= EEPROM_BYTES)
  {
    // No log block was found.    
    return 0;
  }
  
  uint16_t nextReadAddress = readLogFrom(eepromReadAddress, timestamp, data, size);

  if(nextReadAddress == EEPROM_BYTES+1) 
  {    
    // Indicates no log entry was read (0 start byte) 
    return 0;
  }
      
  // Was read OK so we need to kill that byte, we won't trust the user to have
  // given the correct size here, instead read the start byte
  makeEEPROMSpace(eepromReadAddress, (readEEPROMByte(eepromReadAddress)>>5)+5);
  
  eepromReadAddress = nextReadAddress;
  return 1;
}


DS3231_Simple::DateTime DS3231_Simple::read()
{
  DateTime currentDate;
  uint8_t  x; 

  // Set the register address by doing a write of just the address
  rtc_i2c_seek(0x00);

  // Read in the 7 bytes which store the
  //  Seconds, Minutes, Hours, Day-Of-Week, Day, Month, Year
  if(Wire.requestFrom(RTC_ADDRESS,(uint8_t) 7) == 7)
  {
    currentDate.Second = bcd2bin(Wire.read());
    currentDate.Minute = bcd2bin(Wire.read());
    
    // 6th Bit of hour indicates 12/24 Hour mode, we will always use 24 hour mode, because we is smart
    x = Wire.read();    
    if(x & _BV(6))
    {
      currentDate.Hour = bcd2bin(x & 0B11111) + (x & _BV(5) ? 0 : 12);
    }
    else
    {
      currentDate.Hour = bcd2bin(x & 0B111111);
    }
    
    currentDate.Dow = bcd2bin(Wire.read());
    currentDate.Day = bcd2bin(Wire.read());
    
    x = Wire.read();
    // bit 7 of month indicates if the year is going to be 100+Year or just Year
    if(x&_BV(7))
    {
      currentDate.Year = 100;
    }
    else
    {
      currentDate.Year = 0;
    }
    currentDate.Month = bcd2bin(x & 0B01111111);
    currentDate.Year += bcd2bin(Wire.read());
  }  
  return currentDate;
}

uint8_t DS3231_Simple::write(const DateTime &currentDate)
{
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x00); // Start address of data
  Wire.write(bin2bcd(currentDate.Second));
  Wire.write(bin2bcd(currentDate.Minute));
  Wire.write(bin2bcd(currentDate.Hour));
  Wire.write(bin2bcd(currentDate.Dow?currentDate.Dow:1)); // People might not bother with Dow, make sure it's valid, in case.
  Wire.write(bin2bcd(currentDate.Day));
  Wire.write(bin2bcd(currentDate.Month));
  Wire.write(bin2bcd(currentDate.Year));
  return Wire.endTransmission() ? 0 : 1; // endTransmission returns a code in the response, to make it "Simple" we will return 0 for any fail, and 1 for OK
}

uint8_t DS3231_Simple::setAlarm(const DateTime &AlarmDate, uint8_t AlarmMode)
{
  uint8_t controlByte;
  
  // Read the control byte, we will need to modify the alarm enable bits  
  if(!rtc_i2c_read_byte(0xE,controlByte)) return 0;
  
  //if(AlarmMode >> 5 == 3) // Some custom modes we will rewrite the data and recurse with a standard mode
  if((AlarmMode & 0B00000011) == 0B00000011) // Some custom modes we will rewrite the data and recurse with a standard mode
  {
    
    // If AlarmDate was not declared as a constant reference
    // we could force these, but it comes at a significant cost
    // in flash consumption.
    //
    // AlarmDate.Hour    = 0;
    // AlarmDate.Minute  = 0;
    // AlarmDate.Day     = 1;
    // AlarmDate.Dow     = 1;
  
    // Set to the equivalent Alarm Mode 2, 
    //    for Hourly this will be Match Minute, 
    //    for Daily it will be Match Minute and Match Hour,
    //    for Weekly, Match Minute, Hour and Day-Of-Week (Dow)
    //    for Montly, Match Minute, Hour, and Day-Of-Month (Date)
    
    AlarmMode         = AlarmMode & 0B11111110; 
  }
  
  Wire.beginTransmission(RTC_ADDRESS);
  //if(((AlarmMode >> 5) & 3) == 1) // Alarm 1 Modes
  if(AlarmMode & 0B00000001)
  {
    Wire.write(0x7); // Start address of data for Alarm1
    Wire.write(bin2bcd(AlarmDate.Second) | (AlarmMode & 0B10000000));        
    controlByte = controlByte | _BV(0) | _BV(2); // Enable Alarm 1, set interrupt output on alarm.
  }
  else
  {
    Wire.write(0xB); // Start address of data for Alarm2
    controlByte = controlByte | _BV(1) | _BV(2); // Enable Alarm 2, set interrupt output on alarm.
  }    
  AlarmMode = AlarmMode << 1;
  
  Wire.write(bin2bcd(AlarmDate.Minute) | (AlarmMode & 0B10000000));  
  AlarmMode = AlarmMode << 1;
  
  Wire.write(bin2bcd(AlarmDate.Hour)   | (AlarmMode & 0B10000000));  
  AlarmMode = AlarmMode << 1;
  
  if(AlarmMode & 0B01000000) // DOW indicator
  {
    Wire.write(bin2bcd(AlarmDate.Dow)   | (AlarmMode & 0B10000000) | _BV(6));          
  }
  else
  {
    Wire.write(bin2bcd(AlarmDate.Day)   | (AlarmMode & 0B10000000));
  }    
  AlarmMode = AlarmMode << 2;  // Value and Date/Day indicator
      
  if(Wire.endTransmission()) return 0;
  
  // Write the control byte
  if(rtc_i2c_write_byte(0xE, controlByte)) return 0;
  
  return AlarmMode >> 5;
}

uint8_t DS3231_Simple::setAlarm(uint8_t AlarmMode)
{
  return setAlarm(read(), AlarmMode);
}

uint8_t DS3231_Simple::checkAlarms(uint8_t PauseClock, uint8_t ClearAlarms)
{
  uint8_t StatusByte = 0;
  
  if(PauseClock)
  {
    if(rtc_i2c_read_byte(0xE,StatusByte))
    {
      rtc_i2c_write_byte(0xE, StatusByte | _BV(7));
    }
  }
  
  rtc_i2c_read_byte(0xF,StatusByte);

  if(ClearAlarms)
  {
  if(StatusByte & 0x3)
  {
    // Clear the alarm
    rtc_i2c_write_byte(0xF,StatusByte & ~0x3);    
  }
  }

  if(PauseClock)
  {
    if(rtc_i2c_read_byte(0xE, PauseClock))
    {
      rtc_i2c_write_byte(0xE, PauseClock & ~(_BV(7)));
    }    
  }
  
  return StatusByte & 0x3;
}

uint8_t DS3231_Simple::disableAlarms()
{
  // There's no way to actually disable the alarms from triggering, so
  // we have to set them to some unreachable date
  // (NB: you can disable the alarms from putting the SQW pin low, but they still trigger
  //   in the register itself, you can't stop that, hence this tom-foolery).
  #if 0
  DateTime invalid = { 0,0,0,0,31,2,0 }; 
  #else
  // This saves 4 bytes interestingly (assuming you are already going to be using read() somewhere)
  DateTime invalid = read();
  invalid.Day = 31;
  invalid.Month = 2;
  #endif
  setAlarm(invalid, ALARM_MATCH_MINUTE_HOUR_DATE);
  setAlarm(invalid, ALARM_MATCH_SECOND_MINUTE_HOUR_DATE);
  checkAlarms(); // A dummy check alarm to kill the alarm flags.
  
  return 1;  
}

uint8_t DS3231_Simple::getTemperature()
{
  rtc_i2c_seek(0x11);

  uint8_t t = 0;
  uint8_t x = 0;
  if(Wire.requestFrom(RTC_ADDRESS,(uint8_t) 2) == 2)
  {
    t = Wire.read();
    
    // The last 2 bits are the number of 0.25 degree units above
    // the whole degree, we will implement "Bankers Rounding"
    // such that 0.5 (.25 * 2) is rounded to the nearest even number
    x = (Wire.read() >> 6);
    
    if( (x > 2) || (( x == 2 ) && (t & 0x01)))
    {      
      t++;    
    }
  }
  return t;
}

float DS3231_Simple::getTemperatureFloat()
{
  rtc_i2c_seek(0x11);

  float t = 0;
  if(Wire.requestFrom(RTC_ADDRESS,(uint8_t) 2) == 2)
  {
    t = Wire.read();
    t += (Wire.read() >> 6) * 0.25;
  }
  
  return t;
}





void DS3231_Simple::printTo(Stream &Printer)
{
    printTo(Printer, read());
}

void DS3231_Simple::printTo(Stream &Printer, const DateTime &timestamp)
{
  printDateTo_YMD(Printer, timestamp);
  Printer.print('T');
  printTimeTo_HMS(Printer, timestamp);
}

void DS3231_Simple::printDateTo_DMY(Stream &Printer, const DateTime &Timestamp, const char separator)
{  
  print_zero_padded(Printer, Timestamp.Day);
  Printer.print(separator);
  print_zero_padded(Printer, Timestamp.Month);  
  Printer.print(separator);
  if(Timestamp.Year > 100)
  {
    Printer.print('2');
  }
  else
  {
    Printer.print(F("20"));
  }
  print_zero_padded(Printer, Timestamp.Year); 
}

void DS3231_Simple::printDateTo_MDY(Stream &Printer, const DateTime &Timestamp, const char separator)
{  
  print_zero_padded(Printer, Timestamp.Month);  
  Printer.print(separator);
  print_zero_padded(Printer, Timestamp.Day);
  Printer.print(separator);
  if(Timestamp.Year > 100)
  {
    Printer.print('2');
  }
  else
  {
    Printer.print(F("20"));
  }
  print_zero_padded(Printer, Timestamp.Year); 
}

void DS3231_Simple::printDateTo_YMD(Stream &Printer, const DateTime &Timestamp, const char separator)
{  
  if(Timestamp.Year > 100)
  {
    Printer.print('2');
  }
  else
  {
    Printer.print(F("20"));
  }
  print_zero_padded(Printer, Timestamp.Year); 
  Printer.print(separator);
  print_zero_padded(Printer, Timestamp.Month);  
  Printer.print(separator);
  print_zero_padded(Printer, Timestamp.Day);  
}
    
void DS3231_Simple::printTimeTo_HMS(Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator , const char minutesToSecondsSeparator )
{
  print_zero_padded(Printer, Timestamp.Hour);
  Printer.print(hoursToMinutesSeparator);
  print_zero_padded(Printer, Timestamp.Minute);
  
  if(minutesToSecondsSeparator != 0x03)
  {
    Printer.print(minutesToSecondsSeparator);
    print_zero_padded(Printer, Timestamp.Second);    
  }
}


void DS3231_Simple::printTimeTo_HM (Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator )
{
  printTimeTo_HMS(Printer, Timestamp, hoursToMinutesSeparator, 0x03);
}


void DS3231_Simple::print12HourTimeTo_HMS(Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator , const char minutesToSecondsSeparator )
{
  if(Timestamp.Hour > 12) 
  {
    Printer.print(Timestamp.Hour-12);    
  }
  else
  {
    Printer.print(Timestamp.Hour ? Timestamp.Hour : 12); // Handle 0 hour = 12 as well    
  }
  
  Printer.print(hoursToMinutesSeparator);
  print_zero_padded(Printer, Timestamp.Minute);
  
  if(minutesToSecondsSeparator != 0x03)
  {
    Printer.print(minutesToSecondsSeparator);
    print_zero_padded(Printer, Timestamp.Second);    
  }

  if(Timestamp.Hour > 12)
  {
    Printer.print(F(" PM"));
  }
  else
  {
    Printer.print(F(" AM"));   
  }  
}

void DS3231_Simple::print12HourTimeTo_HM (Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator )
{
  print12HourTimeTo_HMS(Printer, Timestamp, hoursToMinutesSeparator, 0x03);
}

void DS3231_Simple::promptForTimeAndDate(Stream &Serial)
{
  char buffer[3] = { 0 };
  DateTime Settings;
    
  /*
  Serial.printf("\nSerial.getTimeout() = %d\n", Serial.getTimeout());
  Serial.setTimeout(2*Serial.getTimeout());
  Serial.printf("Serial.getTimeout() now %d\n", Serial.getTimeout());
  */

  Serial.println(F("Clock is set when all data is entered and you send 'Y' to confirm."));
  do
  {
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter day of month (2 digits, 01-31): "));
    while (Serial.available() < 2) ; // Wait for 2 bytes  
    Serial.readBytes(buffer, 2);
    Settings.Day = atoi(buffer[0] == '0' ? buffer+1 : buffer); // Parse, skipping leading 0 if any
  
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter month (2 digits, 01-12): "));
    while(Serial.available() < 2) ; // Wait for 2 bytes
    Serial.readBytes(buffer, 2);
    Settings.Month = atoi(buffer[0] == '0' ? buffer+1 : buffer); // Parse, skipping leading 0 if any
  
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter year (2 digits, 00-99): "));
    while(Serial.available() < 2) ; // Wait for 2 bytes
    Serial.readBytes(buffer, 2);
    Settings.Year = atoi(buffer[0] == '0' ? buffer+1 : buffer); // Parse, skipping leading 0 if any

    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter hour (2 digits, 24 hour clock, 00-23): "));
    while(Serial.available() < 2) ; // Wait for 2 bytes
    Serial.readBytes(buffer, 2);
    Settings.Hour = atoi(buffer[0] == '0' ? buffer+1 : buffer); // Parse, skipping leading 0 if any
 
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter minute (2 digits, 00-59): "));
    while(Serial.available() < 2) ; // Wait for 2 bytes
    Serial.readBytes(buffer, 2);
    Settings.Minute = atoi(buffer[0] == '0' ? buffer+1 : buffer); // Parse, skipping leading 0 if any
  
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter second (2 digits, 00-59): "));
    while (Serial.available() < 2) ; // wait until 2 bytes
    Serial.readBytes(buffer, 2);
    Settings.Second = atoi(buffer[0] == '0' ? buffer+1 : buffer); // Parse, skipping leading 0 if any
 
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    memset(buffer, 0, sizeof(buffer));
    while(Serial.available()) Serial.read(); // clear buffer
    Serial.print(F("\nEnter day of week (1 digit, 1-7, arbitrarily 1 = Mon, 7 = Sun): "));    
    while(!Serial.available()) ; // Wait for 1 bytes
    Serial.readBytes(buffer, 1);
    Settings.Dow = atoi(buffer);
      
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    Serial.print(F("\nEntered timestamp: ")); 
    printTo(Serial, Settings);  
    while(Serial.available()) Serial.read();  // clear buffer
    Serial.print(F("\nEnter 'Y' to set the clock, anything else to start again: "));       
    while(!Serial.available()) ; // Wait until bytes
    Serial.readBytes(buffer, 1);
    #ifdef INPUT_DELAY 
    delay(INPUT_DELAY);
    #endif
    while(Serial.available()) Serial.read(); 
    if(buffer[0] == 'Y' || buffer[0] == 'y')
    {  
      write(Settings); 
      break;
    }
  } while(1);   
}
