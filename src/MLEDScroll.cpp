#include <pgmspace.h>
#include "MLEDScroll.h"
#include "MLEDScrollFonts.h"

#define MAXTEXTLEN        254
#define ICONPOSSTART      256
#define ICONMAX           4
#define DEFPINDELAY       1

#define DISP_CTRL_CMD     0x80
#define DISP_OFF          0x00
#define DISP_ON           0x08
#define DISP_INTENS_MASK  0x07

#define DISP_DATA_CMD     0x40
#define DISP_DATA_INC     0x00
#define DISP_DATA_FIX     0x04

#define DISP_ADDR_CMD     0xC0
#define DISP_ADDR_MASK    0x0F

unsigned long thisMs = 0;
volatile unsigned long lastMs = 0;
volatile unsigned long pauseStart;
bool pauseDisplay = false;
uint16_t msgPos = 0;
uint8_t buffPos = 0;
char charMsg[MAXTEXTLEN];

MLEDScroll::MLEDScroll(uint8_t _intens, uint8_t _dataPin, uint8_t _clockPin, bool _flip) {
  this->dataPin = _dataPin;
  this->clockPin = _clockPin;

  if(_intens>8)
    _intensity=8;
  else
    _intensity=_intens;

  scrollSpeed = DEFSCROLLSPEED;
  msgPauseTime = DEFPAUSETIME;
  flip = _flip;
}

void MLEDScroll::begin() {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(dataPin, HIGH);
  digitalWrite(clockPin, HIGH);

  clear();
  setIntensity(_intensity);
}

void MLEDScroll::setIntensity(uint8_t _intens) {
  if(_intens>8)
    _intensity=8;
  else
    _intensity=_intens;

  if (_intensity==0)
    sendCommand(DISP_CTRL_CMD | DISP_OFF);                                      // display off
  else
    sendCommand(DISP_CTRL_CMD | DISP_ON | ((_intensity-1) & DISP_INTENS_MASK));
}

uint8_t MLEDScroll::getIntensity() {
  return (_intensity);
}

void MLEDScroll::display() {
  sendDataBlock();
}

void MLEDScroll::display(uint8_t _intens) {
  display();
  setIntensity(_intens);
}

void MLEDScroll::clear() {
  memset(disBuffer, 0x00, 16);
  display();
}

void MLEDScroll::dot(uint8_t _x, uint8_t _y, bool _draw, bool _updCurrRow) {
  _x&=0x07;
  _y&=0x07;

  if(_draw) {
    disBuffer[_y]|=(1<<(7-_x));
  } else {
    disBuffer[_y]&=~(1<<(7-_x));
  }

  if (_updCurrRow) {
    if (flip)
      sendData(_y, disBuffer[_y]);
    else
      sendData(7-_y, swap(disBuffer[_y]));
    digitalWrite(dataPin, LOW);
    digitalWrite(clockPin, LOW);
    digitalWrite(clockPin, HIGH);
    digitalWrite(dataPin, HIGH);    
  }
}

void MLEDScroll::initScroll() {
  msgPos = 0;
  buffPos = 0;
  fetchChr();
  pauseDisplay = false;
  pauseStart = 0;
  lastMs = 0;
}

void MLEDScroll::fetchChr() {
  if (charMsg[msgPos] == 0) {
    msgPos = 0;
    pauseDisplay = true;
    pauseStart = thisMs;
  }
  memcpy_P(disBuffer+8, matrix_fonts+(charMsg[msgPos++]*8), 8);
}

void MLEDScroll::moveScrollBuffer(uint8_t _direction) {
  switch(_direction) {
    case SCROLLLEFT:                                                            // from right to left
      {
        for(uint8_t i=0;i<8;i++) {
          disBuffer[i] = disBuffer[i] << 1;
          if (disBuffer[i + 8] & 128) {
            disBuffer[i] = disBuffer[i] | 1;
          }
          disBuffer[i+8] = disBuffer[i+8] << 1;
        }
      }  
      break;
    case SCROLLRIGHT:                                                           // from left to right
      {
        for(uint8_t i=0;i<8;i++) {
          disBuffer[i] = disBuffer[i] >> 1;
          if (disBuffer[i + 8] & 1) {
            disBuffer[i] = disBuffer[i] | 128;
          }
          disBuffer[i+8] = disBuffer[i+8] >> 1;
        }
      }  
      break;
    case SCROLLDOWN:                                                            // from top to bottom
      {
        char _tmp = disBuffer[15];
        for(uint8_t i=7;i>0;i--) {
          memmove(disBuffer+i,disBuffer+(i-1),1);
          memmove(disBuffer+(i+8),disBuffer+(i+7),1);
        }
        disBuffer[0] = _tmp;
        disBuffer[8] = 0x00;
      }
      break;
    case SCROLLUP:                                                              // from bottom to top
      memmove(disBuffer, disBuffer+1, 15);
      disBuffer[15] = 0x00;
      break;
    default:
    break;
  }

  buffPos++;
  if (buffPos > 7) {
    buffPos = 0;
    fetchChr();
  }
}

uint8_t MLEDScroll::scroll(uint8_t _direction) {
  thisMs = millis();
  if (thisMs - lastMs > scrollSpeed) {
    lastMs = thisMs;
    if (pauseDisplay == true) {
      if (thisMs - pauseStart > msgPauseTime) {
        initScroll();
        return SCROLL_ENDED;
      } else {
        return SCROLL_PAUSED;      
      }
    } else {
      moveScrollBuffer(_direction);
      display();
      return SCROLL_MOVED;
    }  
  }
  return SCROLL_WAITED;
}

uint8_t MLEDScroll::scroll(uint8_t _direction, uint16_t _speed) {
  scrollSpeed = _speed;
  return scroll(_direction);
}

void MLEDScroll::message(String _msg) {
  if (_msg.length() < (MAXTEXTLEN-1)) {
    memset(charMsg, 0, sizeof(charMsg));
    _msg.toCharArray(charMsg, _msg.length()+1);
    initScroll();
  }
}

void MLEDScroll::message(String _msg, uint16_t _speed) {
  scrollSpeed = _speed;
  message(_msg);
}

void MLEDScroll::message(String _msg, uint16_t _speed, unsigned long _pauseTime) {
  scrollSpeed = _speed;
  msgPauseTime = _pauseTime;
  message(_msg);
}

void MLEDScroll::character(const char* _character) {
  memcpy_P(disBuffer, matrix_fonts+(*_character*8), 8);
  memset(charMsg, 0, sizeof(charMsg));
  initScroll();
  display();
}

void MLEDScroll::character(char _character) {
  char out[2];
  out[0] = _character;
  out[1] = char(0);
  character(out);
}

void MLEDScroll::character(String _character) {
  character(_character.substring(0,1).c_str());
}

void MLEDScroll::icon(uint8_t _icon) {
  if (_icon>=ICONMAX)
    _icon = 0; 
  memcpy_P(disBuffer, matrix_fonts+((ICONPOSSTART + _icon)*8), 8);
  memset(charMsg, 0, sizeof(charMsg));
  initScroll();
  display();
}

void MLEDScroll::sendStart() {
  digitalWrite(dataPin, LOW);
  delayMicroseconds(DEFPINDELAY);
  digitalWrite(clockPin, LOW);
  delayMicroseconds(DEFPINDELAY);
}

void MLEDScroll::sendEnd() {
  digitalWrite(dataPin, LOW);
  delayMicroseconds(DEFPINDELAY);
  digitalWrite(clockPin, HIGH);
  delayMicroseconds(DEFPINDELAY);
  digitalWrite(dataPin, HIGH);
  delayMicroseconds(DEFPINDELAY); 
}

void MLEDScroll::send(uint8_t _data) {
  for(uint8_t i=0;i<8;i++) { 
    digitalWrite(dataPin, !!(_data & (1<<i)));
    delayMicroseconds(DEFPINDELAY);
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(DEFPINDELAY);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(DEFPINDELAY);
  }
}

void MLEDScroll::sendCommand(uint8_t _cmd) {
  sendStart();
  send(_cmd);
  sendEnd();
}

void MLEDScroll::sendData(uint8_t _address, uint8_t _data) {
  sendCommand(DISP_DATA_CMD | DISP_DATA_FIX);                                   // set data fixed address
  sendStart();
  send(DISP_ADDR_CMD | (_address & DISP_ADDR_MASK));
  send(_data);
  sendEnd();
}

void MLEDScroll::sendDataBlock() {
  sendCommand(DISP_DATA_CMD | DISP_DATA_INC);                                   // set data address autoincrement
  sendStart();
  send(DISP_ADDR_CMD);                                                          // send 8 data bytes at addr 0x00
  for(uint8_t i=0;i<8;i++) {
    if (flip)
      send(disBuffer[i]);
    else
      send(swap(disBuffer[7-i]));
  }
  sendEnd();
}

uint8_t MLEDScroll::swap(uint8_t _x) {
  _x = (((_x&0xaa)>>1) + ((_x&0x55)<<1));
  _x = (((_x&0xcc)>>2) + ((_x& 0x33)<<2));
  return((_x>>4) + (_x<<4));
}
