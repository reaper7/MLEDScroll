#include <pgmspace.h>
#include "MLEDScroll.h"
#include "MLEDScrollFonts.h"

#define MAXTEXTLEN      250
#define ICONPOSSTART    256

unsigned long thisMs = 0;
unsigned long lastMs = 0;
unsigned long pauseStart;
bool pauseDisplay = false;
uint16_t msgPos = 0;
uint8_t buffPos = 0;
char charMsg[MAXTEXTLEN];

MLEDScroll::MLEDScroll(uint8_t _intens, uint8_t _dataPin, uint8_t _clockPin, bool _flip) {
	this->dataPin = _dataPin;
	this->clockPin = _clockPin;

	if(_intens>7)
		_intensity=7;
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
  display();
  setIntensity(_intensity);
}

void MLEDScroll::setIntensity(uint8_t _intens) {
	if(_intens>7)
		_intensity=7;
	else
		_intensity=_intens;

  sendCommand(0x88|(_intensity));
}

void MLEDScroll::display() {
  sendDisBuffer();
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
  clear();
  msgPos = 0;
  buffPos = 0;
  fetchChr();
  pauseDisplay = false;
  pauseStart = 0;
  lastMs = 0;
}

void MLEDScroll::fetchChr() {
  int foundVal = 0;

  if (charMsg[msgPos] == 0) {
    msgPos = 0;
    pauseDisplay = true;
    pauseStart = thisMs;
  }
  memcpy_P(disBuffer+8, matrix_fonts+(charMsg[msgPos++]*8), 8);
}

void MLEDScroll::moveScrollBuffer() {
  for(uint8_t i=0;i<8;i++) {
    disBuffer[i] = disBuffer[i] << 1;
    if (disBuffer[i + 8] & 128) {
      disBuffer[i] = disBuffer[i] | 1;
    }
    disBuffer[i + 8] = disBuffer[i + 8] << 1;
  }

  buffPos++;
  if (buffPos > 7) {
    buffPos = 0;
    fetchChr();
  }
}

bool MLEDScroll::scroll() {
  bool res = false;
  thisMs = millis();
  if (thisMs - lastMs > scrollSpeed) {
    lastMs = thisMs;
    if (pauseDisplay == true) {
      if (thisMs - pauseStart > msgPauseTime) {
        initScroll();
        res = true;
      }
    } else {
      moveScrollBuffer();
      display();
      res = true;
    }  
  }
  return res;
}

void MLEDScroll::message(String _msg) {
  if (_msg.length() < (MAXTEXTLEN - 2)) {
    _msg.toCharArray(charMsg, _msg.length()+1);
    initScroll();
  }
}

void MLEDScroll::message(String _msg, uint16_t _speed) {
  scrollSpeed = _speed;
  message(_msg);
}

void MLEDScroll::letter(const char* _letter) {
  memcpy_P(disBuffer, matrix_fonts+(*_letter*8), 8);
  display();
}

void MLEDScroll::letter(char _letter) {
  char out[2];
  out[0] = _letter;
  out[1] = '\0';
  letter(out);
}

void MLEDScroll::letter(String _letter) {
  letter(_letter.substring(0,1).c_str());
}

void MLEDScroll::icon(uint8_t _icon) {
  memcpy_P(disBuffer, matrix_fonts+((ICONPOSSTART + _icon) * 8), 8);
  display();
}

void MLEDScroll::sendCommand(uint8_t _cmd) {
  digitalWrite(dataPin, LOW);
  send(_cmd);
  digitalWrite(dataPin, HIGH);
}

void MLEDScroll::sendData(uint8_t _address, uint8_t _data) {
  sendCommand(0x44);
  digitalWrite(dataPin, LOW);
  send(0xC0 | _address);
  send(_data);
  digitalWrite(dataPin, HIGH);
}

void MLEDScroll::send(uint8_t _data) {
  for(uint8_t i=0;i<8;i++) {
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, !!(_data & (1 << i)));
    digitalWrite(clockPin, HIGH);
  }
}

void MLEDScroll::sendDisBuffer() {
  sendCommand(0x40);                                                            //set data autoincrement
  digitalWrite(dataPin, LOW);
  send(0xC0);                                                                   //set address to 0x00
  for(uint8_t i=0;i<8;i++) {
    if (flip)
      send(disBuffer[i]);
    else
      send(swap(disBuffer[7-i]));
  }
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(clockPin, HIGH);
  digitalWrite(dataPin, HIGH); 
}

uint8_t MLEDScroll::swap(uint8_t _x) {
  _x = (((_x&0xaa)>>1) + ((_x&0x55)<<1));
  _x = (((_x&0xcc)>>2) + ((_x& 0x33)<<2));
  return((_x>>4) + (_x<<4));
}
