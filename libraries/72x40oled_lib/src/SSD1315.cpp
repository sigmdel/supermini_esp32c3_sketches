#include "SSD1315.h"

static void I2C_Write(uint8_t value,uint8_t cmd){
    Wire.beginTransmission(0x3c);
    Wire.write(cmd);
    Wire.write(value);
    Wire.endTransmission();
}

SSD1315::SSD1315(uint8_t rstPin):_rst(rstPin),_rotation(0){}

void SSD1315::command(uint8_t c){I2C_Write(c,0x00);} 
void SSD1315::data(uint8_t d){I2C_Write(d,0x40);} 

void SSD1315::begin(){
    if (_rst < NO_RESET_PIN) {
        pinMode(_rst,OUTPUT);
        digitalWrite(_rst,HIGH);
        delay(10);
        digitalWrite(_rst,LOW);
        delay(10);
        digitalWrite(_rst,HIGH);
    }
    command(0xae);
    command(0xd5);
    command(0x80);
    command(0xa8);
    command(0x27);
    command(0xd3);
    command(0x00);
    command(0xad);
    command(0x30);
    command(0x8d);
    command(0x14);
    command(0x40);
    command(0xa6);
    command(0xa4);
    command(0xa1);
    command(0xc8);
    command(0xda);
    command(0x12);
    command(0x81);
    command(0xaf);
    command(0xd9);
    command(0x22);
    command(0xdb);
    command(0x20);
    command(0xaf);
    clear();
    display();
}

void SSD1315::clear(){
    memset(buffer,0,sizeof(buffer));
}

void SSD1315::fill(uint8_t color){
    memset(buffer,color?0xff:0x00,sizeof(buffer));
}

void SSD1315::invert(bool en){
    command(en?0xA7:0xA6);
}

void SSD1315::setRotation(uint8_t r){
    _rotation=r&3;
}

void SSD1315::setBrightness(uint8_t level){
    command(0x81);
    command(level);
}

void SSD1315::sleep(bool en){
    command(en?0xAE:0xAF);
}

void SSD1315::drawPixel(int x,int y,uint8_t color){
    switch(_rotation & 3){
        case 1: {int t = x; x = y; y = SSD1315_WIDTH - 1 - t; break;}
        case 2: x = SSD1315_WIDTH - 1 - x; y = SSD1315_HEIGHT - 1 - y; break;
        case 3: {int t = x; x = SSD1315_HEIGHT - 1 - y; y = t; break;}
    }
    if(x<0||x>=SSD1315_WIDTH||y<0||y>=SSD1315_HEIGHT)return;
    uint16_t i=x+(y/8)*SSD1315_WIDTH;
    uint8_t m=1<<(y%8);
    if(color==2) buffer[i]^=m; else if(color) buffer[i]|=m; else buffer[i]&=~m;
}

void SSD1315::drawRect(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,bool fillRect,bool invert){
    // ensure coordinates are ordered from top-left to bottom-right
    if(x1 < x0){ uint8_t t = x0; x0 = x1; x1 = t; }
    if(y1 < y0){ uint8_t t = y0; y0 = y1; y1 = t; }
    for(uint8_t y=y0;y<=y1;y++){
        for(uint8_t x=x0;x<=x1;x++){
            bool edge=(x==x0||x==x1||y==y0||y==y1);
            if(fillRect||edge){
                drawPixel(x,y,invert?2:1);
            }
        }
    }
}

void SSD1315::drawBitmap(uint8_t x,uint8_t y,const uint8_t* bmp,uint8_t w,uint8_t h,bool invert){
    uint8_t bw=(w+7)/8;
    for(uint8_t j=0;j<h;j++){
        for(uint8_t i=0;i<w;i++){
            if(pgm_read_byte(bmp+j*bw+i/8)&(128>>(i&7)))
                drawPixel(x+i,y+j,invert?2:1);
        }
    }
}

void SSD1315::drawChar(uint8_t x,uint8_t y,char c,uint8_t size,bool invert){
    if(size <= 5){
        const uint8_t *font = &Font0508[c - ' '][0];
        for(uint8_t i = 0; i < 5; i++){
            uint8_t line = pgm_read_byte(font + i);
            for(uint8_t j = 0; j < 8; j++){
                bool pix = line & 0x01;
                if(invert) pix = !pix;
                drawPixel(x + i, y + j, pix ? 1 : 0);
                line >>= 1;
            }
        }
        return;
    }

    const uint8_t *font = (size == 12) ? &Font1206[c - ' '][0]
                                       : &Font1608[c - ' '][0];
    uint8_t y0 = y;
    for(uint8_t i = 0; i < size; i++){
        uint8_t temp = pgm_read_byte(font + i);
        for(uint8_t j = 0; j < 8; j++){
            bool pix = temp & 0x80;
            if(invert) pix = !pix;
            drawPixel(x, y, pix ? 1 : 0);
            temp <<= 1;
            y++;
            if((y - y0) == size){
                y = y0;
                x++;
                break;
            }
        }
    }
}

void SSD1315::drawString(uint8_t x,uint8_t y,const char* s,uint8_t size,bool invert){
    uint8_t advance = (size <= 5) ? 5 : size / 2;
    while(*s){
        if(x > (SSD1315_WIDTH - advance)){
            x = 0;
            y += size;
            if(y > (SSD1315_HEIGHT - size)) y = 0;
        }
        drawChar(x, y, *s, size, invert);
        x += advance;
        s++;
    }
}

void SSD1315::display(){
    for(uint8_t page=0;page<SSD1315_PAGES;page++){
        command(0xB0+page);
        command(0x0c);
        command(0x11);
        for(uint8_t i=0;i<SSD1315_WIDTH;i++) data(buffer[i+page*SSD1315_WIDTH]);
    }
}

uint8_t* SSD1315::getBuffer(){return buffer;}

#include "font_data.h"
