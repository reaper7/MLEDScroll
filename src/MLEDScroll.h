#ifndef __MLEDScroll_H
#define __MLEDScroll_H

#include <Arduino.h>

//#define MATRIXDEBUG

#define DEFSCROLLSPEED  50
#define DEFPAUSETIME    1500
#define DEFINTENSITY    1

// scroll direction
#define SCROLL_LEFT     0
#define SCROLL_RIGHT    1                                                       // to right not fully working jet
#define SCROLL_DOWN     2
#define SCROLL_UP       3

// scroll status
#define SCROLL_WAITED   0
#define SCROLL_MOVED    1
#define SCROLL_PAUSED   2
#define SCROLL_ENDED    3

#if defined ( ESP8266 )
#define MLEDPINDATA     13
#define MLEDPINCLOCK    14
#elif defined ( ESP32 )
#define MLEDPINDATA     23
#define MLEDPINCLOCK    18
#else
#define MLEDPINDATA     13
#define MLEDPINCLOCK    14
#endif

class MLEDScroll
{

  public:
    MLEDScroll(uint8_t _intens=DEFINTENSITY, uint8_t _dataPin=MLEDPINDATA, uint8_t _clockPin=MLEDPINCLOCK, bool _flip=true);
    void begin();
    void display();
    void display(uint8_t _intens);
    void clear();
    void dot(uint8_t _x, uint8_t _y, bool _draw=true, bool _updCurrRow=false);
    void hLine(uint8_t _x, uint8_t _y, uint8_t _l, bool _draw=true);
    void vLine(uint8_t _x, uint8_t _y, uint8_t _l, bool _draw=true);
    void rect(uint8_t _x, uint8_t _y, uint8_t _l, uint8_t _h, bool _draw=true);
    char disBuffer[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t scroll(uint8_t _direction=SCROLL_LEFT);
    uint8_t scroll(uint8_t _direction, uint16_t _speed);
    void message(String _msg);
    void message(String _msg, uint16_t _speed);
    void message(String _msg, uint16_t _speed, unsigned long _pauseTime);
    void character(const char* _character);
    void character(char _character);
    void character(String _character);
    void icon(uint8_t _icon);
    void setIntensity(uint8_t _intens);
    uint8_t getIntensity();
    uint16_t scrollSpeed;                                                       // scroll speed in ms (lower = faster):
    unsigned long msgPauseTime;                                                 // pause after message in ms (0 = no pause):
    bool flip;

  private:
    void sendStart();
    void sendEnd();
    void send(uint8_t _data);
    void sendCommand(uint8_t _cmd);
    void sendData(uint8_t _address, uint8_t _data);                             // send 1 data byte at fixed address
    void sendDataBlock();                                                       // send 8 data bytes at once with autoincrement (much faster)
    uint8_t dataPin;
    uint8_t clockPin;
    uint8_t _intensity;
    uint8_t _scrollStatus;
    void initScroll();
    void fetchChr();
    void moveScrollBuffer(uint8_t _direction);
    uint8_t swap(uint8_t _x);

    // debug part
#if defined (MATRIXDEBUG)
    void _serialInit();
    void _printFrame();
    void _printValue(int8_t _pos=-1);
#endif
};

#endif
