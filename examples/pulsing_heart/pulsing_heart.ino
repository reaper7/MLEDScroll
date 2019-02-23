#include <MLEDScroll.h>

MLEDScroll matrix;

void heartpulse(uint8_t _icon) {
  matrix.setIntensity(0);
  matrix.icon(_icon);
  for(uint8_t i=0;i<=8;i++) {
    matrix.setIntensity(i);
    delay(20);  
  }
  for(uint8_t i=8;i>1;i--) {
    matrix.setIntensity(i);
    delay(20);  
  }
}

void setup() {
  matrix.begin();
  //matrix.flip=false;
  matrix.clear();
}

void loop() {
  heartpulse(260); // 0x105 = 260 = Heart
  delay(900); 
}
