#include <pgmspace.h>
#include "MLEDScroll.h"
#include "MLEDScrollFonts.h"

#define MAXTEXTLEN        254
#define ICONPOSSTART      256
#define ICONMAX           6
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

unsigned long thisMs;
volatile unsigned long lastMs;
volatile unsigned long pauseStart;
bool pauseDisplay;
uint16_t msgPos;
uint16_t msgLen;
uint8_t buffPos;
uint8_t currDir;
bool firstChrSet;
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

  thisMs = 0;
  lastMs = 0;
  pauseStart = 0;
  pauseDisplay = false;
  msgPos = 0;
  msgLen = 0;
  buffPos = 0;
  currDir = SCROLL_LEFT;
  firstChrSet = false;
  _scrollStatus = SCROLL_WAITED;
}

void MLEDScroll::begin() {
#if defined (MATRIXDEBUG)
  _serialInit();
#endif
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
  }
}

void MLEDScroll::hLine(uint8_t _x, uint8_t _y, uint8_t _l, bool _draw) {
    for( uint8_t i = 0; i < _l; i++){
        dot(_x+i, _y, _draw);
    }
}

void MLEDScroll::vLine(uint8_t _x, uint8_t _y, uint8_t _l, bool _draw) {
    for( uint8_t i = 0; i < _l; i++){
        dot(_x, _y+i, _draw);
    }
}

void MLEDScroll::rect(uint8_t _x, uint8_t _y, uint8_t _l, uint8_t _h, bool _draw) {
    hLine(_x, _y, _l, _draw);
    hLine(_x, _y+_h, _l, _draw);
    vLine(_x, _y, _h, _draw);
    vLine(_x+_l, _y, _h, _draw);
}

void MLEDScroll::initScroll() {
#if defined (MATRIXDEBUG)
  _printFrame();
#endif
  msgPos = 0;
  buffPos = 0;
  firstChrSet = false;
  pauseDisplay = false;
  pauseStart = 0;
  lastMs = 0;
}

void MLEDScroll::fetchChr() {
  if (msgPos==msgLen) {
    pauseDisplay = true;
    pauseStart = thisMs;
  }
  memcpy_P(disBuffer+8, matrix_fonts+(charMsg[msgPos]*8), 8);
  msgPos++;
}

void MLEDScroll::moveScrollBuffer(uint8_t _direction) {
  if (!firstChrSet) {
    fetchChr();
    firstChrSet = true;
  }

  switch(_direction) {
    case SCROLL_LEFT:                                                           // from right to left
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
    case SCROLL_RIGHT:                                                          // from left to right
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
    case SCROLL_DOWN:                                                           // from top to bottom
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
    case SCROLL_UP:                                                             // from bottom to top
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
  if (currDir != _direction) {
    currDir = _direction;
  }
  if (thisMs - lastMs > scrollSpeed) {
    lastMs = thisMs;
    if (pauseDisplay == true) {
      if (thisMs - pauseStart > msgPauseTime) {
        initScroll();
        _scrollStatus = SCROLL_ENDED;
      } else {
        _scrollStatus = SCROLL_PAUSED;
      }
    } else {
      moveScrollBuffer(currDir);
      display();
      _scrollStatus = SCROLL_MOVED;
    }
  } else {
    _scrollStatus = SCROLL_WAITED;
  }
#if defined (MATRIXDEBUG)
  Serial.print("\033[?25l");                                                    // cursor off
  if (_scrollStatus!=SCROLL_WAITED)
    Serial.printf("\033[%d;%dH%d", 12, 14, _scrollStatus);
  Serial.printf("\033[%d;%dH%d", 15, 14, currDir);
#endif
  yield();
  return _scrollStatus;
}

uint8_t MLEDScroll::scroll(uint8_t _direction, uint16_t _speed) {
  scrollSpeed = _speed;
  return scroll(_direction);
}

void MLEDScroll::message(String _msg) {
  uint16_t _len = _msg.length();
  if (_len < (MAXTEXTLEN-1)) {
    memset(charMsg, 0, sizeof(charMsg));
    _msg.toCharArray(charMsg, _len+1);
    msgLen = _len;
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
  msgLen = 1;
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
  msgLen = 1;
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
#if defined (MATRIXDEBUG)
  _printValue(_address);
#endif
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
#if defined (MATRIXDEBUG)
  _printValue();
#endif
}

uint8_t MLEDScroll::swap(uint8_t _x) {
  _x = (((_x&0xaa)>>1) + ((_x&0x55)<<1));
  _x = (((_x&0xcc)>>2) + ((_x& 0x33)<<2));
  return((_x>>4) + (_x<<4));
}

// debug part
#if defined (MATRIXDEBUG)
void MLEDScroll::_serialInit() {
  Serial.begin(115200);
  Serial.printf("\033[2J\033[H");                                               // ESC command + clear screen + move cursor to home pos
}

void MLEDScroll::_printFrame() {
  Serial.print("\033[?25l");                                                    // cursor off
  Serial.print("\033[H");                                                       // draw terminal table
  Serial.print("\033[1;1H |\033[31m76543210\033[39;49m|\033[37m76543210|");
  Serial.print("\033[2;1H-|--------|--------|");
  for(uint8_t i=3;i<11;i++) {
    Serial.printf("\033[%d;1H%d|        |        |", i, i-3);
  }
  Serial.print("\033[11;1H-|--------|--------|");

  Serial.printf("\033[%d;%dH%s", 12, 1, "SCROLL STAT:");
  Serial.printf("\033[%d;%dH%s", 13, 1, "MESSAGE POS:");
  Serial.printf("\033[%d;%dH%s", 14, 1, "BUFFER  POS:");
  Serial.printf("\033[%d;%dH%s", 15, 1, "CURRENT DIR:");
}

void MLEDScroll::_printValue(int8_t _pos) {
  uint8_t _min, _max;
  if (_pos < 0) {
    _min=0;
    _max=8;
  } else {
    _min=_pos;
    _max=_pos;
  }
  Serial.print("\033[?25l");                                                    // cursor off
  for(uint8_t i=_min;i<_max;i++) {
    for(uint8_t x=0;x<2;x++) {
      uint8_t a = disBuffer[(8*x)+i];
      char b[sizeof(a)*8+1] = {0};
      for (size_t z=0;z<sizeof(a)*8;z++) {
        b[sizeof(a)*8-1-z] = ((a>>z) & 0x1) ? '#' : ' ';
      }
      Serial.printf("\033[%dm\033[%d;%dH%s", x?37:31, i+3, x?12:3, b);
    }
  }
  Serial.print("\033[39;49m");                                                  // restore default colors
  Serial.printf("\033[%d;%dH%d ", 13, 14, msgPos);
  Serial.printf("\033[%d;%dH%d", 14, 14, buffPos);
}
#endif
