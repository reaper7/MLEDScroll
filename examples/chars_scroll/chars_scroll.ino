#include <MLEDScroll.h>

MLEDScroll matrix;

void letterBlink(String _letter, uint8_t _pause) {
  matrix.setIntensity(0);
  matrix.character(_letter);
  for(uint8_t i=0;i<=8;i++) {
    matrix.setIntensity(i);
    delay(_pause);  
  }
  delay(_pause*10);
  for(uint8_t i=8;i<=0;i--) {
    matrix.setIntensity(i);
    delay(_pause);  
  }   
}

void letterScroll(String _letter, uint8_t _direction) {
  matrix.setIntensity(0);
  matrix.character(_letter);
  for(uint8_t i=0;i<=8;i++) {
    matrix.setIntensity(i);
    delay(50);  
  }
  delay(500);
  uint8_t scrRes;
  do {
    scrRes=matrix.scroll(_direction);
    if (scrRes==1)
      matrix.setIntensity(matrix.getIntensity()-1);
  } while (scrRes<=SCROLL_MOVED);
}

void setup() {
  matrix.begin();
  //matrix.flip=false;
}

void loop() {
  letterBlink("3", 50);
  letterBlink("2", 50);
  letterBlink("1", 50);
  letterBlink("0", 50);
  letterBlink("S", 25);
  letterBlink("t", 25);
  letterBlink("a", 25);
  letterBlink("r", 25);
  letterBlink("t", 25);

  delay(500);

  letterScroll("L", SCROLL_LEFT);
  letterScroll("R", SCROLL_RIGHT);
  letterScroll("D", SCROLL_DOWN);
  letterScroll("U", SCROLL_UP);

  matrix.setIntensity(1);
  matrix.message("done... ");
  while (matrix.scroll()!=SCROLL_ENDED) {
    yield();
  }

  matrix.clear();
  delay(500); 
}
