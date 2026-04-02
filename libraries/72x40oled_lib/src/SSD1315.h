#ifndef SSD1315_H
#define SSD1315_H

#include <Arduino.h>
#if defined(__AVR_ATtiny85__) || defined(ARDUINO_AVR_DIGISPARK)
#include <TinyWireM.h>
#define Wire TinyWireM
#else
#include <Wire.h>
#endif
#if defined(AVR)
#include <avr/pgmspace.h>
#else  //defined(AVR)
#include <pgmspace.h>
#endif  //defined(AVR)

#define SSD1315_WIDTH 72
#define SSD1315_HEIGHT 40
#define SSD1315_PAGES (SSD1315_HEIGHT/8)

#define NO_RESET_PIN 255

class SSD1315 {
public:
    SSD1315(uint8_t rstPin=8);
    void begin();
    void clear();
    void fill(uint8_t color);
    void display();
    void invert(bool enable);
    void setRotation(uint8_t r);
    void setBrightness(uint8_t level);
    void sleep(bool enable);
    void drawPixel(int x,int y,uint8_t color);
    void drawRect(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,bool fill=false,bool invert=false);
    void drawBitmap(uint8_t x,uint8_t y,const uint8_t* bmp,uint8_t w,uint8_t h,bool invert=false);
    void drawChar(uint8_t x,uint8_t y,char c,uint8_t size=12,bool invert=false);
    void drawString(uint8_t x,uint8_t y,const char* str,uint8_t size=12,bool invert=false);
    uint8_t* getBuffer();
private:
    uint8_t _rst;
    uint8_t _rotation;
    uint8_t buffer[SSD1315_WIDTH*SSD1315_HEIGHT/8];
    void command(uint8_t c);
    void data(uint8_t d);
};

extern const uint8_t Font1612[11][32] PROGMEM;
extern const uint8_t Font1206[95][12] PROGMEM;
extern const uint8_t Font1608[95][16] PROGMEM;
extern const uint8_t Font3216[11][64] PROGMEM;
extern const uint8_t Font0508[95][5] PROGMEM;
extern const uint8_t PIC1[] PROGMEM;
extern const uint8_t PIC2[] PROGMEM;
extern const uint8_t Signal816[16] PROGMEM;
extern const uint8_t Msg816[16] PROGMEM;
extern const uint8_t Bat816[16] PROGMEM;
extern const uint8_t Bluetooth88[8] PROGMEM;
extern const uint8_t GPRS88[8] PROGMEM;
extern const uint8_t Alarm88[8] PROGMEM;

#endif
