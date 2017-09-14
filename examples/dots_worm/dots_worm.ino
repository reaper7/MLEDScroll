#include <MLEDScroll.h>

MLEDScroll matrix;

void setup() {
  matrix.begin();
  //matrix.flip=false;
}

void loop() {
  for(uint8_t i=0;i<8;i++) {
    matrix.dot(i, 0, 1);
    matrix.dot(0, 7-i, 0);
    matrix.display();
    delay(100);
  }

  for(uint8_t i=0;i<8;i++) {
    matrix.dot(7, i, 1);
    matrix.dot(i, 0, 0);
    matrix.display();
    delay(100);
  }

  for(uint8_t i=0;i<8;i++) {
    matrix.dot(7-i, 7, 1);
    matrix.dot(7, i, 0);
    matrix.display();
    delay(100);
  }

  for(uint8_t i=0;i<8;i++) {
    matrix.dot(0, 7-i, 1);
    matrix.dot(7-i, 7, 0);
    matrix.display();
    delay(100);
  }
}
