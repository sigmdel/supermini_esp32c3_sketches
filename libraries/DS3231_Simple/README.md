# DS3231_Simple
An Arduino Library for EASY communication with DS3231 I2C RTC Clock and Atmel AT24C32 I2C EEPROM commonly found on the same board.  Implements setting, getting the time/date, setting, checking and clearing alarms, and dead-easy circular-buffered logging of data with timestamp.

## What is this module?

The DS3231 is a precision Real Time Clock module using the I2C bus.

It includes the ability to read and set the time/date, set up to 2 alarms (one with seconds precision, one with minutes precision), and the ability to read the current temperature with precision of up to 0.25 degrees C.

Additionally the RTC is often found paired with an I2C EEPROM (usually AT24C32) which can be used for convenient data-logging purposes.

![ZS-042 RTC and EEPROM Module](https://cdn.rawgit.com/sleemanj/DS3231_Simple/master/docs/zs-042.jpg)

## Why this library?

This library was written because other libraries I had seen seemed less than satisfactorily simple.

This library makes it a very simple job to handle all the functions of the clock, and extremely easy to log data of **arbitrary data types** with essentially just one command - for example if you wanted to log the results of an `analogRead()`, then simply... `Clock.writeData(analogRead(A1));` is all you need.  Whats more you can log any other scalar *or struct* data type in exactly the same way (through the wonderous magic of C++ templates).

## How do I connect my module?

These modules are I2C devices, so that's easy as pie.

* SCL     ->     SCL (Uno: A5, Mega: 21, Leo*: SCL)
* SDA     ->     SDA (Uno: A4, Mega: 20, Leo*: SDA)
* VCC     ->     5v
* GND     ->     GND

*Arduino Leonardo users should read this thread:  http://forum.arduino.cc/index.php?topic=113431.0

## Download, Install and Example

* Download: https://sparks.gogo.co.nz/DS3231_Simple.zip
* Open the Arduino IDE (this library was tested in 1.6.7, but older versions should be fine too)
* Select the menu item Sketch > Import Library > Add Library
* Choose to install the DS3231_Simple.zip file you downloaded
* Now you can choose File > Examples > DS3231_Simple > HelloWorld

Upload the example to your Arduino and open the Serial Terminal at 9600 Baud.  The HelloWorld sketch will prompt you to set the time, the (battery on your module will keep the time ticking away once it is set).

Choose the other examples starting with those in "z1_TimeAndDate" for the basis and progressing through to z4_DataLogging for the advanced topics (z1 / z2... is just because the ArduinoIDE doesn't have a good way to sort the Examples).

## Full Class Reference

I recommend to just look at the examples which show you how to use all the features, but if you want the nitty-gritty then here is the [full class reference](https://cdn.rawgit.com/sleemanj/DS3231_Simple/31d0dac/docs/html/class_d_s3231___simple.html)

Thanks to the super kind folks of [RawGit](https://rawgit.com/) and [MaxCDN](http://www.maxcdn.com/)
