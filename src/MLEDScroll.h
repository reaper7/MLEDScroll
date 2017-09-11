#ifndef __MLEDScroll_H
#define __MLEDScroll_H

#include <Arduino.h>

#define DEFSCROLLSPEED  50
#define DEFPAUSETIME    1500
#define DEFINTENSITY    1

#if defined ( ESP8266 )
#define MLEDPINDATA   13
#define MLEDPINCLOCK  14
#elif defined ( ESP32 )
#define MLEDPINDATA   23
#define MLEDPINCLOCK  18
#else
#define MLEDPINDATA   13
#define MLEDPINCLOCK  14
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
    char disBuffer[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    bool scroll();
    void message(String _msg);
    void message(String _msg, uint16_t _speed);
    void character(const char* _character);
    void character(char _character);
    void character(String _character);
    void icon(uint8_t _icon);
    void setIntensity(uint8_t _intens);
    uint8_t getIntensity();
    uint16_t scrollSpeed;                                                       // Scroll speed in ms (lower = faster):
    unsigned long msgPauseTime;                                                 // Pause after message in ms (0 = no pause): 
    bool flip;

  private:
    void sendCommand(uint8_t _cmd);
    void sendData(uint8_t _address, uint8_t _data);
    void send(uint8_t _data);
    uint8_t dataPin;
    uint8_t clockPin;
    void sendDisBuffer();                                                       // Send 8 bytes at one with autoincrement (much faster)
    uint8_t _intensity;
    void initScroll();
    void fetchChr();
    void moveScrollBuffer();
    uint8_t swap(uint8_t _x);
};

#endif
