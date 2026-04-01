/**
 * Simple DS3231 RTC and AT24C32 EEPROM Library
 *
 * Implements a simple interface to read/set the clock and alarms of an
 * DS3231 Real Time Clock, and also to utilise an often-companion AT24C32
 * EEPROM as a data logging storage for generic timestamped data. 
 * 
 * Copyright (C) 2016 James Sleeman
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 * 
 * @author James Sleeman, http://sparks.gogo.co.nz/
 * @license MIT License
 */

// Please note, only one underscore is permitted in the guard define (Arduino IDE weirdness)
#ifndef DS3231Easy_h
#define DS3231Easy_h
#include <Wire.h>
#include <Stream.h>

#ifndef _BV
#define _BV(b) (1UL << (b))
#endif

class DS3231_Simple
{
  public:
    
 // The DateTime structure can be implemented using bit-fields to probably reduce RAM
 // footprint, but costs quite a lot of flash (about 400 bytes) due to bit-juggling, 
 // Functionally equivalent in all other respects.
    
 // #define USE_BIT_FIELDS
    #ifdef USE_BIT_FIELDS
    struct DateTime 
    {      
      uint8_t Second:6;  // 0-59 = 6 bits
      uint8_t Minute:6;  // 0-59 = 6 bits
      uint8_t Hour:5;    // 0-23 = 5 bits
      uint8_t Dow:3;     // 1-7  = 3 bits (Day Of Week)
      uint8_t Day:5;     // 1-31 = 5 bits
      uint8_t Month:4;   // 1-12 = 4 bits
      uint8_t Year;      // 0-199 = 8 bits                  
    };
    #else
    struct DateTime 
    {      
      uint8_t Second;//:6;  // 0-59 = 6 bits
      uint8_t Minute;//:6;  // 0-59 = 6 bits
      uint8_t Hour;//:5;    // 0-23 = 5 bits
      uint8_t Dow;//:3;     // 1-7  = 3 bits (Day Of Week)
      uint8_t Day;//:5;     // 1-31 = 5 bits
      uint8_t Month;//:4;   // 1-12 = 4 bits
      uint8_t Year;      // 0-199 = 8 bits                  
    };
    #endif

    
    
  protected: 
  
    static const uint8_t      RTC_ADDRESS  = 0x68; 
      // Datasheet, Page 16, Figure 3.  Note that this is 7 bits, the last (LSB) bit of an I2C address indicates Read from Slave (1) or Write to Slave (1)   
      
    static uint8_t bcd2bin(uint8_t binaryRepresentation);
    static uint8_t bin2bcd(uint8_t bcdRepresentation);
    static uint8_t rtc_i2c_seek(const uint8_t Address);
    static uint8_t rtc_i2c_write_byte(const uint8_t Address, const uint8_t Byte);    
    static uint8_t rtc_i2c_read_byte(const uint8_t Address,  uint8_t &Byte);    
    static void    print_zero_padded(Stream &Printer, uint8_t x);    
          
  public:
    /* 
    static const uint8_t ALARM_EVERY_SECOND                    = 0B00101111;
    static const uint8_t ALARM_MATCH_SECOND                    = 0B00101110;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE             = 0B00101100;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE_HOUR        = 0B00101000;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE_HOUR_DATE   = 0B00100000;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE_HOUR_DOW    = 0B00110000;

    static const uint8_t ALARM_EVERY_MINUTE                    = 0B01000111;
    static const uint8_t ALARM_MATCH_MINUTE                    = 0B01000110;
    static const uint8_t ALARM_MATCH_MINUTE_HOUR               = 0B01000100;
    static const uint8_t ALARM_MATCH_MINUTE_HOUR_DATE          = 0B01000000;
    static const uint8_t ALARM_MATCH_MINUTE_HOUR_DOW           = 0B01010000;

    static const uint8_t ALARM_HOURLY                          = 0B11000110;
    static const uint8_t ALARM_DAILY                           = 0B11000100;
    static const uint8_t ALARM_WEEKLY                          = 0B11010000;
    static const uint8_t ALARM_MONTHLY                         = 0B11000000;    
    */
    
    static const uint8_t ALARM_EVERY_SECOND                    = 0B11110001;
    static const uint8_t ALARM_MATCH_SECOND                    = 0B01110001;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE             = 0B00110001;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE_HOUR        = 0B00010001;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE_HOUR_DATE   = 0B00000001;
    static const uint8_t ALARM_MATCH_SECOND_MINUTE_HOUR_DOW    = 0B00001001;

    static const uint8_t ALARM_EVERY_MINUTE                    = 0B01110010; 
    static const uint8_t ALARM_MATCH_MINUTE                    = 0B00110010;
    static const uint8_t ALARM_MATCH_MINUTE_HOUR               = 0B00010010;
    static const uint8_t ALARM_MATCH_MINUTE_HOUR_DATE          = 0B00000010;
    static const uint8_t ALARM_MATCH_MINUTE_HOUR_DOW           = 0B00001010;

    static const uint8_t ALARM_HOURLY                          = 0B00110011;
    static const uint8_t ALARM_DAILY                           = 0B00010011;
    static const uint8_t ALARM_WEEKLY                          = 0B00001011;
    static const uint8_t ALARM_MONTHLY                         = 0B00000011;  
    
    /** Initialize.
     *  
     */
     
    void begin();

    /** Read the current date and time, returning a structure containing that information.
     *  
     */
     
    DateTime read();

    /** Set the date and time from the settings in the given structure.
     *  
     *  @param The date/time
     */
     
    uint8_t  write(const DateTime&);

    void     promptForTimeAndDate(Stream &Serial);
    
    /** Sets an alarm, the alarm will pull the SQW pin low (you can monitor with an interrupt).
     *  
     *  @param The date/time for the alarm, as appropriate for the alarm mode (example, for 
     *    ALARM_MATCH_SECOND then AlarmTime.Second will be the matching criteria).
     *    
     *  @param The mode of the alarm, from the following...
     *  
     *    ALARM_EVERY_SECOND                    
     *    ALARM_MATCH_SECOND                    
     *    ALARM_MATCH_SECOND_MINUTE             
     *    ALARM_MATCH_SECOND_MINUTE_HOUR        
     *    ALARM_MATCH_SECOND_MINUTE_HOUR_DATE   
     *    ALARM_MATCH_SECOND_MINUTE_HOUR_DOW    
     *    
     *    ALARM_EVERY_MINUTE                    
     *    ALARM_MATCH_MINUTE                    
     *    ALARM_MATCH_MINUTE_HOUR               
     *    ALARM_MATCH_MINUTE_HOUR_DATE          
     *    ALARM_MATCH_MINUTE_HOUR_DOW           
     *    
     *    ALARM_HOURLY                          
     *    ALARM_DAILY                           
     *    ALARM_WEEKLY                          
     *    ALARM_MONTHLY                      
     *    
     */

    uint8_t  setAlarm(const DateTime &AlarmTime, uint8_t AlarmMode);

    /** Set an alarm using the current date/time as it's basis.  Just a convenience function.
     *  
     *  @param The mode of the alarm, from the following...
     *  
     *    ALARM_EVERY_SECOND                    
     *    ALARM_MATCH_SECOND                    
     *    ALARM_MATCH_SECOND_MINUTE             
     *    ALARM_MATCH_SECOND_MINUTE_HOUR        
     *    ALARM_MATCH_SECOND_MINUTE_HOUR_DATE   
     *    ALARM_MATCH_SECOND_MINUTE_HOUR_DOW    
     *    
     *    ALARM_EVERY_MINUTE                    
     *    ALARM_MATCH_MINUTE                    
     *    ALARM_MATCH_MINUTE_HOUR               
     *    ALARM_MATCH_MINUTE_HOUR_DATE          
     *    ALARM_MATCH_MINUTE_HOUR_DOW           
     *    
     *    ALARM_HOURLY
     *    ALARM_DAILY                          
     *    ALARM_WEEKLY                         
     *    ALARM_MONTHLY
     */
     
    uint8_t  setAlarm(uint8_t AlarmMode);    

    
    /** Disable any existing alarm settings.             
     *  
     *  @return Success True/False
     */
     
    uint8_t  disableAlarms();
    
    /** Determine if an alarm has triggered, also clears the alarm if so.
     *  
     *  If you do not pause the clock then be aware that there is a small chance 
     *  that you may miss an alarm between the very short time that the alarms 
     *  are checked and then cleared (ie, if an alarm is checked and is 0, and 
     *  before we clear it, the alarm is triggered, but then we go and clear it).
     *  
     *  If you pause the clock, then this won't happen, but your time keeping
     *  will not be so accurate because every time you check the alarm you will 
     *  "get behind" a small amount.
     * 
     *  Can be "read only" by disabling the clearing of the alarms. By default
     *  will clear alarms.
     *  
     *  @return 0 For no alarm, 1 for Alarm 1, 2 for Alarm 2, and 3 for Both Alarms     
     */
     
    uint8_t  checkAlarms(uint8_t PauseClock = false, uint8_t ClearAlarms = true);

    /** Get the temperature accurate to within 1 degree (C)
     *  
     */
     
    uint8_t  getTemperature();

    /** Get the temprature accurate to within 0.25 degrees (C)
     *  
     */
     
    float    getTemperatureFloat();

    /** Print the current DateTime structure in ISO8601 Format
     *  
     *  YYYY-MM-DDThh:mm:ss
     *  
     *  https://www.w3.org/TR/NOTE-datetime
     *  
     *  Example: Clock.printTo(Serial);
     */
     
    void     printTo(Stream &Printer);

    /** Print the given DateTime structure in ISO8601 Format
     *  
     *  YYYY-MM-DDThh:mm:ss
     *  
     *  https://www.w3.org/TR/NOTE-datetime
     *  
     *  Example: Clock.printTo(Serial, MyDateAndTimeVaraible);
     */
     
    void     printTo(Stream &Printer, const DateTime &Timestamp);
    
    /** Print the date portion of the given DateTime structure in DD/MM/YYYY format.
     *  
     */
    void     printDateTo_DMY(Stream &Printer, const DateTime &Timestamp, const char separator = '/');
    
    /** Print the date portion of the current DateTime structure in DD/MM/YYYY format.
     *  
     */
    void     printDateTo_DMY(Stream &Printer) { printDateTo_DMY(Printer, read()); }
    
    /** Print the date portion of the given DateTime structure in MM/DD/YYYY format.
     *  
     */
    void     printDateTo_MDY(Stream &Printer, const DateTime &Timestamp, const char separator = '/');
    
    /** Print the date portion of the current DateTime structure in MM/DD/YYYY format.
     *  
     */
    void     printDateTo_MDY(Stream &Printer) { printDateTo_MDY(Printer, read()); }
    
    /** Print the date portion of the given DateTime structure in YYYY-MM-DD format.
     *  
     */
    void     printDateTo_YMD(Stream &Printer, const DateTime &Timestamp, const char separator = '-');       
    
    /** Print the date portion of the current DateTime structure in YYYY-MM-DD format.
     *  
     */
    void     printDateTo_YMD(Stream &Printer) { printDateTo_YMD(Printer, read()); }
    
    /** Print the time portion of the given DateTime structure in HH:MM:SS format (24 Hour Clock)
     *  
     */
    void     printTimeTo_HMS(Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator = ':', const char minutesToSecondsSeparator = ':');
    
    /** Print the time portion of the current DateTime structure in HH:MM:SS format (24 Hour Clock)
     *  
     */
    void     printTimeTo_HMS(Stream &Printer) { printTimeTo_HMS(Printer, read()); }

    /** Print the time portion of the given DateTime structure in HH:MM format (24 Hour Clock)
     *  
     */
    void     printTimeTo_HM (Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator = ':');
    
    /** Print the time portion of the current DateTime structure in HH:MM format (24 Hour Clock)
     *  
     */
    void     printTimeTo_HM(Stream &Printer) { printTimeTo_HM(Printer, read()); }

    /** Print the time portion of the given DateTime structure in HH:MM:SS [AM/PM] format (12 Hour Clock)
     *  
     */
    void     print12HourTimeTo_HMS(Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator = ':', const char minutesToSecondsSeparator = ':');
    
    /** Print the time portion of the current DateTime structure in HH:MM:SS [AM/PM] format (12 Hour Clock)
     *  
     */
    void     print12HourTimeTo_HMS(Stream &Printer) { print12HourTimeTo_HMS(Printer, read()); }

    /** Print the time portion of the given DateTime structure in HH:MM:SS [AM/PM] format (12 Hour Clock)
     *  
     */    
    void     print12HourTimeTo_HM (Stream &Printer, const DateTime &Timestamp, const char hoursToMinutesSeparator = ':');
    
    /** Print the time portion of the current DateTime structure in HH:MM:SS [AM/PM] format (12 Hour Clock)
     *  
     */    
    void     print12HourTimeTo_HM(Stream &Printer) { print12HourTimeTo_HM(Printer, read()); }
    
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // EEPROM LOGGING
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  protected:
  
    static const uint8_t      EEPROM_ADDRESS   = 0x57;                          
      // 7 Bit address, the first 4 bits are 1010, the the last 3 bits according to A2, A1 and A0
      // On the common ZS-042 board, this corresponds to (where x is jumper open, and 1 is jumper closed)
      // A0    A1    A2
      //  x    x     x    0x57  (default)
      //  1    x     x    0x56
      //  x    1     x    0x55
      //  1    1     x    0x54
      //  0    0     1    0x53
      //  1    0     1    0x52
      //  1    1     1    0x51
                                                                                
                                                                                
    static const uint16_t     EEPROM_SIZE_KBIT = 32768;                         // EEPROMs are sized in kilobit
    static const uint8_t      EEPROM_PAGE_SIZE = 32;                            // And have a number of bytes per page
    static const uint16_t     EEPROM_BYTES     = EEPROM_SIZE_KBIT/8;            
    static const uint8_t      EEPROM_PAGES     = EEPROM_BYTES/EEPROM_PAGE_SIZE; 

    // EEPROM structure       
    //  The EEPROM is used to store "log entries" which each consist of a 5 byte header and an additional 0 to 7 data bytes.
    //  The Header of each block includes a count of the data bytes and then a binary representation of the timestamp.
    //
    //  Blocks are recorded in a circular-buffer fashion in order to reduce wear on the EEPROM, that is, each next block is stored
    //  after the block preceeding it, when the top of the EEPROM is reached the next block goes to the EEPROM address zero.
    //
    //  Blocks do NOT cross EEPROM boundary (a block will not start at the top of the EEPROM and loop around to the bottom) before ending
    //  if a block doesn't fit in the available space at the top of the blocks in the EEPROM, it will be placed in address zero.  
    // 
    //  Blocks are zeroed completely on read or when they are to be overwritten (even partially overwritten the entire block is nuked).
    //    
    //  <Block>     ::= <Header><DataBytes>
    //  <Header> ::= 0Bzzzwwwyy yyyyyymm mmdddddh hhhhiiii iissssss binary representation of DateTime, (zzz = number of data bytes following timestamp, www = day-of-week)
    //  <DataBytes> ::= DB1..7   

    
    uint16_t                  eepromWriteAddress   = EEPROM_BYTES;               // Byte address of the "top" of the EEPROM "stack", the next
                                                                                // "block" stored will be put here, this location may be 
                                                                                // a valid block start byte, or it may be 00000000 in which case
                                                                                // there are zero bytes until the next block start which will be
                                                                                // the first of the series.      
                                                                                
    uint16_t                  eepromReadAddress = EEPROM_BYTES;                 // Byte address of the "bottom" of the EEPROM "stack", the next
                                                                                // "block" to read is found here, this location may be 
                                                                                // a valid block start byte, or it may be 00000000 in which case
                                                                                // there are zero bytes to read.

    /** Searches the EEPROM for the next place to store a block, sets eepromWriteAddress
     *  
     *  @return eepromWriteAddress
     */
     
    uint16_t findEEPROMWriteAddress();    

    /** Delete enough complete blocks to have enough free space for the 
     *  given required number of bytes.
     *  
     *  @return True/False for success/fail
     */
     
    uint8_t  makeEEPROMSpace(uint16_t Address, int8_t BytesRequired);

    /** Find the oldest block to read (based on timestamp date), set eepromReadAddress
     *  
     *  Note: Has to search entire EEPROM, slow.
     *  
     *  @return eepromReadAddress
     */
     
    uint16_t findEEPROMReadAddress();

    /** Read log timestamp and data from a given EEPROM address.
     *  
     *  @param Address Byte address of log block
     *  @param timestamp DateTime structure to put the timestamp
     *  @param data Memory location to put the data associated with the log
     *  @param size Max size of the data to read (any more is discarded)
     */
     
    uint16_t readLogFrom(uint16_t Address, DateTime &timestamp, uint8_t *data, uint8_t size = 0);

    /** Start a "pagewize" write at the eepromWriteAddress.
     *  
     *  Follow this call with 1 or more calls to writeBytePagewize()      
     *  Finish with a call to writeBytePagewizeEnd()
     *  
     *  Writes in the same page (or rather, 16 byte sections of a page) 
     *  are performed as a single write, thence a new write is started
     *  for the next section.
     *  
     *  This solves 2 problems, first the 32 byte Wire buffer which isn't 
     *  large enough for a whole page of bytes plus the address bytes.
     *  Second the fact that the EEPROM loops back to the start of page
     *  when you run off the end of the page while writing (instead of
     *  to the start of the next page).
     *       
     *  Note that you must not modify the eepromWriteAddress between
     *  a writeBytePagewizeStart() and a writeBytePagewizeEnd()
     *  or it's all going to get out of kilter.
     * 
     *  @return Success (boolean) 1/0, presently this method always reports success.
     *  
     */
    uint8_t writeBytePagewizeStart();

    /** Write a byte during a pagewize operation.
     *  
     *  Note that this function increments the eepromWriteAddress.
     *  
     *  @param data Byte to write.
     *  @see DS3231::writeBytePagewizeStart()
     *  @return Success (boolean) 1/0, presently this method always reports success.
     */
     
    uint8_t writeBytePagewize(const uint8_t data);

    /** End a pagewize write operation.
     *      
     *  @see DS3231::writeBytePagewizeStart()
     *  @return Success (boolean) 1/0, this method will return 0 if Wire.endTransmission reports an error.
     */
    
    uint8_t writeBytePagewizeEnd();
      
    /** Read a byte from the EEPROM
     * 
     *  @param Address The address of the EEPROM to read from.
     *  @return The data byte read.  
     *  @note   There is limited error checking, if you provide an invalid address, or the EEPROM is not responding etc behaviour is undefined (return 0, return 1, might or might not block...).
     */
    uint8_t  readEEPROMByte(const uint16_t Address);

    
  public:
    /** Erase the EEPROM ready for storing log entries. */
    
    uint8_t  formatEEPROM();
  
    /** Write a log entry to the EEPROM, having current timestamp, with an attached data of arbitrary datatype (7 bytes max).
     *  
     *  This method allows you to record a "log entry" which you can later retrieve.
     *  
     *  The full timestamp is recorded along with one piece of data.  The type of data you provide
     *  is up to you, as long as your data is less than 7 bytes (byte, int, float, char, 
     *  or a 6 character string would all be fine).  
     *  
     *  To store more than one piece of data in a log, use a struct, again, in total your structure
     *  needs to occupy 7 bytes or less of memory.
     *  
     *  Examples: 
     *     Clock.writeLog(analogRead(A0));
     *     Clock.writeLog( MyTimeAndDate, MyFloatVariable );
     *     
     *     // Remember, 7 bytes max!
     *     struct MyDataStructure 
     *     {
     *        unsigned int AnalogValue1;
     *        unsigned int AnalogValue2;
     *     };
     *     MyDataStructure MyData; MyData.AnalogValue1 = 123; MyData.AnalogValue2 = 456;
     *     Clock.writeLog(MyDataStructure);
     *     
     *  
     *  @param data  The data to store, any arbitrary scalar or structur datatype consisting not more than 7 bytes.
     *  @note  To store a string or other pointer contents, you probably want to use `DS3231::writeLog(const DateTime, const uint8_t *data, uint8_t size)`
     *  
     */
     
    template <typename datatype>
      uint8_t  writeLog( const datatype &data  )   { 
         return writeLog(read(), (uint8_t *) &data, (uint8_t)sizeof(datatype));         
      }
   
    /** Write a log entry to the EEPROM, having supplied timestamp, with an attached data of arbitrary datatype (7 bytes max).
     * 
     * @see DS3231::writeLog(const datatype &   data)
     * @param timestamp The timestamp to associate with the log entry.
     * @param data  The data to store, any arbitrary datatype consisting not more than 7 bytes.
     */
    
    template <typename datatype>
      uint8_t  writeLog( const DateTime &timestamp,  const datatype &data  )   {      
         return writeLog(timestamp, (uint8_t *) &data, (uint8_t)sizeof(datatype));         
      }

    /** Write a log entry to the EEPROM, having supplied timestamp, with an attached data.
     *            
     * @param timestamp The timestamp to associate with the log entry.
     * @param data  Pointer to the data to store
     * @param size  Length of data to store - max length is 7 bytes, this is not checked for compliance!
     */
    
    uint8_t  writeLog( const DateTime &timestamp,  const uint8_t *data, uint8_t size = 1 );
    

    /** Read the oldest log entry and clear it from EEPROM.
     *  
     *  @param timestamp Variable to put the timestamp of the log into.
     *  @param data      Variable to put the data.
     *  
     *  @note No check is made as to if the type of "data" and the type of
     *    the data which is read is the same, there is nothing to stop you
     *    doing "writeLog( myFloat );" and subsequently "readLog( myInt );"
     *    but if you do so, results are going to be not what you expect!
     *  
     */
    
    template <typename datatype>
      uint8_t  readLog( DateTime &timestamp,  datatype &data  )   {   
         return readLog(timestamp, (uint8_t *) &data, (uint8_t)sizeof(datatype));         
      }
      
    /** Read the oldest log entry and clear it from EEPROM.
     *  
     *  @param timestamp Variable to put the timestamp of the log into.
     *  @param data      Pointer to buffer to put data associated with the log.
     *  @param size      Size of the data buffer.  Maximum 7 bytes.
     *  
     *  @note If the data in the log entry is larger than the buffer, it will be truncated.
     *  
     */
    
    uint8_t  readLog( DateTime &timestamp,         uint8_t *data,       uint8_t size = 1 );
    

    /** Compare two DateTime objects to determine which one is older.
     *  
     *  If A is older than B return -1
     *  If A is equal to B   return  0
     *  If A is newer than B return  1
     *  
     * @return -1, 0 or 1
     */
    
    int8_t   compareTimestamps(const DateTime &A, const DateTime &B);
    
};

typedef DS3231_Simple::DateTime DateTime;
#endif
